'use strict';
// On-screen controls: each button synthesises the same keyboard events the game already listens for.
(() => {
  const send = (code, down) => window.dispatchEvent(new KeyboardEvent(down ? 'keydown' : 'keyup', { code, bubbles: true, cancelable: true }));
  const buttons = [...document.querySelectorAll('[data-code]')];
  const state = new Map(buttons.map(btn => [btn.dataset.code, { btn, pointers: new Set() }]));
  const held = new Map();

  const press = (code, id) => {
    const s = state.get(code);
    if (!s || s.pointers.has(id)) return;
    if (!s.pointers.size) { send(code, true); s.btn.classList.add('on'); if (navigator.vibrate) navigator.vibrate(8); }
    s.pointers.add(id);
  };
  const release = (code, id) => {
    const s = state.get(code);
    if (!s || !s.pointers.has(id)) return;
    s.pointers.delete(id);
    if (!s.pointers.size) { send(code, false); s.btn.classList.remove('on'); }
  };
  const bind = (id, code) => {
    const prev = held.get(id);
    if (prev === code) return;
    if (prev) release(prev, id);
    if (code) { press(code, id); held.set(id, code); }
    else held.delete(id);
  };
  const releaseAll = () => { for (const id of [...held.keys()]) bind(id, null); };

  document.addEventListener('pointerdown', e => {
    const btn = e.target.closest && e.target.closest('[data-code]');
    if (!btn) return;
    e.preventDefault();
    try { btn.setPointerCapture(e.pointerId); } catch (err) { }
    bind(e.pointerId, btn.dataset.code);
  });
  // iPhone often misses per-button pointerup when the thumb slides off the right edge
  const onUp = e => bind(e.pointerId, null);
  document.addEventListener('pointerup', onUp);
  document.addEventListener('pointercancel', onUp);
  document.addEventListener('lostpointercapture', e => {
    if (!held.has(e.pointerId)) return;
    if (e.buttons) return;
    bind(e.pointerId, null);
  });
  addEventListener('visibilitychange', () => { if (document.hidden) releaseAll(); });
  addEventListener('pagehide', releaseAll);
  addEventListener('blur', releaseAll);

  for (const ev of ['contextmenu', 'gesturestart', 'dblclick']) document.addEventListener(ev, e => e.preventDefault());
  document.addEventListener('touchmove', e => e.preventDefault(), { passive: false });
  const fs = document.getElementById('fs');
  const root = document.documentElement;
  const isFull = () => document.fullscreenElement || document.webkitFullscreenElement;
  const toast = t => { const m = document.getElementById('msg'); const old = m.textContent; m.textContent = t; setTimeout(() => { if (m.textContent === t) m.textContent = old === t ? '' : old; }, 3500); };
  let lastFs = 0;
  const toggleFs = async e => {
    e.preventDefault(); e.stopPropagation();
    if (performance.now() - lastFs < 400) return;       // pointerup + click both fire on some browsers
    lastFs = performance.now();
    try {
      if (isFull()) { (document.exitFullscreen || document.webkitExitFullscreen).call(document); return; }
      const req = root.requestFullscreen || root.webkitRequestFullscreen;
      if (!req) { toast('Fullscreen is not supported here - use Share > Add to Home Screen'); return; }
      await req.call(root, { navigationUI: 'hide' });
      try { await screen.orientation.lock('landscape'); } catch (err) { }
    } catch (err) { toast('Fullscreen blocked: ' + (err && err.message || err)); }
  };
  fs.addEventListener('pointerup', toggleFs);
  fs.addEventListener('click', toggleFs);
  fs.addEventListener('pointerdown', e => e.stopPropagation());
  if (!('ontouchstart' in window) && !navigator.maxTouchPoints) document.body.classList.add('desktop');
})();
