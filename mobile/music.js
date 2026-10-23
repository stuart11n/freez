'use strict';
// Tracker-style music player (think ProTracker / FastTracker 2 era).
//
//  * Songs are an order list of patterns; a pattern is rows x channels, a row is 6 "ticks" (speed) long and the
//    tick length comes from the BPM exactly like a .MOD player (tick = 2.5 / BPM seconds).
//  * Instruments are tiny 8-bit looped waveforms / one-shot drum samples, pitched by changing the playback rate,
//    with hard-panned Amiga-style stereo (L R R L).
//  * Cells carry a note, an instrument, a volume and an effect. Effects run once per tick:
//      a<xy>  arpeggio (0, x, y semitones)    w<xy>  vibrato (speed x, depth y)    u<xx>/d<xx>  slide up/down
//    plus  ^^  note cut.  Volume changes are stepped per tick, which gives the classic tracker "zipper" envelopes.

const Music = (() => {
  let ctx = null, master = null, bus = null, inst = null;
  let song = null, wanted = null, timer = null;
  let order = 0, row = 0, tick = 0, nextTime = 0, chans = [];

  const SR = 22050;
  const NOTES = { 'C-': 0, 'C#': 1, 'D-': 2, 'D#': 3, 'E-': 4, 'F-': 5, 'F#': 6, 'G-': 7, 'G#': 8, 'A-': 9, 'A#': 10, 'B-': 11 };
  const noteNum = s => NOTES[s.slice(0, 2)] + 12 * (+s[2] + 1);          // MIDI number, C-4 = 60
  const hz = m => 440 * Math.pow(2, (m - 69) / 12);

  // ---------------------------------------------------------------- instruments (synthesised "samples")
  const q8 = v => Math.round(Math.max(-1, Math.min(1, v)) * 127) / 127;     // 8-bit quantisation = tracker crunch
  function cycle(len, fn) {
    const b = ctx.createBuffer(1, len, SR), d = b.getChannelData(0);
    for (let i = 0; i < len; i++) d[i] = q8(fn(i / len));
    return b;
  }
  function oneShot(sec, fn) {
    const n = Math.floor(SR * sec), b = ctx.createBuffer(1, n, SR), d = b.getChannelData(0);
    for (let i = 0; i < n; i++) d[i] = q8(fn(i / SR, i / n));
    return b;
  }
  function makeInstruments() {
    const noiseSeed = (() => { let s = 1; return () => ((s = (s * 16807) % 2147483647) / 2147483647) * 2 - 1; })();
    return {
      // base: the MIDI note the stored cycle plays at when playbackRate = 1.  loop: cycle length in samples
      1: { name: 'saw lead', buf: cycle(64, p => 1 - 2 * p - 0.18 * Math.sin(p * 6.2832 * 3)), loop: 1, len: 64, decay: .985, att: 1, hold: .0 },
      2: { name: 'pulse arp', buf: cycle(32, p => (p < .25 ? .9 : -.9)), loop: 1, len: 32, decay: .992, att: 1 },
      3: { name: 'sub bass', buf: cycle(64, p => Math.sin(p * 6.2832) * .75 + Math.sin(p * 6.2832 * 3) * .2 + Math.sin(p * 6.2832 * 2) * .08), loop: 1, len: 64, decay: .975, att: 1 },
      4: { name: 'pad', buf: cycle(128, p => (Math.sin(p * 6.2832) + .5 * Math.sin(p * 6.2832 * 3) + .25 * Math.sin(p * 6.2832 * 5) + .12 * Math.sin(p * 6.2832 * 8.02)) * .55), loop: 1, len: 128, decay: 1, att: .25, swell: 8 },
      5: { name: 'kick', buf: oneShot(.2, (t, u) => Math.sin(2 * Math.PI * (48 * t + 130 * (1 - Math.exp(-t * 28)) / 28 * 0 + 92 * (1 - Math.exp(-t * 32)) / 32)) * Math.pow(1 - u, 1.6)), loop: 0 },
      6: { name: 'snare', buf: oneShot(.18, (t, u) => (noiseSeed() * .8 + Math.sin(2 * Math.PI * 190 * t) * .5) * Math.pow(1 - u, 2.2)), loop: 0 },
      7: { name: 'hat', buf: oneShot(.06, (t, u) => noiseSeed() * Math.pow(1 - u, 3) * .8), loop: 0 },
      8: { name: 'tom', buf: oneShot(.22, (t, u) => Math.sin(2 * Math.PI * (120 * t - 60 * Math.pow(t, 2) * 3)) * Math.pow(1 - u, 1.4)), loop: 0 },
    };
  }

  // ---------------------------------------------------------------- pattern text
  // Row cell: NOTE [i<n>] [v<0-64>] [a<xy> | w<xy> | u<xx> | d<xx>]     '.' = nothing    '^^' = cut
  function parseCell(tok) {
    if (!tok || tok === '.') return null;
    if (tok === '^^') return { cut: 1 };
    const m = /^([A-G][-#]\d)((?:i\d+|v\d+|a[0-9a-f]{3}|w[0-9a-f]{2}|[ud][0-9a-f]{2})*)$/.exec(tok);
    if (!m) throw new Error('bad cell ' + tok);
    const c = { note: noteNum(m[1]) };
    const hx = ch => parseInt(ch, 16);
    for (const e of m[2].match(/i\d+|v\d+|a[0-9a-f]{3}|w[0-9a-f]{2}|[ud][0-9a-f]{2}/g) || []) {
      const k = e[0], v = e.slice(1);
      if (k === 'i') c.inst = +v; else if (k === 'v') c.vol = +v;
      else if (k === 'a') c.arp = [hx(v[1]), hx(v[2])];
      else if (k === 'w') c.vib = [hx(v[0]), hx(v[1])];
      else if (k === 'u') c.slide = parseInt(v, 16) / 16; else if (k === 'd') c.slide = -parseInt(v, 16) / 16;
    }
    return c;
  }
  // channels: array of strings of space separated cells (one cell per row). All the same number of rows.
  function pattern(chs) {
    const cols = chs.map(s => s.trim().split(/\s+/).map(parseCell));
    const rows = cols[0].length;
    cols.forEach((c, i) => { if (c.length !== rows) throw new Error('pattern channel ' + i + ' has ' + c.length + ' rows, expected ' + rows); });
    return { rows, cols };
  }

  // ---- composition helpers: build one 8-row chord block per chord
  const N = (name, o) => name + o;
  const CHORDS = {   // [root name, quality arp, bass root octave notes]
    Am: ['A', '037'], F: ['F', '047'], C: ['C', '047'], G: ['G', '047'], Dm: ['D', '037'], Em: ['E', '037'], E: ['E', '047'], Bb: ['A#', '047'],
  };
  const dots = n => Array(n).fill('.').join(' ');
  function arpRow(ch, oct) {   // pulse chord arpeggio held for 8 rows
    const [r, q] = CHORDS[ch];
    return `${N(r + (r.length === 1 ? '-' : ''), oct)}i2a${q}v34 ${dots(7)}`;
  }
  function padRow(ch, oct) {
    const [r] = CHORDS[ch];
    return `${N(r + (r.length === 1 ? '-' : ''), oct)}i4w32v30 ${dots(7)}`;
  }
  function bassRow(ch, v) {
    const [r] = CHORDS[ch], b = r + (r.length === 1 ? '-' : '');
    const o3 = N(b, 2), o4 = N(b, 3);
    const fifth = { A: 'E-3', F: 'C-3', C: 'G-2', G: 'D-3', D: 'A-2', E: 'B-2' }[r[0]];
    if (v === 1) return `${o3}i3v60 . ${o3}i3v52 . ${o4}i3v56 . ${o3}i3v52 ${fifth ? fifth + 'i3v50' : '.'}`;
    return `${o3}i3v60 . ${o4}i3v48 ${o3}i3v52 . ${o3}i3v52 ${o4}i3v48 .`;
  }
  const drumRow = {
    a: 'C-3i5v64 . C-3i7v34 . C-3i6v56 . C-3i7v34 C-3i5v40',
    b: 'C-3i5v64 C-3i7v28 C-3i7v34 . C-3i6v56 . C-3i7v34 C-3i7v28',
    f: 'C-3i5v64 . C-3i7v34 . C-3i6v56 C-3i6v46 C-3i6v52 C-3i6v60',     // fill
    g: 'C-3i5v64 . C-3i7v34 . C-3i6v56 . G-2i8v50 G-2i8v56',
  };
  const Z = (a, b, c, d) => [a, b, c, d].join(' ');

  function levelPattern(chs, lead, drums, vb) {
    return pattern([
      lead,
      Z(...chs.map(c => arpRow(c, 4))),
      Z(...chs.map(c => bassRow(c, vb))),
      Z(...drums.map(d => drumRow[d])),
      Z(...chs.map(c => padRow(c, 3))),
    ]);
  }
  const L = {  // lead lines, 4 chords x 8 rows
    M1: 'E-5i1v50 . A-5 . G-5 . E-5 .   F-5 . A-5 . C-6 . A-5 .   G-5 . E-5 . G-5 . C-6w32 .   D-6 . B-5 . G-5 . B-5 .',
    M2: 'A-4i1v46 C-5 E-5 A-5 E-5 C-5 E-5 A-5   F-4 A-4 C-5 F-5 C-5 A-4 C-5 F-5   E-5 G-5 C-6 G-5 E-5 G-5 C-6 E-6   D-5 G-5 B-5 D-6 B-5 G-5 B-5 D-6',
    M3: 'A-5i1v52 . F-5 . D-5 . F-5 A-5   B-5 . G-5 . D-5 . G-5 B-5   C-6 . E-6 . G-5 . E-6 .   A-5 . C-6 . E-6w43 . C-6 A-5',
    M4: 'C-6i1v52 . A-5 . F-5 . A-5 C-6   D-6 . B-5 . G-5 . D-6 B-5   E-6 . B-5 . G-5 . B-5 E-6   G#5 . B-5 . E-6w43 . D-6 B-5',
  };
  const lead = s => s.replace(/\s{2,}/g, ' ');

  // ---------------------------------------------------------------- songs
  function makeSongs() {
    const P = {
      A: levelPattern(['Am', 'F', 'C', 'G'], lead(L.M1), ['a', 'a', 'a', 'f'], 1),
      B: levelPattern(['Am', 'F', 'C', 'G'], lead(L.M2), ['a', 'b', 'a', 'f'], 2),
      C: levelPattern(['Dm', 'G', 'C', 'Am'], lead(L.M3), ['a', 'a', 'b', 'a'], 1),
      D: levelPattern(['F', 'G', 'Em', 'E'], lead(L.M4), ['b', 'a', 'a', 'f'], 2),
    };
    const level = { bpm: 125, speed: 6, orders: [P.A, P.A, P.B, P.B, P.C, P.D, P.B, P.D], loop: true, vol: 1 };

    // menu: slow and spacious, no drums, arp + pad + sparse lead
    const menuLead = 'E-5i1v36 . . . . . . .   C-5 . . . . . D-5 .   E-5 . . . G-5 . . .   B-4 . . . D-5 . . .';
    const quiet = (chs, ld, hat) => pattern([
      ld,
      Z(...chs.map(c => arpRow(c, 4))),
      Z(...chs.map(c => `${CHORDS[c][0]}${CHORDS[c][0].length === 1 ? '-' : ''}2i3v50 . . . . . . .`.replace('A#-', 'A#'))),
      Z(...chs.map(() => hat)),
      Z(...chs.map(c => padRow(c, 3))),
    ]);
    const hat = 'C-3i7v18 . . . C-3i7v14 . . .';
    const menu = {
      bpm: 92, speed: 6, loop: true, vol: .9,
      orders: [quiet(['Am', 'F', 'C', 'G'], lead(menuLead), hat), quiet(['Am', 'F', 'G', 'E'], lead(menuLead.replace('E-5i1v36', 'C-5i1v36')), hat)],
    };

    // win fanfare (plays once)
    const win = {
      bpm: 132, speed: 6, loop: false, vol: 1,
      orders: [pattern([
        'C-5i1v56 E-5 G-5 C-6 . G-5 C-6 .   D-5 F-5 A-5 D-6 . A-5 D-6 .   E-5 G-5 C-6 E-6 . C-6 E-6 .   G-5 B-5 D-6 G-6 . . . .',
        'C-4i2a047v40 . . . . . . .   D-4i2a037v40 . . . . . . .   C-4i2a047v40 . . . . . . .   G-4i2a047v40 . . . . . . .',
        'C-2i3v60 . . . . . . .   D-2i3v60 . . . . . . .   C-2i3v60 . . . . . . .   G-2i3v60 . . . . . . .',
        'C-3i5v64 . C-3i7v30 . C-3i6v52 . C-3i7v30 .   C-3i5v64 . C-3i7v30 . C-3i6v52 . C-3i7v30 .   C-3i5v64 . C-3i7v30 . C-3i6v52 C-3i6v56 C-3i6v60 C-3i6v64   C-3i5v64 . . . . . . .',
        'C-3i4v34w32 . . . . . . .   D-3i4v34w32 . . . . . . .   C-3i4v34w32 . . . . . . .   G-3i4v34w32 . . . . . . .',
      ]), pattern([
        'C-6i1v60w43 . . . . . . .   . . . . . . . .   . . . . . . . .   ^^ . . . . . . .',
        'C-4i2a047v40 . . . . . . .   . . . . . . . .   . . . . . . . .   ^^ . . . . . . .',
        'C-2i3v64 . . . . . . .   . . . . . . . .   . . . . . . . .   ^^ . . . . . . .',
        'C-3i5v64 . . . . . . .   . . . . . . . .   . . . . . . . .   ^^ . . . . . . .',
        'C-3i4v38w32 . . . . . . .   . . . . . . . .   . . . . . . . .   ^^ . . . . . . .',
      ])],
    };
    // game over (plays once): falling minor line
    const over = {
      bpm: 84, speed: 6, loop: false, vol: 1,
      orders: [pattern([
        'E-5i1v56 . . . D-5 . . .   C-5 . . . A-4 . . .   G#4 . . . A-4i1w55 . . .   . . . . ^^ . . .',
        'A-3i2a037v36 . . . . . . .   F-3i2a047v36 . . . . . . .   E-3i2a047v36 . . . A-3i2a037v36 . . .   . . . . ^^ . . .',
        'A-2i3v60 . . . . . . .   F-2i3v60 . . . . . . .   E-2i3v60 . . . A-2i3v60 . . .   . . . . ^^ . . .',
        'C-3i5v64 . . . C-3i6v50 . . .   C-3i5v64 . . . C-3i6v50 . . .   C-3i5v64 . . . C-3i6v50 . . .   G-2i8v60 . G-2i8v50 . . . . .',
        'A-3i4v30w32 . . . . . . .   F-3i4v30w32 . . . . . . .   E-3i4v30w32 . . . . . . .   . . . . ^^ . . .',
      ])],
    };
    return { level, menu, win, over };
  }

  // ---------------------------------------------------------------- player
  const PAN = [-.55, .55, .55, -.55, -.2];       // lead L, arp R, bass R, drums L, pad (centre-left)
  function ensure() {
    if (ctx) return true;
    if (typeof actx === 'undefined' || !actx) return false;
    ctx = actx;
    master = ctx.createGain(); master.gain.value = .2; master.connect(ctx.destination);
    inst = makeInstruments();
    song_cache = makeSongs();
    return true;
  }
  let song_cache = null;

  function newChannel(i) {
    const g = ctx.createGain(), p = ctx.createStereoPanner ? ctx.createStereoPanner() : null;
    if (p) { p.pan.value = PAN[i] || 0; g.connect(p); p.connect(master); } else g.connect(master);
    return { gain: g, src: null, note: 0, inst: 0, vol: 64, arp: null, vib: null, slide: 0, age: 0, bend: 0, level: 1, cut: false };
  }
  function trigger(ch, cell, t) {
    if (cell.cut) { stopVoice(ch, t); ch.cut = true; return; }
    if (cell.inst) ch.inst = cell.inst;
    const ins = inst[ch.inst]; if (!ins) return;
    if (cell.vol !== undefined) ch.vol = cell.vol;
    stopVoice(ch, t);
    const s = ctx.createBufferSource();
    s.buffer = ins.buf; s.loop = !!ins.loop;
    s.connect(ch.gain);
    s.start(t);
    ch.src = s; ch.note = cell.note; ch.arp = cell.arp || null; ch.vib = cell.vib || null; ch.slide = cell.slide || 0;
    ch.age = 0; ch.bend = 0; ch.level = ins.att !== undefined && ins.swell ? ins.att : 1; ch.cut = false; ch.ins = ins;
  }
  function stopVoice(ch, t) {
    if (ch.src) { try { ch.gain.gain.setValueAtTime(0, t); ch.src.stop(t + .005); } catch (e) { } ch.src = null; }
  }
  function tickVoice(ch, t, tickDur) {
    if (!ch.src || !ch.ins) return;
    const ins = ch.ins;
    let semis = 0;
    if (ch.arp) semis += [0, ch.arp[0], ch.arp[1]][ch.age % 3];
    if (ch.vib) semis += Math.sin(ch.age * ch.vib[0] * .55) * ch.vib[1] * .12;
    ch.bend += ch.slide;
    semis += ch.bend;
    if (ins.loop) {
      const f = hz(ch.note + semis);
      ch.src.playbackRate.setValueAtTime(f / (SR / ins.len), t);
    } else if (ch.age === 0) {
      // drum "samples": note picks the pitch (C-3 = original speed)
      ch.src.playbackRate.setValueAtTime(Math.pow(2, (ch.note - 48) / 12), t);
    }
    // per-tick stepped volume envelope
    if (ins.swell) ch.level = Math.min(1, ch.level + (1 - ins.att) / ins.swell); else if (ch.age > 0) ch.level *= ins.decay || 1;
    ch.gain.gain.setValueAtTime((ch.vol / 64) * ch.level, t);
    ch.age++;
  }

  function processTick() {
    const sd = song.orders[order], tickDur = 2.5 / song.bpm;
    if (tick === 0) {
      sd.cols.forEach((col, ci) => {
        const cell = col[row];
        if (!chans[ci]) chans[ci] = newChannel(ci);
        if (cell) trigger(chans[ci], cell, nextTime);
      });
    }
    chans.forEach(ch => tickVoice(ch, nextTime, tickDur));
    nextTime += tickDur;
    if (++tick >= song.speed) {
      tick = 0;
      if (++row >= sd.rows) {
        row = 0; order++;
        if (order >= song.orders.length) {
          if (song.loop) order = 0;
          else { song = null; return false; }
        }
      }
    }
    return true;
  }
  function schedule() {
    if (!ctx || !song) return;
    while (song && nextTime < ctx.currentTime + .2) if (!processTick()) break;
    if (!song) stopTimer();
  }
  function stopTimer() { if (timer) { clearInterval(timer); timer = null; } }
  function silence() {
    const t = ctx ? ctx.currentTime : 0;
    chans.forEach(ch => stopVoice(ch, t));
    chans = [];
  }
  function start(name) {
    wanted = name;
    if (!Settings.music || !ensure()) return;
    if (song && song === song_cache[name] && timer) return;
    stopTimer(); silence(); song = null;
    if (!name || !song_cache[name]) return;
    song = song_cache[name]; order = 0; row = 0; tick = 0; nextTime = ctx.currentTime + .06;
    master.gain.value = .2 * (song.vol || 1);
    timer = setInterval(schedule, 25); schedule();
  }
  return {
    play: start,
    get debugMaster() { return master; },
    stop() { wanted = null; stopTimer(); if (ctx) silence(); song = null; },
    ready() { if (wanted) start(wanted); },
    refresh() { if (Settings.music) start(wanted); else { stopTimer(); if (ctx) silence(); song = null; } },
  };
})();
