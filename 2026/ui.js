'use strict';
// Settings panel + hotkeys for the mobile build (CRT, scaling filter, music, sound effects).
const UI = (() => {
  const $ = id => document.getElementById(id);
  const toast = t => {
    const m = $('toast'); m.textContent = t; m.classList.add('show');
    clearTimeout(toast.t); toast.t = setTimeout(() => m.classList.remove('show'), 1600);
  };
  function refresh() {
    $('opt-crt').textContent = 'CRT: ' + CRT_NAMES[Settings.crt];
    $('opt-scale').textContent = 'Scaling: ' + SCALE_NAMES[Settings.scale];
    $('opt-music').textContent = 'Music: ' + (Settings.music ? 'On' : 'Off');
    $('opt-sfx').textContent = 'Sound FX: ' + ((GameSetup & 1) ? 'On' : 'Off');
    document.body.classList.toggle('crt', Settings.crt > 0);
  }
  const act = {
    crt() { Settings.crt = (Settings.crt + 1) % 3; Settings.save(); refresh(); toast('CRT: ' + CRT_NAMES[Settings.crt]); },
    scale() { Settings.scale = (Settings.scale + 1) % 3; Settings.save(); refresh(); toast('Scaling: ' + SCALE_NAMES[Settings.scale]); },
    music() { Settings.music = !Settings.music; Settings.save(); Music.refresh(); refresh(); toast('Music ' + (Settings.music ? 'on' : 'off')); },
    sfx() { GameSetup ^= 1; saveSetup(); refresh(); toast('Sound FX ' + ((GameSetup & 1) ? 'on' : 'off')); },
  };
  function init() {
    for (const k of Object.keys(act)) {
      const b = $('opt-' + k);
      b.addEventListener('pointerdown', e => { e.preventDefault(); e.stopPropagation(); });
      b.addEventListener('click', e => { e.preventDefault(); e.stopPropagation(); act[k](); });
    }
    const gear = $('gear'), panel = $('panel');
    const toggle = e => { e.preventDefault(); e.stopPropagation(); panel.classList.toggle('open'); refresh(); };
    gear.addEventListener('pointerdown', e => { e.preventDefault(); e.stopPropagation(); });
    gear.addEventListener('click', toggle);
    $('crt').addEventListener('pointerdown', e => { e.preventDefault(); e.stopPropagation(); act.crt(); });
    refresh();
  }
  return {
    init, refresh, toast,
    hotkey(code) { if (code === 'F6') act.crt(); else if (code === 'F7') act.scale(); else if (code === 'F8') act.music(); },
  };
})();
