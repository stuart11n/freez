'use strict';
// The small blue bird that bursts out of tree foliage. It is drawn procedurally (a plump blue bird facing right with
// an orange beak and a flapping wing), at 28x16 and then scaled down 50% to 14x8, so it needs no image file.
// To use hand-made art instead, replace makeBirdFrames() with a loader that returns the same shape:
//   { w, h, frames, data }  where data is frames*w*h palette indices (0 = transparent), facing right.
const BIRD = (() => {
  const FR = 8, SW = 28, SH = 16, W = SW >> 1, H = SH >> 1;
  const COL = { 1: [29, 63, 138], 2: [74, 114, 184], 3: [134, 184, 236], 4: [255, 255, 255], 5: [240, 160, 32] };   // navy, body, belly, eye, beak

  function drawFrame(f) {
    const g = new Uint8Array(SW * SH);
    const put = (x, y, c) => { x |= 0; y |= 0; if (x >= 0 && x < SW && y >= 0 && y < SH) g[y * SW + x] = c; };
    const ph = (f / FR) * Math.PI * 2, flap = Math.sin(ph);                    // -1 wings down .. +1 wings up
    // tail
    for (let y = 6; y <= 12; y++) for (let x = 1; x <= 6; x++) if (x + Math.abs(y - 9) * 1.6 <= 8) put(x, y, 1);
    // body (ellipse) with a lighter belly
    for (let y = 4; y <= 14; y++) for (let x = 3; x <= 24; x++) {
      const dx = (x - 13.5) / 11, dy = (y - 9.5) / 4.6;
      if (dx * dx + dy * dy <= 1) put(x, y, y >= 10 ? 3 : 2);
    }
    // head
    for (let y = 4; y <= 12; y++) for (let x = 17; x <= 26; x++) { const dx = (x - 21.5) / 4.6, dy = (y - 8.4) / 4.2; if (dx * dx + dy * dy <= 1) put(x, y, 2); }
    // beak, eye
    for (let x = 25; x <= 27; x++) { put(x, 8, 5); put(x, 9, 5); }
    for (let y = 6; y <= 7; y++) for (let x = 22; x <= 23; x++) put(x, y, 4);          // eye: one whole 2x2 block so it survives the downscale
    // wing: a triangle from the back that swings from below the body (down) to above it (up)
    const tipY = 7 - flap * 6.5, baseY = 8;
    for (let x = 8; x <= 18; x++) {
      const t = 1 - Math.abs(x - 12.5) / 5.5; if (t <= 0) continue;
      const y1 = baseY, y2 = baseY + (tipY - baseY) * t;
      for (let y = Math.min(y1, y2); y <= Math.max(y1, y2); y++) put(x, y, flap > -0.2 ? 3 : 2);
    }
    return g;
  }

  // 2x2 box downscale: a pixel is kept if at least 2 of the 4 source pixels are filled, taking the most common colour
  function down(g) {
    const o = new Uint8Array(W * H);
    for (let y = 0; y < H; y++) for (let x = 0; x < W; x++) {
      const cnt = {}; let n = 0;
      for (let dy = 0; dy < 2; dy++) for (let dx = 0; dx < 2; dx++) { const c = g[(y * 2 + dy) * SW + x * 2 + dx]; if (c) { cnt[c] = (cnt[c] || 0) + 1; n++; } }
      if (n >= 2) { let best = 0, bc = 0; for (const k in cnt) if (cnt[k] > bc || (cnt[k] === bc && +k > best)) { best = +k; bc = cnt[k]; } o[y * W + x] = best; }
    }
    return o;
  }

  // pal = 768 bytes of 6-bit RGB. Only palette entries < 128 that the game does not animate are used (>= 128 is solid ground).
  function build(pal) {
    const allowed = [];
    for (let i = 2; i < 128; i++) if (!(i >= 55 && i <= 57) && !(i >= 64 && i <= 103)) allowed.push(i);
    const map = {};
    for (const k of Object.keys(COL)) {
      const [r, g, b] = COL[k].map(v => v >> 2); let best = allowed[0], bd = 1e9;
      for (const i of allowed) { const d = (pal[i * 3] - r) ** 2 + (pal[i * 3 + 1] - g) ** 2 + (pal[i * 3 + 2] - b) ** 2; if (d < bd) { bd = d; best = i; } }
      map[k] = best;
    }
    const data = new Uint8Array(FR * W * H);
    for (let f = 0; f < FR; f++) { const s = down(drawFrame(f)); for (let i = 0; i < W * H; i++) data[f * W * H + i] = s[i] ? map[s[i]] : 0; }
    return { w: W, h: H, frames: FR, data };
  }
  return { build, W, H, FR };
})();
if (typeof module !== 'undefined') module.exports = BIRD;
