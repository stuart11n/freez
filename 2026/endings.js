'use strict';
// End screens (win / game over). They are drawn with a 2D canvas, then quantised into the game's 8-bit
// framebuffer using a fixed 6x6x6 colour cube so they go through the same palette/fade code as everything else.
const Endings = (() => {
  const c = document.createElement('canvas'); c.width = 320; c.height = 200;
  const g = c.getContext('2d', { willReadFrequently: true });
  const P = new Uint8Array(768);
  for (let r = 0; r < 6; r++) for (let gg = 0; gg < 6; gg++) for (let b = 0; b < 6; b++) {
    const i = (r * 36 + gg * 6 + b) * 3; P[i] = Math.round(r * 63 / 5); P[i + 1] = Math.round(gg * 63 / 5); P[i + 2] = Math.round(b * 63 / 5);
  }
  for (let i = 216; i < 256; i++) { const v = Math.round((i - 216) * 63 / 39); P[i * 3] = P[i * 3 + 1] = P[i * 3 + 2] = v; }
  const BAYER = [0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5];

  function toVram() {
    const d = g.getImageData(0, 0, 320, 200).data;
    for (let y = 0; y < 200; y++) for (let x = 0; x < 320; x++) {
      const o = (y * 320 + x) * 4, t = (BAYER[(y & 3) * 4 + (x & 3)] / 16 - .47) * 44;
      const r = Math.max(0, Math.min(5, Math.round((d[o] + t) / 51)));
      const gg = Math.max(0, Math.min(5, Math.round((d[o + 1] + t) / 51)));
      const b = Math.max(0, Math.min(5, Math.round((d[o + 2] + t) / 51)));
      vram[y * 320 + x] = r * 36 + gg * 6 + b;
    }
  }
  const pad = (n, w) => String(n).padStart(w, '0');
  function text(s, x, y, size, fill, shadow) {
    g.font = 'bold ' + size + 'px "Trebuchet MS", "Arial Black", sans-serif'; g.textAlign = 'center'; g.textBaseline = 'middle';
    if (shadow) { g.fillStyle = shadow; g.fillText(s, x + 1.5, y + 1.5); }
    g.fillStyle = fill; g.fillText(s, x, y);
  }
  const pressed = () => anyKey063() || keys[28] || keys[57] || keys[203] || keys[205];
  async function exit() {
    palMain.set(P); await Fade2Black(0); Clear(); SCopy();
  }

  async function win(st) {
    Music.play('win');
    palMain.set(P); setDAC(P);
    const stars = Array.from({ length: 70 }, () => ({ x: Math.random() * 320, y: Math.random() * 120, p: Math.random() * 6 }));
    const parts = [];
    const burst = () => {
      const cx = 40 + Math.random() * 240, cy = 30 + Math.random() * 60, hue = Math.random() * 360, n = 26 + (Math.random() * 14 | 0);
      for (let i = 0; i < n; i++) { const a = i / n * Math.PI * 2, v = 0.8 + Math.random() * 1.3; parts.push({ x: cx, y: cy, vx: Math.cos(a) * v, vy: Math.sin(a) * v - .4, life: 60 + Math.random() * 25, hue }); }
    };
    const bonus = ((st.timer >> 6) * 10), total = Math.min(65535, st.score + bonus);
    let shown = 0, t = 0;
    for (;;) {
      t++;
      const sky = g.createLinearGradient(0, 0, 0, 200); sky.addColorStop(0, '#050520'); sky.addColorStop(.6, '#2a1250'); sky.addColorStop(1, '#6a2a60');
      g.fillStyle = sky; g.fillRect(0, 0, 320, 200);
      for (const s of stars) { const a = .4 + .6 * Math.abs(Math.sin((t + s.p * 20) / 24)); g.fillStyle = 'rgba(255,255,255,' + a + ')'; g.fillRect(s.x | 0, s.y | 0, 1, 1); }
      g.fillStyle = '#10082a'; g.beginPath(); g.moveTo(0, 200);
      for (let x = 0; x <= 320; x += 8) g.lineTo(x, 172 + 10 * Math.sin(x / 38) + 6 * Math.sin(x / 17));
      g.lineTo(320, 200); g.fill();
      if (t % 38 === 1) burst();
      for (let i = parts.length - 1; i >= 0; i--) {
        const p = parts[i]; p.x += p.vx; p.y += p.vy; p.vy += .022; p.vx *= .992; p.life--;
        if (p.life <= 0) { parts.splice(i, 1); continue; }
        g.fillStyle = 'hsla(' + p.hue + ',100%,' + (55 + p.life / 3) + '%,' + Math.min(1, p.life / 30) + ')'; g.fillRect(p.x | 0, p.y | 0, 2, 2);
      }
      const bob = Math.sin(t / 14) * 2;
      text('CONGRATULATIONS!', 160, 34 + bob, 25, '#ffe36a', '#7a2a00');
      text('FREEZ COMPLETE', 160, 58, 14, '#9ff0ff', '#064');
      g.fillStyle = 'rgba(0,0,0,.45)'; g.fillRect(70, 76, 180, 70);
      if (t > 40 && shown < bonus) shown = Math.min(bonus, shown + Math.max(10, bonus / 90 | 0));
      text('SCORE  ' + pad(Math.min(65535, st.score + shown), 6), 160, 90, 13, '#fff', '#000');
      text('LIVES  x' + st.lives, 160, 108, 11, '#ff8aa0', '#000');
      text('TIME BONUS  +' + shown, 160, 125, 11, '#8ff7a0', '#000');
      if (shown >= bonus) text('FINAL  ' + pad(total, 6), 160, 141, 12, '#ffe36a', '#000');
      if ((t >> 5) & 1) text('PRESS ANY KEY', 160, 186, 10, '#fff', '#000');
      toVram();
      await vsync();
      if (t > 150 && shown >= bonus && pressed()) break;
    }
    while (pressed()) await vsync();
    await exit();
  }

  async function gameOver(st) {
    Music.play('over');
    palMain.set(P); setDAC(P);
    let t = 0;
    for (;;) {
      t++;
      const bg = g.createRadialGradient(160, 100, 10, 160, 100, 190); bg.addColorStop(0, '#3a0a12'); bg.addColorStop(1, '#050005');
      g.fillStyle = bg; g.fillRect(0, 0, 320, 200);
      for (let i = 0; i < 40; i++) { g.fillStyle = 'rgba(255,255,255,' + Math.random() * .08 + ')'; g.fillRect(Math.random() * 320, (t * 3 + i * 47) % 200, 2 + Math.random() * 30, 1); }
      const j = t % 90 < 4 ? (Math.random() - .5) * 6 : 0;
      text('GAME OVER', 160 + j, 78, 34, '#ff4a4a', '#300');
      text('SCORE  ' + pad(st.score, 6), 160, 122, 14, '#fff', '#000');
      if (t > 120 && ((t >> 5) & 1)) text('PRESS ANY KEY', 160, 170, 10, '#ddd', '#000');
      toVram();
      await vsync();
      if (t > 150 && pressed()) break;
    }
    while (pressed()) await vsync();
    await exit();
  }
  return { win, gameOver };
})();
