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
  fs.addEventListener('click', () => {
    const d = document.documentElement;
    if (document.fullscreenElement) document.exitFullscreen();
    else if (d.requestFullscreen) d.requestFullscreen().then(() => screen.orientation && screen.orientation.lock && screen.orientation.lock('landscape').catch(() => { })).catch(() => { });
  });
  if (!('ontouchstart' in window) && !navigator.maxTouchPoints) document.body.classList.add('desktop');
})();
