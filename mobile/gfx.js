'use strict';
// Display pipeline for the mobile build: 320x200 palette frame -> WebGL shader (scaling filters + CRT), with a
// plain 2D canvas fallback. The canvas is sized to the real on-screen pixel size so the shader works at native
// resolution instead of stretching a fixed 640x480 image.

const Settings = (() => {
  const s = { crt: 0, scale: 1, music: true, sfx: true };   // crt: 0 off, 1 scanlines, 2 full CRT;  scale: 0 pixel, 1 sharp, 2 smooth
  try { Object.assign(s, JSON.parse(localStorage.getItem('freez.settings') || '{}')); } catch (e) { }
  s.save = () => { try { localStorage.setItem('freez.settings', JSON.stringify({ crt: s.crt, scale: s.scale, music: s.music, sfx: s.sfx })); } catch (e) { } };
  return s;
})();
const CRT_NAMES = ['Off', 'Scanlines', 'Full CRT'];
const SCALE_NAMES = ['Pixel', 'Sharp', 'Smooth'];

const Gfx = (() => {
  let canvas, gl = null, ctx2d = null, prog, tex, uRes, uMode, uCrt, uTime;
  const px = new Uint8Array(320 * 200 * 4);
  const px32 = new Uint32Array(px.buffer);
  const lut = new Uint32Array(256);
  let w = 640, h = 480, lowCanvas, lowCtx, lowImg;

  const VS = `attribute vec2 a; varying vec2 v_uv; void main(){ v_uv = a*.5+.5; v_uv.y = 1.-v_uv.y; gl_Position = vec4(a,0.,1.); }`;
  const FS = `
precision highp float;
uniform sampler2D u_tex; uniform vec2 u_res; uniform float u_mode; uniform float u_crt; uniform float u_time;
varying vec2 v_uv;
const vec2 SRC = vec2(320.,200.);
vec3 texel(vec2 t){ return texture2D(u_tex, (floor(t)+.5)/SRC).rgb; }
vec2 curve(vec2 uv){ uv = uv*2.-1.; vec2 o = abs(uv.yx)/vec2(5.5,4.2); uv += uv*o*o; return uv*.5+.5; }
vec3 epx(vec2 p){
  vec2 f = fract(p); vec2 s = vec2(f.x>.5?1.:-1., f.y>.5?1.:-1.);
  vec3 P = texel(p), H = texel(p+vec2(s.x,0.)), V = texel(p+vec2(0.,s.y));
  vec3 Ho = texel(p-vec2(s.x,0.)), Vo = texel(p-vec2(0.,s.y));
  if(H==V && H!=Vo && V!=Ho) return H;
  return P;
}
void main(){
  vec2 uv = v_uv;
  if(u_crt > 1.5) uv = curve(uv);
  if(uv.x<0.||uv.x>1.||uv.y<0.||uv.y>1.){ gl_FragColor = vec4(0.,0.,0.,1.); return; }
  vec2 p = uv*SRC;
  vec2 scale = u_res/SRC;
  vec3 col;
  if(u_mode < .5){
    col = texel(p);
  } else if(u_mode < 1.5){
    // sharp bilinear: crisp texel centres, 1 output pixel of blend at the edges
    vec2 loc = fract(p);
    vec2 k = clamp(loc*scale, 0., .5) + clamp((loc-1.)*scale + .5, 0., .5);
    col = texture2D(u_tex, (floor(p)+k)/SRC).rgb;
  } else {
    col = epx(p);
  }
  if(u_crt > .5){
    float strength = u_crt > 1.5 ? 1. : .6;
    // scanline beam profile: bright in the centre of each source row, dark gap between rows
    float fy = fract(p.y);
    float beam = exp(-pow((fy-.5)/.38, 2.));
    col *= mix(1., .35 + 1.05*beam, strength);
    if(u_crt > 1.5){
      // phosphor shadow mask (only when there are enough output pixels per texel to avoid moire)
      float m = clamp((scale.x-2.2)*.5, 0., 1.);
      float tri = mod(gl_FragCoord.x, 3.);
      vec3 mask = vec3(tri<1.?1.:.78, tri>=1.&&tri<2.?1.:.78, tri>=2.?1.:.78);
      col *= mix(vec3(1.), mask, m*.9);
      // cheap horizontal bloom
      vec3 bl = (texel(p+vec2(-1.,0.)) + texel(p+vec2(1.,0.)))*.5;
      col += .12*bl*beam;
      // vignette + rounded corners
      vec2 q = uv*2.-1.;
      col *= 1. - .28*dot(q*vec2(.8,.9), q*vec2(.8,.9));
      vec2 d = abs(q) - (1.-.06);
      float corner = length(max(d,0.)) - .0;
      col *= 1. - smoothstep(.0,.04,corner);
      col = pow(col, vec3(.95)) * 1.12;
      // faint flicker
      col *= 1. + .006*sin(u_time*50.);
    } else {
      col *= 1.08;
    }
  }
  gl_FragColor = vec4(col, 1.);
}`;

  function compile(type, src) {
    const s = gl.createShader(type); gl.shaderSource(s, src); gl.compileShader(s);
    if (!gl.getShaderParameter(s, gl.COMPILE_STATUS)) throw new Error(gl.getShaderInfoLog(s));
    return s;
  }
  function initGL() {
    gl = canvas.getContext('webgl', { antialias: false, alpha: false, preserveDrawingBuffer: false, powerPreference: 'high-performance' });
    if (!gl) return false;
    prog = gl.createProgram();
    gl.attachShader(prog, compile(gl.VERTEX_SHADER, VS));
    gl.attachShader(prog, compile(gl.FRAGMENT_SHADER, FS));
    gl.linkProgram(prog);
    if (!gl.getProgramParameter(prog, gl.LINK_STATUS)) throw new Error(gl.getProgramInfoLog(prog));
    gl.useProgram(prog);
    const buf = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, buf);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([-1, -1, 1, -1, -1, 1, 1, 1]), gl.STATIC_DRAW);
    const loc = gl.getAttribLocation(prog, 'a');
    gl.enableVertexAttribArray(loc); gl.vertexAttribPointer(loc, 2, gl.FLOAT, false, 0, 0);
    tex = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, tex);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 320, 200, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
    gl.uniform1i(gl.getUniformLocation(prog, 'u_tex'), 0);
    uRes = gl.getUniformLocation(prog, 'u_res'); uMode = gl.getUniformLocation(prog, 'u_mode');
    uCrt = gl.getUniformLocation(prog, 'u_crt'); uTime = gl.getUniformLocation(prog, 'u_time');
    return true;
  }
  function resize() {
    const r = canvas.getBoundingClientRect();
    const dpr = Math.min(window.devicePixelRatio || 1, 3);
    w = Math.max(320, Math.min(2560, Math.round(r.width * dpr)));
    h = Math.max(200, Math.min(1920, Math.round(r.height * dpr)));
    if (canvas.width !== w || canvas.height !== h) { canvas.width = w; canvas.height = h; }
    if (gl) gl.viewport(0, 0, w, h);
  }
  function init(c) {
    canvas = c;
    try { if (!initGL()) gl = null; } catch (e) { console.warn('WebGL unavailable, using 2D fallback', e); gl = null; }
    if (!gl) {
      ctx2d = canvas.getContext('2d');
      lowCanvas = document.createElement('canvas'); lowCanvas.width = 320; lowCanvas.height = 200;
      lowCtx = lowCanvas.getContext('2d'); lowImg = lowCtx.createImageData(320, 200);
    }
    resize();
    new ResizeObserver(resize).observe(canvas);
    addEventListener('resize', resize);
  }
  function draw(vram, dac) {
    for (let i = 0; i < 256; i++) {
      const r = dac[i * 3] & 63, g = dac[i * 3 + 1] & 63, b = dac[i * 3 + 2] & 63;
      lut[i] = (255 << 24) | (((b << 2) | (b >> 4)) << 16) | (((g << 2) | (g >> 4)) << 8) | ((r << 2) | (r >> 4));
    }
    for (let i = 0; i < 64000; i++) px32[i] = lut[vram[i]];
    if (gl) {
      gl.bindTexture(gl.TEXTURE_2D, tex);
      gl.texSubImage2D(gl.TEXTURE_2D, 0, 0, 0, 320, 200, gl.RGBA, gl.UNSIGNED_BYTE, px);
      gl.uniform2f(uRes, w, h);
      gl.uniform1f(uMode, Settings.scale);
      gl.uniform1f(uCrt, Settings.crt);
      gl.uniform1f(uTime, performance.now() / 1000);
      gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
    } else {
      lowImg.data.set(px); lowCtx.putImageData(lowImg, 0, 0);
      ctx2d.imageSmoothingEnabled = Settings.scale !== 0;
      ctx2d.drawImage(lowCanvas, 0, 0, w, h);
      if (Settings.crt) {
        ctx2d.fillStyle = Settings.crt > 1 ? 'rgba(0,0,0,.45)' : 'rgba(0,0,0,.3)';
        const rowH = h / 200;
        for (let y = 0; y < 200; y++) ctx2d.fillRect(0, y * rowH + rowH * .62, w, Math.max(1, rowH * .38));
      }
    }
  }
  return { init, draw, get webgl() { return !!gl; } };
})();
