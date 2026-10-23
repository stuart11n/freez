'use strict';
// On-screen controls: each button synthesises the same keyboard events the game already listens for.
(() => {
  const send = (code, down) => window.dispatchEvent(new KeyboardEvent(down ? 'keydown' : 'keyup', { code, bubbles: true, cancelable: true }));
  document.querySelectorAll('[data-code]').forEach(btn => {
    const code = btn.dataset.code;
    const active = new Set();
    const down = e => {
      e.preventDefault();
      btn.setPointerCapture(e.pointerId);
      if (!active.size) { send(code, true); btn.classList.add('on'); if (navigator.vibrate) navigator.vibrate(8); }
      active.add(e.pointerId);
    };
    const up = e => {
      active.delete(e.pointerId);
      if (!active.size) { send(code, false); btn.classList.remove('on'); }
    };
    btn.addEventListener('pointerdown', down);
    btn.addEventListener('pointerup', up);
    btn.addEventListener('pointercancel', up);
    btn.addEventListener('lostpointercapture', up);
  });
  document.getElementById('crt').addEventListener('pointerdown', e => { e.preventDefault(); toggleScanlines(); });
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
