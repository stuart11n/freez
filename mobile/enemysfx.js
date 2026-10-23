'use strict';
// Ambient enemy voices (synthesised, nothing to download): birds squawk, the yellow hoppers squish when they land.
const EnemySFX = (() => {
  let noise = null, lastBird = 0, lastSquish = 0;
  const cue = new Int16Array(200), lastIdx = new Int8Array(200).fill(-1);

  function noiseBuf() {
    if (noise) return noise;
    const n = actx.sampleRate >> 1, b = actx.createBuffer(1, n, actx.sampleRate), d = b.getChannelData(0);
    for (let i = 0; i < n; i++) d[i] = Math.random() * 2 - 1;
    return (noise = b);
  }
  function out(pan, vol) {
    const g = actx.createGain(); g.gain.value = vol;
    if (actx.createStereoPanner) { const p = actx.createStereoPanner(); p.pan.value = pan; g.connect(p); p.connect(actx.destination); }
    else g.connect(actx.destination);
    return g;
  }
  function env(g, t, a, hold, rel, peak) {
    g.gain.setValueAtTime(0, t);
    g.gain.linearRampToValueAtTime(peak, t + a);
    g.gain.setValueAtTime(peak, t + a + hold);
    g.gain.exponentialRampToValueAtTime(.001, t + a + hold + rel);
  }

  // "ka-RAWK": two harsh falling chirps with a fast wobble, formant-filtered, plus a little breath noise
  function squawk(vol, pan) {
    const t0 = actx.currentTime + .005, base = 780 + Math.random() * 260, o = out(pan, vol);
    [[0, 1.0], [.14, 1.35]].forEach(([dt, k]) => {
      const t = t0 + dt, osc = actx.createOscillator(), lfo = actx.createOscillator(), lg = actx.createGain();
      const bp = actx.createBiquadFilter(), g = actx.createGain();
      osc.type = 'sawtooth';
      osc.frequency.setValueAtTime(base * k * 1.7, t);
      osc.frequency.exponentialRampToValueAtTime(base * k * .85, t + .11);
      lfo.frequency.value = 42; lg.gain.value = 70; lfo.connect(lg); lg.connect(osc.frequency);
      bp.type = 'bandpass'; bp.frequency.value = 1700; bp.Q.value = 2.6;
      env(g, t, .006, .05, .09, 1);
      osc.connect(bp); bp.connect(g); g.connect(o);
      osc.start(t); lfo.start(t); osc.stop(t + .2); lfo.stop(t + .2);
    });
    const ns = actx.createBufferSource(), hp = actx.createBiquadFilter(), ng = actx.createGain();
    ns.buffer = noiseBuf(); hp.type = 'highpass'; hp.frequency.value = 2400;
    env(ng, t0, .004, .02, .12, .18);
    ns.connect(hp); hp.connect(ng); ng.connect(o); ns.start(t0, Math.random() * .3); ns.stop(t0 + .25);
  }

  // "sp-lurp": a wet pitch bend (up then down) with a resonant noise burst
  function squish(vol, pan) {
    const t = actx.currentTime + .005, o = out(pan, vol);
    const osc = actx.createOscillator(), g = actx.createGain();
    osc.type = 'triangle';
    osc.frequency.setValueAtTime(95, t);
    osc.frequency.exponentialRampToValueAtTime(340, t + .05);
    osc.frequency.exponentialRampToValueAtTime(60, t + .17);
    env(g, t, .005, .04, .13, .9);
    osc.connect(g); g.connect(o); osc.start(t); osc.stop(t + .22);
    const ns = actx.createBufferSource(), bp = actx.createBiquadFilter(), ng = actx.createGain();
    ns.buffer = noiseBuf(); bp.type = 'bandpass'; bp.Q.value = 7;
    bp.frequency.setValueAtTime(500, t); bp.frequency.exponentialRampToValueAtTime(1800, t + .06); bp.frequency.exponentialRampToValueAtTime(420, t + .15);
    env(ng, t, .004, .03, .11, .7);
    ns.connect(bp); bp.connect(ng); ng.connect(o); ns.start(t, Math.random() * .3); ns.stop(t + .2);
  }

  // called for every enemy that is drawn this frame. rx,ry = on-screen position of the sprite
  function voice(e, ctrl, typ, status, frame, rx, ry) {
    if (status !== 0 || !(GameSetup & 1) || !actx || actx.state !== 'running') return;
    const dx = rx + 16 - 160, dy = ry + 16 - 100, dist = Math.hypot(dx, dy);
    if (dist > 300) return;
    const near = 1 - dist / 300, pan = Math.max(-1, Math.min(1, dx / 170)), now = performance.now();
    if (ctrl === 1 && typ === 1) {                         // yellow hopper: squish as it lands (squat frame)
      const idx = frame >> 2;
      if (idx === 7 && lastIdx[e] !== 7 && now - lastSquish > 130) { lastSquish = now; squish(.34 * near * near + .05, pan); }
      lastIdx[e] = idx;
    } else if (ctrl === 2 && typ === 0) {                  // bird: squawks now and then
      if (cue[e] > 0) { cue[e]--; return; }
      cue[e] = 150 + (Math.random() * 260 | 0);
      if (now - lastBird > 500) { lastBird = now; squawk(.3 * near + .04, pan); }
    }
  }
  function reset() { cue.fill(0); lastIdx.fill(-1); for (let i = 0; i < 200; i++) cue[i] = (Math.random() * 120) | 0; }
  return { voice, reset, squawk, squish };
})();
