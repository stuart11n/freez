'use strict';
// Chiptune engine: two pulse channels, a triangle bass and a noise drum channel, driven by a small step
// sequencer on top of Web Audio. Tracks are generated from chord progressions + melody templates.

const Music = (() => {
  let ctx = null, master = null, noiseBuf = null, pulse25 = null, pulse50 = null;
  let current = null, wanted = null, timer = null, step = 0, nextTime = 0, jingle = null;

  const NOTE = { C: 0, 'C#': 1, D: 2, 'D#': 3, E: 4, F: 5, 'F#': 6, G: 7, 'G#': 8, A: 9, 'A#': 10, B: 11 };
  const midi = (n, oct) => 12 * (oct + 1) + NOTE[n];
  const hz = m => 440 * Math.pow(2, (m - 69) / 12);
  const CH = {   // chord tones (semitones above root)
    Am: [9, 12, 16], F: [5, 9, 12], C: [0, 4, 7], G: [7, 11, 14], Dm: [2, 5, 9], Em: [4, 7, 11], E: [4, 8, 11],
  };
  const ROOT = { Am: 9, F: 5, C: 0, G: 7, Dm: 2, Em: 4, E: 4 };

  // melody templates: indexes into the chord tones (0..2 = root/3rd/5th, 3..5 = same an octave up), null = rest
  const T = {
    A: [2, null, null, 2, null, 0, null, 2, 3, null, null, 2, null, 1, null, null],
    B: [0, 1, 2, 3, 2, 1, 2, 3, null, null, 4, null, 3, null, 2, null],
    C: [4, null, 3, null, 2, null, 3, null, 4, null, 5, null, 4, 3, 2, null],
    D: [3, null, 2, null, 3, null, 4, null, 5, null, null, 4, null, 3, 2, null],
  };

  function build(o) {
    const lead = [], arp = [], bass = [], drums = [];
    o.bars.forEach((chord, bi) => {
      const tones = CH[chord], root = ROOT[chord];
      const tpl = T[o.melody[bi % o.melody.length]];
      for (let s = 0; s < 16; s++) {
        const idx = tpl[s];
        lead.push(o.noLead || idx === null ? null : 60 + tones[idx % 3] + 12 * Math.floor(idx / 3) + (o.leadOct || 0));
        arp.push(o.arp ? 72 + [tones[0], tones[1], tones[2], tones[1]][s % 4] - 12 : null);
        const bp = [0, null, 0, null, 12, null, 0, null, 0, null, 0, null, 7, null, 12, null][s];
        bass.push(bp === null || o.noBass ? null : 36 + root + bp);
        let d = '';
        if (!o.noDrums) {
          if (s === 0 || s === 8 || (s === 10 && bi % 2)) d += 'k';
          if (s === 4 || s === 12) d += 's';
          if (bi % 4 === 3 && s >= 13) d += 's';
          if (s % 2 === 0) d += 'h';
        } else if (s % 4 === 0) d += 'h';
        drums.push(d);
      }
    });
    return { bpm: o.bpm, len: lead.length, lead, arp, bass, drums, vol: o.vol || 1 };
  }

  const TRACKS = {
    level: build({
      bpm: 152,
      bars: ['Am', 'F', 'C', 'G', 'Am', 'F', 'C', 'G', 'Dm', 'G', 'C', 'Am', 'F', 'G', 'Em', 'E'],
      melody: ['A', 'A', 'A', 'A', 'B', 'B', 'B', 'B', 'C', 'C', 'D', 'D', 'C', 'C', 'B', 'D'],
      arp: true,
    }),
    menu: build({
      bpm: 100, bars: ['Am', 'F', 'C', 'G', 'Am', 'F', 'G', 'E'], melody: ['D', 'D', 'A', 'A', 'D', 'D', 'C', 'A'],
      noDrums: true, arp: true, vol: .85,
    }),
  };

  // one-shot jingles: [midi, beats]
  const JINGLES = {
    win: { bpm: 132, notes: [[72, .5], [76, .5], [79, .5], [84, 1], [79, .5], [84, 2], [null, .5], [74, .5], [77, .5], [81, .5], [86, 1], [81, .5], [86, 1.5],
      [null, .5], [76, .5], [79, .5], [84, .5], [88, 1], [84, .5], [88, 1], [91, .5], [96, 3]] },
    over: { bpm: 90, notes: [[64, 1], [62, 1], [60, 1], [57, 1], [null, .5], [56, .5], [57, 3]] },
    clear: { bpm: 150, notes: [[72, .5], [76, .5], [79, .5], [84, 1.5]] },
  };

  function noise() {
    const b = ctx.createBuffer(1, ctx.sampleRate, ctx.sampleRate), d = b.getChannelData(0);
    for (let i = 0; i < d.length; i++) d[i] = Math.random() * 2 - 1;
    return b;
  }
  function pulseWave(duty) {
    const n = 32, real = new Float32Array(n), imag = new Float32Array(n);
    for (let k = 1; k < n; k++) real[k] = (2 / (k * Math.PI)) * Math.sin(k * Math.PI * duty);
    return ctx.createPeriodicWave(real, imag);
  }
  function ensure() {
    if (ctx) return true;
    if (typeof actx === 'undefined' || !actx) return false;
    ctx = actx;
    master = ctx.createGain(); master.gain.value = .16; master.connect(ctx.destination);
    noiseBuf = noise(); pulse25 = pulseWave(.25); pulse50 = pulseWave(.5);
    return true;
  }

  function tone(wave, freq, t, dur, vol, type) {
    const o = ctx.createOscillator(), g = ctx.createGain();
    if (type) o.type = type; else o.setPeriodicWave(wave);
    o.frequency.setValueAtTime(freq, t);
    g.gain.setValueAtTime(0, t);
    g.gain.linearRampToValueAtTime(vol, t + .006);
    g.gain.exponentialRampToValueAtTime(.0001, t + Math.max(.03, dur));
    o.connect(g); g.connect(master);
    o.start(t); o.stop(t + dur + .05);
  }
  function drum(kind, t) {
    if (kind === 'k') {
      const o = ctx.createOscillator(), g = ctx.createGain();
      o.frequency.setValueAtTime(170, t); o.frequency.exponentialRampToValueAtTime(42, t + .11);
      g.gain.setValueAtTime(.9, t); g.gain.exponentialRampToValueAtTime(.001, t + .14);
      o.connect(g); g.connect(master); o.start(t); o.stop(t + .16);
      return;
    }
    const src = ctx.createBufferSource(), f = ctx.createBiquadFilter(), g = ctx.createGain();
    src.buffer = noiseBuf;
    if (kind === 's') { f.type = 'bandpass'; f.frequency.value = 1900; f.Q.value = .8; g.gain.setValueAtTime(.55, t); g.gain.exponentialRampToValueAtTime(.001, t + .11); }
    else { f.type = 'highpass'; f.frequency.value = 7500; g.gain.setValueAtTime(.18, t); g.gain.exponentialRampToValueAtTime(.001, t + .04); }
    src.connect(f); f.connect(g); g.connect(master);
    src.start(t, Math.random() * .5); src.stop(t + .15);
  }

  function schedule() {
    if (!ctx || !current) return;
    const tr = current;
    const sd = 60 / tr.bpm / 4;
    while (nextTime < ctx.currentTime + .18) {
      const i = step % tr.len, t = Math.max(nextTime, ctx.currentTime);
      const l = tr.lead[i], a = tr.arp[i], b = tr.bass[i], d = tr.drums[i];
      if (l !== null) tone(pulse50, hz(l), t, sd * 1.9, .55 * tr.vol);
      if (a !== null) tone(pulse25, hz(a), t, sd * .9, .22 * tr.vol);
      if (b !== null) tone(null, hz(b), t, sd * 1.9, .9 * tr.vol, 'triangle');
      for (const k of d) drum(k, t);
      nextTime += sd; step++;
    }
  }
  function playJingle(j, loop) {
    let t = ctx.currentTime + .08;
    const bd = 60 / j.bpm;
    for (const [m, beats] of j.notes) {
      if (m !== null) {
        tone(pulse50, hz(m), t, beats * bd * .95, .6);
        tone(pulse25, hz(m - 12), t, beats * bd * .95, .25);
        if (beats >= 1) tone(null, hz(m - 24), t, beats * bd * .95, .7, 'triangle');
      }
      t += beats * bd;
    }
  }

  function stopTimer() { if (timer) { clearInterval(timer); timer = null; } }
  function start(name) {
    wanted = name;
    if (!Settings.music || !ensure()) return;
    if (name === current && timer) return;
    stopTimer(); current = null;
    if (!name) return;
    if (JINGLES[name]) { playJingle(JINGLES[name]); return; }
    current = TRACKS[name]; step = 0; nextTime = ctx.currentTime + .08;
    timer = setInterval(schedule, 25); schedule();
  }
  return {
    play: start,
    stop() { wanted = null; stopTimer(); current = null; },
    ready() { if (wanted) start(wanted); },                 // called once the AudioContext exists
    refresh() { if (Settings.music) start(wanted); else { stopTimer(); current = null; } },
  };
})();
