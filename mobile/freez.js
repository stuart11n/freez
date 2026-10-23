'use strict';
// FREEZ (c) Digital Synapse 1996 -- JavaScript port of the Borland C / TASM source.
// The structure follows the original files: FREEZE.CPP (main + GameLoop), CORE.H (loaders/menus
// helpers), MAIN.CPP (pmain), TILE.CPP (ptile), ENEMY.CPP (psprite32), BONUS.CPP (psprite16),
// PBRIDGE.CPP, MENUS.CPP, MISC.CPP, FADERS.CPP, PLOTCHAR.CPP and the headers included into GameLoop.

// ===================================================================================
// Runtime: VGA mode 13h, DAC palette, 70Hz retrace timing, keyboard, sound
// ===================================================================================
const TICK = 1000 / 70;
const canvas = document.getElementById('screen');
Gfx.init(canvas);                          // gfx.js: WebGL scaling + CRT shader (2D fallback)
const vram = new Uint8Array(65536);       // A000:0000
const buffer = new Uint8Array(65536);     // off-screen buffer (also the collision map!)
const dac = new Uint8Array(768);          // VGA DAC, 6 bit components

function setDAC(arr) { dac.set(arr.subarray(0, 768)); }
function present() { Gfx.draw(vram, dac); }

let acc = 0, lastT = performance.now(), waiter = null;
function rafLoop(t) {
  acc += Math.min(t - lastT, 250);
  lastT = t;
  if (waiter && acc >= TICK) { acc -= TICK; const w = waiter; waiter = null; w(); }
  requestAnimationFrame(rafLoop);
}
requestAnimationFrame(rafLoop);

function nextTick() {
  if (acc >= TICK) { acc -= TICK; return Promise.resolve(); }
  return new Promise(r => { waiter = r; });
}
// Wait for vertical retrace: the frame that was built since the last retrace becomes visible.
async function vsync() { present(); await nextTick(); }
const DVR = vsync;                                  // MISC.CPP: DVR()
const sleep = ms => new Promise(r => setTimeout(r, ms));

// --- keyboard: set-1 scancodes, extended (E0) keys are +128, as in CORE.H keyb() -------------
const keys = new Uint8Array(256);
const CODEMAP = {
  Escape: 1, Digit1: 2, Digit2: 3, Digit3: 4, Digit4: 5, Digit5: 6, Digit6: 7, Digit7: 8, Digit8: 9, Digit9: 10, Digit0: 11,
  Minus: 12, Equal: 13, Backspace: 14, Tab: 15, KeyQ: 16, KeyW: 17, KeyE: 18, KeyR: 19, KeyT: 20, KeyY: 21, KeyU: 22, KeyI: 23,
  KeyO: 24, KeyP: 25, BracketLeft: 26, BracketRight: 27, Enter: 28, NumpadEnter: 28, ControlLeft: 29, KeyA: 30, KeyS: 31,
  KeyD: 32, KeyF: 33, KeyG: 34, KeyH: 35, KeyJ: 36, KeyK: 37, KeyL: 38, Semicolon: 39, Quote: 40, Backquote: 41, ShiftLeft: 42,
  Backslash: 43, KeyZ: 44, KeyX: 45, KeyC: 46, KeyV: 47, KeyB: 48, KeyN: 49, KeyM: 50, Comma: 51, Period: 52, Slash: 53,
  ShiftRight: 54, AltLeft: 56, Space: 57, F1: 59, F2: 60, F3: 61, F4: 62, F5: 63, F6: 64, F7: 65, F8: 66, F9: 67, F10: 68,
  ArrowUp: 72 + 128, ArrowLeft: 75 + 128, ArrowRight: 77 + 128, ArrowDown: 80 + 128, ControlRight: 29 + 128, AltRight: 56 + 128,
};
// Extra aliases so the game is playable in a browser (Ctrl / Alt are awkward there):
//   jump = Ctrl, Up, Z      fire = Alt, X, Shift     (menus accept Space/Enter)
const ALIAS = {
  ArrowLeft: [75], ArrowRight: [77], ArrowUp: [29], KeyZ: [29], ControlRight: [29], KeyX: [56], ShiftLeft: [56], ShiftRight: [56],
  AltRight: [56],
};
const heldCodes = new Set();
function applyKeys() {
  keys.fill(0);
  for (const c of heldCodes) {
    if (CODEMAP[c] !== undefined) keys[CODEMAP[c]] = 1;
    if (ALIAS[c]) for (const k of ALIAS[c]) keys[k] = 1;
  }
}
addEventListener('keydown', e => {
  if (e.code === 'F11' || e.code === 'F12') return;
  if (!e.repeat && (e.code === 'F6' || e.code === 'F7' || e.code === 'F8')) { e.preventDefault(); UI.hotkey(e.code); return; }
  if (CODEMAP[e.code] !== undefined || ALIAS[e.code]) e.preventDefault();
  if (!heldCodes.has(e.code)) pressedAt.set(e.code, performance.now());
  heldCodes.add(e.code); applyKeys(); unlockAudio();
  if (startResolve) { const r = startResolve; startResolve = null; r(); }
});
const pressedAt = new Map();
addEventListener('keyup', e => {
  if (CODEMAP[e.code] !== undefined) e.preventDefault();
  // keep very short taps down for at least ~2 ticks so a 70Hz poll cannot miss them (a real keyboard IRQ never would)
  const wait = 2 * TICK - (performance.now() - (pressedAt.get(e.code) || 0));
  const release = () => { heldCodes.delete(e.code); applyKeys(); };
  if (wait > 0) setTimeout(release, wait); else release();
});
addEventListener('blur', () => { heldCodes.clear(); applyKeys(); });
let startResolve = null;

// --- sound (replaces the Sound Blaster driver in SOUND.C) -------------------------------------
let actx = null;
const SFX = [];            // decoded AudioBuffers indexed as in LoadAudio() in CORE.H
let introBuf = null;
function unlockAudio() {
  if (!actx) { try { actx = new (window.AudioContext || window.webkitAudioContext)(); Music.ready(); } catch (e) { actx = null; } }
  if (actx && actx.state === 'suspended') actx.resume();
}
function playBuf(b) {
  if (!b || !actx) return;
  const s = actx.createBufferSource(); s.buffer = b; s.connect(actx.destination); s.start();
}
function ps2(n) {                                   // SOUND.C: ps2() queues SFX[n]
  if (!(GameSetup & 1)) return;
  playBuf(SFX[n]);
}
// distant thunder (rate limited). Pure sub-bass is inaudible on phone/laptop speakers, so the rumble has mid-range body
// (brown noise sweeping 1.1kHz -> 140Hz) plus saturation, which adds harmonics that small speakers can reproduce.
let lastThunder = 0, brownBuf = null, satCurve = null;
function thunder() {
  if (!(GameSetup & 1) || !actx) return;
  const now = performance.now();
  if (now - lastThunder < 9000 + Math.random() * 9000) return;
  lastThunder = now;
  if (actx.state === 'suspended') actx.resume();
  if (!brownBuf) {
    const n = actx.sampleRate * 5, b = actx.createBuffer(1, n, actx.sampleRate), d = b.getChannelData(0);
    let last = 0;
    for (let i = 0; i < n; i++) { last = (last + (Math.random() * 2 - 1) * .04) / 1.02; d[i] = last * 3.6; }
    brownBuf = b;
    satCurve = new Float32Array(1024);
    for (let i = 0; i < 1024; i++) { const x = i / 512 - 1; satCurve[i] = Math.tanh(x * 3.2); }
  }
  const t = actx.currentTime + .2 + Math.random() * .8, len = 3 + Math.random() * 1.5;
  const src = actx.createBufferSource(), lp = actx.createBiquadFilter(), sat = actx.createWaveShaper(), g = actx.createGain();
  src.buffer = brownBuf;
  lp.type = 'lowpass'; lp.Q.value = .9;
  lp.frequency.setValueAtTime(1100, t); lp.frequency.exponentialRampToValueAtTime(140, t + len);
  sat.curve = satCurve;
  g.gain.setValueAtTime(0, t);
  g.gain.linearRampToValueAtTime(.3, t + .08);                       // the crack / first hit
  g.gain.linearRampToValueAtTime(.12, t + .45);
  g.gain.linearRampToValueAtTime(.26, t + .8 + Math.random() * .4);   // the roll
  g.gain.linearRampToValueAtTime(.09, t + 1.7);
  g.gain.linearRampToValueAtTime(.15, t + 2.0 + Math.random() * .4);   // second, softer roll
  g.gain.exponentialRampToValueAtTime(.001, t + len);
  src.connect(lp); lp.connect(sat); sat.connect(g); g.connect(actx.destination);
  src.start(t, Math.random() * 1.5); src.stop(t + len + .1);
}
const SFXFILES = ['23', '26', '21', '6', '7', '4', '2', '8', '18', '25', '11', '19', '3', '50'];

// ===================================================================================
// Data
// ===================================================================================
const assets = new Map();
async function loadAssets() {
  const names = ['bmap1.map', 'bonuses.tga', 'brdg1.tga', 'dragon.tga', 'emap1.map', 'enemy1a.tga', 'enemy1b.tga', 'freez.tga',
    'info.tga', 'ldata.dat', 'level1.tga', 'mainmenu.tga', 'setup.tga', 'smap1.map', 'statgfx.tga', 'tmap1.map', 'tset1a.tga', 'tset1b.tga'];
  await Promise.all(names.map(async n => {
    const r = await fetch('assets/' + n);
    assets.set(n, new Uint8Array(await r.arrayBuffer()));
  }));
  // audio is optional; decoding needs an AudioContext (created lazily on first key press)
  window.__loadSounds = async () => {
    unlockAudio();
    if (!actx) return;
    const dec = async n => {
      try { const r = await fetch('assets/' + n + '.wav'); return await actx.decodeAudioData(await r.arrayBuffer()); } catch (e) { return null; }
    };
    introBuf = await dec('intro');
    for (let i = 0; i < SFXFILES.length; i++) SFX[i] = await dec(SFXFILES[i]);
  };
}
function file(n) { const f = assets.get(n); if (!f) throw new Error('missing ' + n); return f; }

class Rec {
  constructor(n) { this.a = new Uint8Array(n + 8); this.v = new DataView(this.a.buffer); }
  b(o) { return this.a[o]; }
  sb(o) { return this.v.getInt8(o); }
  w(o) { return this.v.getUint16(o, true); }
  sw(o) { return this.v.getInt16(o, true); }
  pb(o, x) { this.a[o] = x; }
  pw(o, x) { this.v.setUint16(o, x & 0xFFFF, true); }
}
const s16 = v => (v << 16) >> 16;

const Level = new Rec(10 * 468);
const En = new Rec(200 * 13);          // MObject32 Enemy[200]
const Bn = new Rec(400 * 5);           // BObject Bonus[400]
const IB = new Rec(24);                // IceBall: ctrl, x(w), y(w), frame
const ST = new Rec(80);                // Steam:   ctrl, frame, x(w), y(w), dummy(w)
const skymap = new Uint8Array(2048 + 8192 + 16);     // tilemap follows skymap in memory, like the C data segment
const tilemap = new Uint8Array(8192 + 16);
const tmv = new DataView(tilemap.buffer);
const BTYPE = new Uint8Array(31488 + 4096);
const BridgeGFX = new Uint8Array(64 + 64);
const StatGFX = new Uint8Array(1088 + 64);
const tile1 = new Uint8Array(65536 + 1024);
const tile2 = new Uint8Array(65536 + 1024);
const MCANIM = new Uint8Array(65536);
const enemy0 = new Uint8Array(19456 + 1024), enemy1 = new Uint8Array(19456 + 1024);
const enemy2 = new Uint8Array(19456 + 1024), enemy3 = new Uint8Array(19456 + 1024);
const pal = new Uint8Array(768 * 3);       // pal, SPal, FadeStore are adjacent in the C data segment
const SPal = pal.subarray(768, 1536);
const FadeStore = pal.subarray(1536, 2304);
const palMain = pal.subarray(0, 768);

let GameSetup = 7;     // 1 sound, 2 weather, 4 parallax sky, 8 widescreen, 16 gamepad
try { const v = localStorage.getItem('freez.setup'); if (v !== null) GameSetup = (+v) & 31; } catch (e) { }
function saveSetup() { try { localStorage.setItem('freez.setup', String(GameSetup)); } catch (e) { } }
let CurrentLevel = 1, AudioOn = 0;
let SEED = 0, SEED2 = 0;

let MCB = 0, Collision = 0, Firing = 0, Frame = 0, MomLeft = 0, MomRight = 0, JumpStat = 0, LastStat = 0, GravStat = 0;
let Lives = 0, Energy = 0, Timer = 0, Score = 0, TTimer = 0, TEnergy = 0, TScore = 0, LevelEnd = 0, HitCount = 0, IceBallNo = 4;
let RapidFire = 0, SuperJ = 0, SteamCount = 0, lp = 0;
let wposx = 0, wposy = 0, DXPos = 0, DYPos = 0, DXPos2 = 0, DYPos2 = 0, FYpos = 0, TileX = 0, FTileX = 0, TileIndex = 0, FTileIndex = 0;
const Drops = new Int16Array(800);         // 400 drops: x,y  (first 200 background, last 200 foreground)
let SkyBright = 63, SkyBrightDr = 0, SkyBrightCt = 0, TractionType = 0, frameinc = 0, BonusFrame = 0, HalfFrame = 0, atilecount = 0;
// Mobile build: jumping keeps its horizontal momentum (inertia); steering in the air is gentle.
let airVX = 0, airFrac = 0, wasAir = false, groundVX = 0;
const AIR_MAX = 2.4, AIR_ACCEL = 0.1, AIR_DRAG = 0.02;
function airSteer() {
  const L = keys[203] || keys[75], R = keys[205] || keys[77];
  let target = 0;
  if (L && !R) { target = -AIR_MAX; MCB &= 254; } else if (R && !L) { target = AIR_MAX; MCB |= 1; }
  if (target === 0) airVX += Math.max(-AIR_DRAG, Math.min(AIR_DRAG, -airVX));            // drift, no instant stop
  else if (!(Math.sign(airVX) === Math.sign(target) && Math.abs(airVX) >= AIR_MAX))     // never slow down a run you are already in
    airVX += Math.max(-AIR_ACCEL, Math.min(AIR_ACCEL, target - airVX));                  // limited braking/turning
  if (airVX < 0 && (Collision & 12)) airVX = 0;
  if (airVX > 0 && (Collision & 48)) airVX = 0;
  airFrac += airVX;
  const st = airFrac < 0 ? Math.ceil(airFrac) : Math.floor(airFrac);
  wposx = s16(wposx + st); airFrac -= st;
}
// land: turn the air speed back into the original momentum counters so ground movement carries on smoothly
function landMomentum() {
  const a = Math.abs(airVX), m = a >= 3.5 ? 35 : a >= 1.5 ? 20 : a > 0.3 ? 10 : 0;
  if (airVX < 0) { MomRight = m; MomLeft = 0; } else { MomLeft = m; MomRight = 0; }
}
const DROPBASE = 0x1234;                   // stands in for OFFSET Drops (only used for colour jitter)

// ===================================================================================
// Core graphics (TILE.CPP, ENEMY.CPP, BONUS.CPP, PBRIDGE.CPP, MAIN.CPP, PLOTCHAR.CPP)
// ===================================================================================
// Zero-transparent sprite blit with clipping. flip = mirrored horizontally.
function spriteT(src, base, w, h, x, y, flip, ylim) {
  const yEnd = Math.min(h, ylim - y);
  for (let row = y < 0 ? -y : 0; row < yEnd; row++) {
    const drow = (y + row) * 320;
    const srow = base + row * w;
    for (let col = 0; col < w; col++) {
      const dx = x + col;
      if (dx < 0 || dx > 319) continue;
      const v = src[srow + (flip ? w - 1 - col : col)];
      if (v) buffer[(drow + dx) & 0xFFFF] = v;
    }
  }
}
function psprite32(x, y, sprite, frame, speed) {
  const src = sprite === 0 ? enemy0 : sprite === 1 ? enemy1 : sprite === 2 ? enemy2 : enemy3;
  spriteT(src, (frame & 0xFC) << 8, 32, 32, x, y, speed < 0, 192);
}
function psprite16(x, y, sprite, frame) {
  spriteT(BTYPE, ((sprite & 0x1F) << 11) + ((frame & 0xFC) << 6), 16, 16, x, y, false, 192);
}
function pbridge(x, y) { spriteT(BridgeGFX, 0, 8, 8, x, y, false, 191); }

// ptile: tiles 0-63 from tile1 (transparent), 64-95 from tile2 (transparent), 96+ from tile2 (opaque)
function ptile(xpos, ypos, tileno) {
  tileno &= 255;
  let xend = 32, yend = 32, di = 288, si = 0, dx = 0, bx = ypos, ax = xpos;
  if (bx < 0) { yend += bx; bx = -bx; si = bx << 5; bx = 0; }
  else if (bx > 158) { yend = (191 - bx) & 255; }
  if (ax < 0) { xend += ax; si -= ax; di -= ax; dx = -ax; ax = 0; }
  else if (ax > 288) { const cx = 320 - ax; xend = cx; di = ax; dx = 32 - cx; }
  let scr = (ax + bx * 320) & 0xFFFF;
  const ystep = di;
  let src;
  if (tileno > 95) {
    si += ((tileno - 64) << 10) & 0xFFFF;
    src = tile2;
    if (xend === 32) {
      for (let y = 0; y < yend; y++) { for (let k = 0; k < 32; k++) buffer[(scr + k) & 0xFFFF] = src[si + k]; si += 32; scr += ystep + 32; }
    } else {
      for (let y = 0; y < yend; y++) { for (let k = 0; k < xend; k++) buffer[(scr + k) & 0xFFFF] = src[si + k]; si += xend + dx; scr += ystep + xend; }
    }
    return;
  }
  if (tileno > 63) { src = tile2; si += (tileno - 64) << 10; } else { src = tile1; si += tileno << 10; }
  if (xend === 32) {
    for (let y = 0; y < yend; y++) {
      if (src[si] === 255) { si += 32; }                        // first byte 255 flags an empty line
      else { for (let k = 0; k < 32; k++) { const v = src[si + k]; if (v) buffer[(scr + k) & 0xFFFF] = v; } si += 32; }
      scr += 320;
    }
    return;
  }
  for (let y = 0; y < yend; y++) {                             // clipped transparent plot
    let d = scr;
    if (xend & 1) { const v = src[si++]; if (v !== 255 && v !== 0) buffer[d & 0xFFFF] = v; d++; }
    if (xend & 2) {
      const a = src[si], b = src[si + 1]; si += 2;
      if (a !== 255 && a !== 0) buffer[d & 0xFFFF] = a;
      if (b !== 0) buffer[(d + 1) & 0xFFFF] = b;
      d += 2;
    }
    let cx = xend;
    if (src[si] === 255) { cx &= 0xFFFC; si += cx; d += cx; }
    else { cx >>= 2; for (let k = 0; k < cx * 4; k++) { const v = src[si + k]; if (v) buffer[(d + k) & 0xFFFF] = v; } si += cx * 4; d += cx * 4; }
    scr = d + ystep; si += dx;
  }
}

// MAIN.CPP: landscape detection + main character plot. dl = MCB, ah = Frame, returns dh (collision flags)
function pmain(dl, ah) {
  const B = buffer;
  let dh = 0;
  if (B[37918] >= 128) dh |= 1;
  if (B[37922] >= 128) dh |= 1;
  if (B[42074 - 1600 - 320] >= 128) dh |= 64;
  if (B[42086 - 1600 - 320] >= 128) dh ^= 64;
  if (B[27024 + 20 - 6 * 320] >= 128) dh |= 2;
  if (B[27024 + 12 - 6 * 320] >= 128) dh |= 2;
  if (B[28308] >= 128) dh |= 4;
  if (B[28308 + 2560] >= 128) dh |= 4;
  if (B[28308 + 2560 + 1280] >= 128) dh |= 4;
  if (B[28308 + 2560 + 2560] >= 128) dh |= 8;
  if (B[28308 + 2560 + 2560 + 1280] >= 128) dh |= 8;
  if (B[28308 + 2560 + 2560 + 2560 + 1280 + 4] >= 128) dh |= 8;
  if (B[35988] >= 128) dh |= 8;
  if (B[28332] >= 128) dh |= 16;
  if (B[28332 + 2560] >= 128) dh |= 16;
  if (B[28332 + 2560 + 1280] >= 128) dh |= 16;
  if (B[28332 + 2560 + 2560] >= 128) dh |= 32;
  if (B[28332 + 2560 + 2560 + 1280] >= 128) dh |= 32;
  if (B[28332 + 2560 + 2560 + 2560 + 1280 - 4] >= 128) dh |= 32;
  if (B[36012] >= 128) dh |= 32;
  spriteT(MCANIM, (ah & 0xFC) << 8, 32, 32, 144, 84, !(dl & 1), 1000);
  return dh;
}

function PlotChars(di, str) {
  for (let c = 0; c < str.length; c++) {
    const s = str[c] * 64;
    for (let r = 0; r < 8; r++) for (let k = 0; k < 8; k++) buffer[(di + r * 320 + k) & 0xFFFF] = StatGFX[s + r * 8 + k];
    di += 8;
  }
}
function PlotIcon(di, n) {
  const s = n * 64;
  for (let r = 0; r < 8; r++) for (let k = 0; k < 8; k++) buffer[(di + r * 320 + k) & 0xFFFF] = StatGFX[s + r * 8 + k];
}

// CORE.H Line(): plots 2-pixel (colour 1) dots along a line
function Line(a, b, c, d) {
  let ax = s16(c - a), bx = s16(d - b);
  const d1x = ax < 0 ? -1 : 1, d1y = bx < 0 ? -1 : 1;
  let d2x = d1x, d2y = 0;
  let m = Math.abs(ax), n = Math.abs(bx);
  if (!(m > n)) { d2x = 0; d2y = d1y; const t = m; m = n; n = t; }
  let acc2 = m >> 1;
  for (let cx = m || 65536; cx > 0; cx--) {
    const di = (a + b * 320) & 0xFFFF;
    buffer[di] = 1; buffer[(di + 1) & 0xFFFF] = 1;
    acc2 += n;
    if (acc2 < m) { a += d2x; b += d2y; }
    else { acc2 -= m; a += d1x; b += d1y; }
  }
}

// ===================================================================================
// Screen / palette helpers (MISC.CPP, FADERS.CPP)
// ===================================================================================
function Clear() { buffer.fill(0, 0, 64000); }
function SCopy() { vram.set(buffer.subarray(0, 64000)); }
function SRCopy() { buffer.set(vram.subarray(0, 64000)); }
function FixPal() {                 // 8 -> 6 bit, BGR -> RGB
  for (let i = 0; i < 256; i++) {
    const r = palMain[i * 3] >> 2, g = palMain[i * 3 + 1] >> 2, b = palMain[i * 3 + 2] >> 2;
    palMain[i * 3] = b; palMain[i * 3 + 1] = g; palMain[i * 3 + 2] = r;
  }
}
const SavedPal = new Uint8Array(768);
const cloudBase = new Uint8Array(9);           // original colours of the cloud palette entries 55-57
function SavePal() { SavedPal.set(palMain); }
function SetPal() { palMain.set(SavedPal); }
function Screen(mode) { /* CRTC tweaks for the original 'widescreen' option: nothing to do here */ }

async function Fade2Black(flag) {
  FadeStore.set(palMain);
  for (let n = flag ? 20 : 64; n > 0; n--) {
    for (let i = 0; i < 768; i++) if (FadeStore[i] !== 0) FadeStore[i]--;
    await vsync();
    setDAC(FadeStore);
  }
}
async function FadeFromBlack(flag) {
  for (let n = flag ? 24 : 64; n > 0; n--) {
    await vsync();
    for (let i = 0; i < 768; i++) if (FadeStore[i] !== palMain[i]) FadeStore[i]++;
    setDAC(FadeStore);
  }
}

const anyKey063 = () => { for (let i = 0; i < 64; i++) if (keys[i]) return true; return false; };
async function WAIT() { while (!anyKey063()) await vsync(); }       // MISC.CPP WAIT(): no gamepad
async function Pause() { SavePal(); await Fade2Black(1); await WAIT(); SetPal(); await FadeFromBlack(1); }

// ===================================================================================
// Loaders (CORE.H, MENUS.CPP LoadLogo)
// ===================================================================================
function makeTextScreen(lines) {
  const c = document.createElement('canvas'); c.width = 320; c.height = 200;
  const g = c.getContext('2d');
  g.fillStyle = '#000'; g.fillRect(0, 0, 320, 200);
  g.fillStyle = '#fff'; g.font = 'bold 24px sans-serif'; g.textAlign = 'center';
  lines.forEach((l, i) => g.fillText(l, 160, 90 + i * 32));
  const d = g.getImageData(0, 0, 320, 200).data;
  for (let i = 0; i < 64000; i++) buffer[i] = d[i * 4] > 100 ? 1 : 0;
  palMain.fill(0); palMain[3] = palMain[4] = palMain[5] = 63;
}
function LoadLogo(IMG) {
  const names = { 1: 'mainmenu.tga', 2: 'setup.tga', 3: 'info.tga', 10: 'level1.tga', 11: 'level1.tga' };
  const n = names[IMG];
  if (n) {
    const f = file(n);
    palMain.set(f.subarray(0x12, 0x12 + 768));
    buffer.set(f.subarray(0x312, 0x312 + 64000));
    SCopy();
    FixPal();
  } else if (IMG === 7) {                                   // lev_comp.tga is not part of the shipped data
    makeTextScreen(['LEVEL COMPLETE']); SCopy();
  } else {                                                  // level2-4 images are not shipped either: reuse level 1
    const f = file('level1.tga');
    palMain.set(f.subarray(0x12, 0x12 + 768));
    buffer.set(f.subarray(0x312, 0x312 + 64000));
    SCopy(); FixPal();
  }
}

async function LoadLevel() {
  await Fade2Black(0);
  LoadLogo(CurrentLevel + 9);
  await FadeFromBlack(0);
  await sleep(1000);                                         // mobile build: hold the level intro screen 1s longer
  BridgeGFX.set(file('brdg1.tga').subarray(0x312, 0x312 + 64));
  tile1.set(file('tset1a.tga').subarray(0x312, 0x312 + 65535));
  tile2.set(file('tset1b.tga').subarray(0x312, 0x312 + 65535));
  // clouds (tiles 47-52) share palette entries 1/38/54 with sprites; give them private entries 55/56/57 so they can fade at night
  for (let t = 47; t <= 52; t++) for (let i = t * 1024; i < t * 1024 + 1024; i++) { const v = tile1[i]; if (v === 1) tile1[i] = 55; else if (v === 38) tile1[i] = 56; else if (v === 54) tile1[i] = 57; }
  enemy0.set(file('enemy1a.tga').subarray(0x312, 0x312 + 19456));
  enemy1.set(file('enemy1b.tga').subarray(0x312, 0x312 + 19456));
  tilemap.set(file('tmap1.map').subarray(0, 8192));
  En.a.fill(0); En.a.set(file('emap1.map').subarray(0, 2600));
  Bn.a.fill(0); Bn.a.set(file('bmap1.map').subarray(0, 2000));
  skymap.set(file('smap1.map').subarray(0, 2048));
  await Fade2Black(0);
  palMain.set(file('dragon.tga').subarray(0x12, 0x12 + 768));
  FixPal();
  for (const [dst, srcIdx] of [[55, 1], [56, 38], [57, 54]]) for (let k = 0; k < 3; k++) { palMain[dst * 3 + k] = palMain[srcIdx * 3 + k]; cloudBase[(dst - 55) * 3 + k] = palMain[srcIdx * 3 + k]; }
}
function LoadGame() {
  MCANIM.set(file('dragon.tga').subarray(0x312, 0x312 + 36864));
  BTYPE.set(file('bonuses.tga').subarray(0x312, 0x312 + 31488));
  StatGFX.set(file('statgfx.tga').subarray(0x312, 0x312 + 1088));
}

// ===================================================================================
// Title sequence (CORE.H LoadTitle)
// ===================================================================================
const vram32 = new Uint32Array(vram.buffer);
function stepSeed() { const c = SEED >>> 31; SEED = (SEED << 1) >>> 0; if (c) SEED = (SEED ^ 0x1D872B41) >>> 0; }
function stosd(di, v) { for (let k = 0; k < 4; k++) vram[(di + k) & 0xFFFF] = (v >>> (8 * k)) & 255; }
function snow() { for (let i = 0; i < 16000; i++) { stepSeed(); vram32[i] = SEED; } }

async function LoadTitle() {
  const f = file('freez.tga');
  FadeStore.set(f.subarray(0x12, 0x12 + 768));
  buffer.set(f.subarray(0x312, 0x312 + 64000));
  for (let i = 0; i < 256; i++) {
    const r = FadeStore[i * 3] >> 2, g = FadeStore[i * 3 + 1] >> 2, b = FadeStore[i * 3 + 2] >> 2;
    FadeStore[i * 3] = b; FadeStore[i * 3 + 1] = g; FadeStore[i * 3 + 2] = r;
  }
  GameSetup |= 1;
  if (!introBuf) GameSetup &= 254;
  for (let i = 0; i < 256; i++) palMain[i * 3] = palMain[i * 3 + 1] = palMain[i * 3 + 2] = i >> 2;   // grey palette
  SPal.fill(0); setDAC(SPal);

  const skippable = () => keys[57] !== 0;
  quick: {
    for (let bx = 64; bx > 0; bx--) {                      // fade in snow
      await DVR();
      snow();
      for (let i = 0; i < 256; i++) if (SPal[i * 3] !== palMain[i * 3]) { SPal[i * 3]++; SPal[i * 3 + 1]++; SPal[i * 3 + 2]++; }
      setDAC(SPal);
    }
    if (skippable()) break quick;
    for (let bx = 100; bx > 0; bx--) {                     // random tuning effect
      await DVR();
      snow();
      const eax = SEED;
      if (bx <= 75 && (eax & 255) <= 90) SCopy();
      let di = ((eax & 0xFFFF) & 0x80FF) + 12000;
      const cx = (eax & 0xFFFF) & 4095, v = (eax ^ 0x1D872B41) >>> 0;
      for (let k = 0; k < cx; k++) { stosd(di, v); di += 4; }
    }
    if (skippable()) break quick;
    for (let bx = 50; ; bx--) {                            // picture with noise bands
      await DVR();
      SCopy();
      if (bx === 1) break;
      stepSeed();
      let di = SEED & 0xFFFF;
      if (di >= 47000) di = 32000;
      const cx = di & 4095;
      for (let k = 0; k < cx; k++) { stosd(di, SEED); di += 4; }
    }
    if (skippable()) break quick;
    await sleep(1000);
    if (introBuf && (GameSetup & 1)) playBuf(introBuf);
    for (let bx = 60; bx > 0; bx--) {                      // grey -> colour
      await DVR(); await DVR(); await DVR(); await DVR();
      for (let i = 0; i < 768; i++) { const p = palMain[i], t = FadeStore[i]; if (p !== t) palMain[i] += p < t ? 1 : -1; }
      setDAC(palMain);
    }
  }
  await Fade2Black(0);
}

// ===================================================================================
// Menus (MENUS.CPP)
// ===================================================================================
function DBox(o, col) {
  for (let i = 0; i < 34; i++) { vram[o + i] = col; vram[o + 10240 + i] = col; }
  for (let i = 0; i < 33; i++) { vram[o + i * 320] = col; vram[o + 33 + i * 320] = col; }
}
function DSBox(o, col) {
  for (let i = 0; i < 15; i++) { vram[o + i] = col; vram[o + 4800 - 320 + i] = col; }
  for (let i = 0; i < 15; i++) { vram[o + i * 320] = col; vram[o + 15 + i * 320] = col; }
}
function Light(o, col) { for (let r = 0; r < 3; r++) for (let k = 0; k < 4; k++) vram[o + r * 320 + k] = col; }
const IsAnyKey = () => keys[57] !== 0 || keys[28] !== 0;
const LEFT = () => keys[203] || keys[75], RIGHT = () => keys[205] || keys[77];

async function MainMenu() {
  Music.play('menu');
  const boxes = [37810, 37872, 37934, 37996];
  for (;;) {
    let txloop = 0, LastKey = 0;
    LoadLogo(1);
    boxes.forEach((b, i) => DBox(b, i === 0 ? 215 : 0));
    await FadeFromBlack(0);
    for (;;) {
      if (LEFT() && !RIGHT() && LastKey !== 1) { ps2(8); LastKey = 1; txloop = txloop === 0 ? 3 : txloop - 1; }
      if (RIGHT() && !LEFT() && LastKey !== 2) { ps2(8); LastKey = 2; txloop = txloop === 3 ? 0 : txloop + 1; }
      boxes.forEach((b, i) => DBox(b, i === txloop ? 215 : 0));
      if (!LEFT() && !RIGHT()) LastKey = 0;
      if (IsAnyKey()) break;
      await vsync();
    }
    ps2(8);
    if (txloop === 1) { await Fade2Black(0); await Options(); continue; }
    if (txloop === 2) { await Fade2Black(0); await Information(); continue; }
    if (txloop === 3) { await Fade2Black(0); return 1; }
    return 0;
  }
}
async function Information() {
  LoadLogo(3);
  await FadeFromBlack(0);
  while (!IsAnyKey()) await vsync();
  await Fade2Black(0);
}
async function Options() {
  const boxes = [4247, 13847, 23127, 32407, 41687, 54487];
  const lights = [6173, 15773, 25053, 34333, 43613];
  let txloop = 0, LastKey = 0;
  LoadLogo(2);
  boxes.forEach((b, i) => DSBox(b, i === 0 ? 37 : 0));
  lights.forEach((l, i) => { if (GameSetup & (1 << i)) Light(l, 37); });
  await FadeFromBlack(0);
  for (;;) {
    if (LEFT() && !RIGHT() && LastKey !== 1) { ps2(8); LastKey = 1; txloop = txloop === 0 ? 5 : txloop - 1; }
    if (RIGHT() && !LEFT() && LastKey !== 2) { ps2(8); LastKey = 2; txloop = txloop === 5 ? 0 : txloop + 1; }
    boxes.forEach((b, i) => DSBox(b, i === txloop ? 37 : 0));
    lights.forEach((l, i) => Light(l, (GameSetup & (1 << i)) ? 37 : 0));
    if (!LEFT() && !RIGHT()) LastKey = 0;
    if (IsAnyKey()) {
      switch (txloop) {
        case 0: if (AudioOn) GameSetup ^= 1; break;
        case 1: GameSetup ^= 2; break;
        case 2: GameSetup ^= 4; break;
        case 3: GameSetup ^= 8; Screen(GameSetup & 8 ? 1 : 0); break;
        case 4: break;                                       // gamepad: not available (ReadAxis returned 3)
        case 5: ps2(8); saveSetup(); await Fade2Black(0); return;
      }
      ps2(8);
      while (IsAnyKey()) await vsync();
    }
    await vsync();
  }
}

// ===================================================================================
// Game: the per-frame sections that FREEZE.CPP #includes into GameLoop
// ===================================================================================
function resetDropsInit() {                                  // FREEZE.CPP "Initialise Weather"
  let ax = 0;
  for (let i = 0; i < 400; i++) {
    const c = (SEED2 >>> 30) & 1;
    SEED2 = (SEED2 << 2) >>> 0;
    if (c) { SEED2 = (SEED2 ^ 0x1D872B41) >>> 0; ax = SEED2 & 0xFFFF; }
    ax &= 0x3FF;
    Drops[i * 2] = ax - 350;
    ax &= 0xFF; ax += ax;
    Drops[i * 2 + 1] = ax;
  }
}
const weatherOn = () => (GameSetup & 2) && (SkyBrightDr === 0 ? SkyBright <= 30 : SkyBright > 30);

// SKYRAIN.H: sky gradient (colours 64-95), lightning, background rain
function skyrain() {
  let c = ((wposy & 0xFF) >> 2) & 31;
  const rows = (~wposy) & 3;
  let di = 0;
  let col = c + 64;
  buffer.fill(col, 0, rows * 320); di = rows * 320;
  col = ((c + 1) & 31) + 64;
  for (let n = 0; n < 64; n++) {
    buffer.fill(col, di, di + 960); di += 960;
    col = ((col + 1) & 31) + 64;
  }
  if (!weatherOn()) return;
  // --- lightning
  let ax = col, dx = 0;       // ax/dx are stale if no carry; ax then holds a sky colour (>20) so nothing strikes
  {
    const c1 = SEED2 >>> 31; SEED2 = (SEED2 << 1) >>> 0;
    if (c1) { SEED2 = (SEED2 ^ 0x1D872B41) >>> 0; ax = SEED2 & 0xFFFF; dx = SEED2 >>> 16; }
    if ((ax & 255) <= 20) {
      thunder();
      ax &= 3;
      let STK4 = ax;
      dx = (dx & 255) + 32;
      if (dx >= 160) STK4 = -STK4;
      let STK1 = dx, STK2 = 0, bx = 0;
      for (let k = 0; k < 40; k++) {
        const cc = SEED2 >>> 31; SEED2 = (SEED2 << 1) >>> 0;
        if (cc) { SEED2 = (SEED2 ^ 0x1D872B41) >>> 0; ax = SEED2 & 0xFFFF; }
        ax = ((ax & 15) + dx + bx) & 0xFFFF;
        const STK3 = s16(ax);
        Line(STK1, STK2, STK3, STK2 + 5);
        STK1 = s16(ax); STK2 += 5; bx += STK4;
      }
    }
  }
  // --- background rain
  ax = wposx;
  DXPos = s16(DXPos - wposx); DYPos = s16(DYPos - wposy);
  const bx2 = DXPos, dy2 = DYPos;
  DXPos2 = bx2; DYPos2 = dy2;
  rain(0, 200, bx2, dy2, 2, ax);
  DXPos = wposx; DYPos = wposy;
}
function rain(first, count, bx, dx, fall, ax) {
  for (let i = first; i < first + count; i++) {
    Drops[i * 2] += bx; Drops[i * 2 + 1] += dx;
    const x = Drops[i * 2], y = Drops[i * 2 + 1];
    if (x >= 0 && x <= 319) {
      ax = y;
      if (!(ax < 0 || ax > 190)) {
        const di = x + ax * 320;
        ax = (ax << 8) & 0xFFFF;
        const si = (DROPBASE + i * 4) & 0xFFFF;
        ax = (ax & 0xFF00) | (SEED2 & 255);
        ax ^= si;
        buffer[di & 0xFFFF] = (ax & 7) + 96;
        ax = (ax & 0xFF00) | ((ax & 7) + 96);
      }
    }
    Drops[i * 2 + 1] += fall;
    const ny = Drops[i * 2 + 1];
    if (ny > 350 || ny < -300) {
      const c = SEED2 >>> 31; SEED2 = (SEED2 << 1) >>> 0;
      if (c) { SEED2 = (SEED2 ^ 0x1D872B41) >>> 0; ax = SEED2 & 0xFFFF; }
      ax &= 0x3FF; ax = (ax - 352) & 0xFFFF;
      Drops[i * 2] = ax;
      ax &= 0x0F;
      Drops[i * 2 + 1] = ax;
    }
  }
}

// SKYPLOT.H: parallax sky tiles
function skyplot() {
  if (!(GameSetup & 4)) return;
  const wx = wposx & 0xFFFF, wy = wposy & 0xFFFF;
  TileX = -((wx >> 1) & 0x1F);
  let si = wy >> 1;
  let cx = si;
  si = -(si & 0x1F);
  cx = ((cx & 0xFFE0) * 2) & 0xFFFF;
  TileIndex = ((wx >> 6) + cx) & 0xFFFF;
  for (let y = 0; y < 7; y++) {
    let di = TileX;
    for (let x = 0; x < 11; x++) {
      if (!(di > 319 || di < -31 || si > 190 || si < -31)) {
        const al = skymap[TileIndex];
        if (al) ptile(di, si, al);
      }
      di += 32; TileIndex++;
    }
    si += 32; TileIndex += 53;
  }
}

// BRIDGE.H
function bridges() {
  for (let rec = 0; rec < 16; rec++) {
    const bx = lp + 74 + rec * 4;
    let si = 0;
    let ax = s16(wposy - 8);
    let cx = Level.sw(bx + 2);
    if (cx === 0) continue;
    if (cx < ax) continue;
    ax += 198; if (cx > ax) continue;
    ax = Level.sw(bx);
    let dx = s16(wposx - 255);
    if (ax < dx) continue;
    dx += 575; if (ax > dx) continue;
    ax = s16(wposy + 100);
    dd: {
      if (cx <= ax) break dd;
      ax += 20;
      if (cx >= ax) break dd;
      cx = Level.sw(bx); ax = wposx; dx = ax; ax += 153; dx += 161;
      do {
        if (!(cx < ax)) { if (cx <= dx) break dd; }
        cx += 8; si++;
      } while (si < 31);
    }
    let sp = si * 32;
    let di = Level.sw(bx);
    for (let n = 0; n < 32; n++) {
      const y = s16(Level.sw(bx + 2) + BridgeDynamics[sp] - wposy);
      const x = s16(di - wposx);
      if (!(x < -8 || x > 319 || y < -8 || y > 190)) pbridge(x, y);
      sp++; di += 8;
    }
  }
}

// BACKPLOT.H
function backplot() {
  const wx = wposx, wy = wposy & 0xFFFF;
  TileX = FTileX = -(wx & 0x1F);
  let si = wy & 0x1F;
  si = -si; FYpos = si;
  let cx = ((wy & 0xFFE0) << 2) & 0xFFFF;
  TileIndex = FTileIndex = ((wx >> 5) + cx) & 0xFFFF;
  for (let y = 0; y < 7; y++) {
    let di = TileX;
    for (let x = 0; x < 11; x++) {
      if (!(di > 319 || di < -31 || si > 190 || si < -31)) {
        let al = tilemap[TileIndex];
        if (!(al & 128)) { if (al) ptile(di, si, al); }
      }
      di += 32; TileIndex++;
    }
    si += 32; TileIndex += 117;
  }
}

// TILEANIM.H
function tileanim() {
  if (BonusFrame & 7) return;
  atilecount++;
  const sub = atilecount === 4;
  for (let k = 0; k < 32; k++) {
    const bx = Level.w(lp + 10 + k * 2);
    if (bx > 8190) continue;
    tmv.setUint16(bx, (tmv.getUint16(bx, true) + (sub ? -3 : 1)) & 0xFFFF, true);
  }
  if (sub) atilecount = 0;
}

// STATBAR.H
const digits = (v, n) => { const a = []; for (let i = 0; i < n; i++) { a.unshift(v % 10); v = Math.floor(v / 10); } return a; };
function statbar() {
  buffer.fill(0, 320 * 191, 320 * 191 + 9 * 320);
  const levelStr = digits(CurrentLevel, 2), scoreStr = digits(TScore & 0xFFFF, 6);
  const timerStr = digits((TTimer & 0xFFFF) >> 6, 3), energyStr = digits(TEnergy, 3), livesStr = digits(Lives, 2);
  if (!(Timer !== 0 && Timer <= 1344 && (frameinc & 16))) PlotIcon(192 * 320 + 8, 12);
  PlotChars(320 * 192 + 18, timerStr);
  if (!(Energy <= 20 && (frameinc & 16))) PlotIcon(192 * 320 + 58, 10);
  PlotChars(320 * 192 + 68, energyStr);
  PlotIcon(192 * 320 + 108, 11);
  PlotChars(320 * 192 + 118, livesStr);
  PlotIcon(192 * 320 + 150, 14);
  PlotChars(320 * 192 + 160, levelStr);
  PlotChars(320 * 192 + 260, scoreStr);
  if (RapidFire !== 0) {
    RapidFire = (RapidFire - 1) & 0xFFFF;
    if (RapidFire > 350 || !(frameinc & 16)) PlotIcon(192 * 320 + 190, 16);
  }
  if (SuperJ !== 0) {
    SuperJ = (SuperJ - 1) & 0xFFFF;
    if (SuperJ > 350 || !(frameinc & 16)) PlotIcon(192 * 320 + 210, 15);
  }
  // smooth score / energy / time transitions
  if (TScore !== Score) { if (s16(TScore) < s16(Score)) TScore = (TScore + 10) & 0xFFFF; else TScore = (TScore - 2) & 0xFFFF; }
  if (TEnergy !== Energy) { if (TEnergy < Energy) TEnergy++; else TEnergy--; }
  if (TTimer !== Timer) {
    if (s16(TTimer) < s16(Timer)) { TTimer = (TTimer + 100) & 0xFFFF; if (s16(TTimer) > s16(Timer)) TTimer = Timer; }
    else { TTimer = (TTimer - 100) & 0xFFFF; if (s16(TTimer) < s16(Timer)) TTimer = Timer; }
  }
}

// STEAM.H
function steam() {
  if (SteamCount !== 0) {
    let al = SteamCount - (BonusFrame & 1);
    const di = al * 8;
    ST.pw(di + 2, wposx + ((MCB & 1) ? 164 : 141));
    ST.pw(di + 4, wposy + 79);
    ST.pb(di, (ST.b(di) + 1) & 255);
    ST.pb(di + 1, (al << 2) & 255);
    SteamCount = al;
  }
  for (let i = 0; i < 8; i++) {
    const si = i * 8;
    if (ST.b(si) === 0) continue;
    const ax = s16(ST.w(si + 2) - wposx), bx = s16(ST.w(si + 4) - wposy);
    if (!(ax > 319 || ax < -16 || bx > 190 || bx < -16)) psprite16(ax, bx, 4, ST.b(si + 1));
    ST.pb(si + 1, (ST.b(si + 1) + (BonusFrame & 1)) & 255);
    if (ST.b(si + 1) === 32) ST.pb(si, 0);
    ST.pw(si + 4, ST.w(si + 4) - 1);
  }
}

// FLNDRAIN.H: foreground tiles then foreground rain
function flndrain() {
  let dx = FYpos, ti = FTileIndex;
  for (let y = 0; y < 7; y++) {
    let cx = FTileX;
    for (let x = 0; x < 11; x++) {
      if (!(dx > 190 || dx < -31 || cx > 319 || cx < -31)) {
        const al = tilemap[ti];
        if ((al & 128) && (al & 127)) ptile(cx, dx, al & 127);
      }
      cx += 32; ti++;
    }
    dx += 32; ti += 117;
  }
  FTileIndex = ti;
  if (weatherOn()) rain(200, 200, DXPos2, DYPos2, 3, 0);
}

// COLLANI.H: bonuses, enemies, platforms, ice balls
function collani() {
  // ---- bonuses
  for (let i = 0; i < 400; i++) {
    const si = i * 5;
    const bl = Bn.b(si);
    if (bl === 0) continue;
    if (bl >= 128) Bn.pb(si, (bl + 1) & 255);
    const cx = Bn.w(si + 1);
    const ax = s16(cx - wposx);
    if (ax < -16 || ax > 319) continue;
    const dx = s16(Bn.w(si + 3) - wposy);
    if (dx < -16 || dx > 192) continue;
    if (bl >= 128) { psprite16(ax, dx, 14, (bl - 224) & 255); continue; }
    psprite16(ax, dx, bl, ((cx & 255) + BonusFrame) & 31);
    if (ax > 169 || ax < 136 || dx > 108 || dx < 76) continue;
    let snd = -1;
    switch (bl) {
      case 1: Score += 150; snd = 1; break;
      case 2: Score += 50; snd = 1; break;
      case 3:
        if (Energy === 100) continue;
        if (Energy < 60) Energy += 40; else Energy = 100;
        snd = 6; Score += 100; break;
      case 6: if (RapidFire !== 0) continue; snd = 3; RapidFire = 2048; Score += 50; break;
      case 7: if (Timer === 0) continue; snd = 5; Timer += 3240; Score += 100; break;
      case 8: snd = 4; Lives++; Score += 150; break;
      case 9: if (SuperJ !== 0) continue; snd = 3; SuperJ = 2048; Score += 50; break;
      case 10: case 11: case 12: case 13: Score += 50; snd = 2; break;
      case 14: snd = 2; break;
    }
    Score &= 0xFFFF; Timer &= 0xFFFF;
    if (snd >= 0) ps2(snd);
    Bn.pb(si, 224);
  }
  BonusFrame = (BonusFrame + 1) & 31;

  // ---- enemies
  for (let e = 0; e < 200; e++) {
    const si = e * 13;
    if (En.b(si) === 0) continue;
    let dl = 0;
    main: {
      if (En.b(si) > 2) break main;                          // not an animated enemy
      let ax = s16(En.w(si + 3) - wposx), bx = s16(En.w(si + 5) - wposy);
      if (En.b(si + 1) !== 5) { if (ax > 640 || ax < -320 || bx > 400 || bx < -200) break main; }
      En.pb(si + 12, En.b(si + 12) & ~4);
      if (!(ax > 319 || ax < -31 || bx > 190 || bx < -31)) {
        psprite32(ax, bx, En.b(si + 7), En.b(si + 8), En.sb(si + 11));
        EnemySFX.voice(e, En.b(si), En.b(si + 7), En.b(si + 1), En.b(si + 8), ax, bx);
        En.pb(si + 12, En.b(si + 12) | 4);
      }
      if (BonusFrame & 2) En.pb(si + 8, (En.b(si + 8) + 1) & 255);
      if (En.b(si + 1) === 0) {
        if (IceBallNo !== 4) {                               // turn to face the ice ball
          const ey = En.sw(si + 5);
          let lim = s16(wposy + 60);
          if (!(ey < lim)) {
            lim += 40;
            if (!(ey > lim)) {
              const rx = s16(En.w(si + 3) - wposx);
              let ok;
              if (rx > 144) ok = (MCB & 1) && !(En.sb(si + 11) > 0);
              else ok = !(MCB & 1) && !(En.sb(si + 11) < 0);
              if (ok) { const d = En.b(si + 9); if (d !== En.b(si + 10) && d !== 0) En.pb(si + 11, (-En.b(si + 11)) & 255); }
            }
          }
        }
        En.pw(si + 3, En.w(si + 3) + En.sb(si + 11));
        En.pb(si + 9, (En.b(si + 9) + En.b(si + 11)) & 255);
        const d = En.b(si + 9);
        if (!(d < En.b(si + 10))) En.pb(si + 11, (-En.b(si + 11)) & 255);
        if (d === 0) En.pb(si + 11, (-En.b(si + 11)) & 255);
        if (En.b(si + 8) === 32) En.pb(si + 8, 0);
      }
      // status control
      const st = En.b(si + 1);
      if (st === 0) break main;
      if (st === 1) { En.pb(si + 8, 32); En.pb(si + 1, 5); }
      else if (st === 2) { En.pb(si + 8, 44); En.pb(si + 1, 5); En.pb(si + 2, 0); }
      else if (st === 3) { En.pb(si + 8, 48); En.pb(si + 1, 5); }
      else if (st === 4) { En.pb(si + 8, 60); En.pb(si + 1, 6); Score = (Score + 100) & 0xFFFF; ps2(7); }
      const fr = En.b(si + 8);
      if (fr === 43) En.pb(si + 1, 2);
      else if (fr === 47) En.pb(si + 8, 44);
      else if (fr === 59) { En.pb(si + 1, 0); En.pb(si + 8, 0); }
      else if (fr === 75) En.pb(si, 0);
      if (En.b(si) === 2) {                                   // airborne: gravity once frozen
        const s = En.b(si + 1);
        if (s !== 0 && s !== 6) {
          const ry = s16(En.w(si + 5) - wposy);
          if (ry > 190) { En.pb(si, 0); Score = (Score + 100) & 0xFFFF; }
          else {
            const off = (s16(En.w(si + 3) - wposx) + 16 + (ry + 32) * 320) & 0xFFFF;
            if (buffer[off] >= 128) En.pb(si + 1, 4);
            En.pw(si + 5, En.w(si + 5) + 3);
          }
        }
      }
      if (En.b(si + 2) === 255) {
        En.pb(si + 1, 3);
        Score = Score < 100 ? 0 : Score - 100;
      }
      En.pb(si + 2, (En.b(si + 2) + 1) & 255);
    }
    // ENDMOLP
    const ctrl = En.b(si);
    donee: {
      if (ctrl === 10) {                                      // motion-mapped platform
        const k = (frameinc + En.w(si + 3)) & 127;
        const x = s16((Circular[k * 2] >> 1) + En.w(si + 3) - wposx);
        const y = s16((Circular[k * 2 + 1] >> 1) + En.w(si + 5) - wposy);
        if (!(x > 319 || x < -31 || y > 190 || y < -31)) ptile(x, y, En.b(si + 7));
        break donee;
      }
      if (ctrl === 9) {                                       // platform
        const x = s16(En.w(si + 3) - wposx), y = s16(En.w(si + 5) - wposy);
        if (!(x > 319 || x < -31 || y > 190 || y < -31)) ptile(x, y, En.b(si + 7));
        En.pw(si + 3, En.w(si + 3) + En.sb(si + 11));
        En.pb(si + 9, (En.b(si + 9) + En.b(si + 11)) & 255);
        const d = En.b(si + 9);
        if (!(d < En.b(si + 10))) En.pb(si + 11, (-En.b(si + 11)) & 255);
        if (d === 0) En.pb(si + 11, (-En.b(si + 11)) & 255);
        break donee;
      }
      if (ctrl === 7 || ctrl === 8) {                         // restart flag / end of level flag
        const x = s16(En.w(si + 3) - wposx), y = s16(En.w(si + 5) - wposy);
        if (!(x > 640 || x < -320 || y > 400 || y < -200) && !(x > 319 || x < -31 || y > 190 || y < -31)) ptile(x, y, En.b(si + 7));
        break donee;
      }
      // collision with the main character
      if ((MCB & 4) || ctrl === 0 || ctrl > 8) continue;
      let bx = s16(En.w(si + 3) - wposx + 16);
      if (bx >= 184 || bx <= 139) break donee;
      bx = s16(En.w(si + 5) - wposy + 16);
      if (bx <= 76 || bx >= 124) break donee;
      enddetect: {
        if (ctrl === 3) {                                     // acid
          JumpStat = 15; MCB |= 12;
          Energy = Energy <= 24 ? 0 : Energy - 25;
          break enddetect;
        }
        if (ctrl === 4) {                                     // spring
          GravStat = 64; JumpStat = En.b(si + 10); MCB |= 8; ps2(10);
          break enddetect;
        }
        if (En.b(si + 1) !== 0) break enddetect;
        Energy = Energy <= 4 ? 0 : Energy - 5;
        JumpStat = 10; MCB |= 12; HitCount = 35;
        if (En.sb(si + 11) < 0) { MomLeft = 80; MCB &= 254; }
        else { MomRight = 80; MCB |= 1; }
      }
      const st = En.b(si + 1);
      if (st === 0 || st === 6 || JumpStat !== 0) break donee;
      if (s16(En.w(si + 5) - wposy) <= 100) break donee;
      En.pb(si + 1, 4); GravStat = 64; JumpStat = 8; MCB |= 8;     // jump on enemy
    }
    // ice ball collision
    if (IceBallNo === 4) continue;
    for (let k = 0; k < 4; k++) {
      const di = k * 6;
      ibhit: {
        if (IB.b(di) === 0) break ibhit;
        dl = 0;
        let ax = s16(IB.w(di + 3) + 8), bx = En.sw(si + 5);
        if (ax <= bx) break ibhit;
        bx += 32; if (ax >= bx) break ibhit;
        dl = 1;
        ax = s16(IB.w(di + 1) + 8); bx = En.sw(si + 3);
        if (ax <= bx) break ibhit;
        bx += 32; if (ax >= bx) break ibhit;
        if (En.b(si) === 7) {                                 // restart point
          Level.pw(lp, En.w(si + 3) - 112);
          Level.pw(lp + 2, En.w(si + 5) - 84);
          Level.pb(lp + 4, En.b(si + 8));
          for (let q = 0; q < 100; q++) if (En.b(q * 13) === 7) En.pb(q * 13 + 7, 60);
          En.pb(si + 7, 61);
        }
        if (En.b(si) === 8) {
          if (LevelEnd === 0) LevelEnd = 128;
          En.pb(si + 7, 63);
        }
        if (En.b(si + 1) === 0) En.pb(si + 1, 1);
        IB.pb(di, 0);
        IceBallNo++;
      }
      // airborne enemies dodge the ice ball
      if (En.b(si + 1) === 0 && En.b(si) === 2) {
        let dodge = false, reset = false;
        if (!(En.b(si + 12) & 4)) reset = true;
        else if (dl !== 1) reset = true;
        else {
          const rx = s16(En.w(si + 3) - wposx);
          dodge = rx > 144 ? !!(MCB & 1) : !(MCB & 1);
        }
        if (dodge) {
          En.pb(si + 12, En.b(si + 12) | 2);
          let a = HalfFrame;
          if (!(En.b(si + 12) & 1)) a = -a;
          En.pw(si + 5, En.w(si + 5) + a);
        } else if (reset) {
          if (En.b(si + 12) & 2) { En.pb(si + 12, En.b(si + 12) & ~2); En.pb(si + 12, En.b(si + 12) ^ 1); }
        }
      }
    }
  }

  // ---- initiate ice ball
  if (Firing === 20) {
    for (let k = 0; k < 4; k++) {
      const si = k * 6;
      if (IB.b(si) !== 0) continue;
      if (MCB & 1) { IB.pb(si, 2); IB.pw(si + 3, wposy + 87); IB.pw(si + 1, wposx + 168); }
      else { IB.pb(si, 1); IB.pw(si + 3, wposy + 87); IB.pw(si + 1, wposx + 136); }
      IB.pb(si + 5, 0);
      IceBallNo = (IceBallNo - 1) & 255;
      ps2(12);
      break;
    }
  }
  // ---- animate ice balls
  if (IceBallNo !== 4) {
    for (let k = 0; k < 4; k++) {
      const si = k * 6;
      if (IB.b(si) === 0) continue;
      const cx = s16(IB.w(si + 1) - wposx), dx = s16(IB.w(si + 3) - wposy);
      if (buffer[(cx + 8 + (dx + 8) * 320) & 0xFFFF] >= 128) { IB.pb(si, 0); IceBallNo++; continue; }
      if (!(dx > 192 || dx < -16 || cx > 319 || cx < -16)) psprite16(cx, dx, 0, IB.b(si + 5));
      if (IB.b(si) === 1) { IB.pw(si + 1, IB.w(si + 1) - 6 - (RapidFire ? 3 : 0)); }
      else if (IB.b(si) === 2) { IB.pw(si + 1, IB.w(si + 1) + 6 + (RapidFire ? 3 : 0)); }
      const x2 = s16(IB.w(si + 1) - wposx);
      if (x2 > 319) { IceBallNo++; IB.pb(si, 0); }
      else if (!(x2 > -16)) { IceBallNo++; IB.pb(si, 0); }
      IB.pb(si + 5, (IB.b(si + 5) + 1) & 31);
    }
  }
}

// ===================================================================================
// GameLoop (FREEZE.CPP)
// ===================================================================================
const parity8 = v => { v &= 255; v ^= v >> 4; v ^= v >> 2; v ^= v >> 1; return (~v) & 1; };   // PF set = even parity

const LAST_LEVEL = 1;
let gameWon = false, gameOverFlag = false;
async function FinishedLevel() {
  await Fade2Black(0);
  if (CurrentLevel >= LAST_LEVEL) {                          // the end: show the win screen
    Music.stop();
    await Endings.win({ score: Score, lives: Lives, timer: Timer });
    CurrentLevel++; LastStat = 14; gameWon = true;
    return;
  }
  LoadLogo(7);
  SRCopy();
  await FadeFromBlack(0);
  await sleep(300);
  await WAIT();
  await Fade2Black(0);
  CurrentLevel++;
  LastStat = 14;
}

// returns 0 = quit / no lives, 1 = level completed
async function GameLoop() {
  DXPos = wposx; DYPos = wposy;
  for (;;) {
    for (const [k, bit] of [[59, 1], [60, 2], [61, 4], [62, 8], [63, 16]]) {
      if (keys[k]) {
        while (keys[k]) await vsync();
        GameSetup ^= bit;
        if (bit === 8) Screen(GameSetup & 8 ? 1 : 0);
        if (bit === 1 && (GameSetup & 1) && !AudioOn) GameSetup &= 254;
      }
    }
    if (keys[38]) LevelEnd = 10;                              // DEV keys: L, T, A
    if (keys[20]) Timer = 1500;
    if (keys[30]) SteamCount = 8;

    skyrain(); skyplot(); bridges(); backplot(); tileanim(); statbar(); collani();

    // ---- Gravity
    let di = 32160, ah = 0;
    for (let cx = 255; cx > 0; cx--) {
      if (buffer[di & 0xFFFF] >= 128) break;
      di += 320; ah++;
    }
    if (ah <= 19) MCB &= 253; else MCB |= 2;
    if (ah >= 128 || (ah >= 19 && !(MCB & 8))) {              // falling
      let bx = GravStat;
      if (bx < 129) GravStat++;
      wposy = s16(wposy + (bx >> 5));
    } else if (ah <= 16) GravStat = 64;
    // terrain handler
    if (ah > 16 && ah < 19) wposy = s16(wposy + ah - 16);
    if (ah < 16 && ah > 4) wposy = s16(wposy - (16 - ah));

    // ---- Plot main character
    Collision = pmain(MCB, Frame);
    if (!(LastStat === 12 || (MCB & 4) || Firing === 0)) {
      let ax, bx, f;
      if (!(MCB & 1)) { ax = 144; bx = 84; if (!(Collision & 1)) { bx = 86; ax = 142; } if (MCB & 8) { bx = 84; ax = 142; } f = 8; }
      else { ax = 160; bx = 84; if (!(Collision & 1)) { bx = 86; ax = 162; } if (MCB & 8) { bx = 84; ax = 162; } f = 4; }
      psprite16(ax, bx, 15, f);
    }
    if (Timer !== 0 && Timer <= 1344) psprite16(152, 84, 5, BonusFrame);   // sweat

    steam();
    flndrain();

    // ---- Suicide / Quit (DEV)
    if (keys[31]) Energy = 0;
    if (keys[1]) return 0;
    if (keys[16]) return 0;

    // ---- Left / Right
    let al = 0;
    if (GameSetup & 16) al = 3;                               // ReadAxis(): no gamepad connected
    let bx = 0, cx = 0;
    if (buffer[36656] >= 128) cx = 1;
    if (buffer[36624] >= 128) bx = 1;
    cx += cx; bx += bx;
    let dl = 1;
    if (TractionType !== 0) dl = HalfFrame;
    const air = !(Collision & 1);
    if (air) {
      if (!wasAir) { airVX = groundVX; airFrac = 0; wasAir = true; }
      if ((MCB & 4) && HitCount === 35) { if (MomRight === 80) airVX = -4; else if (MomLeft === 80) airVX = 4; MomLeft = MomRight = 0; }
      MCB &= 191;
      airSteer();
    } else {
      if (wasAir) { landMomentum(); wasAir = false; }
      const x0 = wposx;
      left: {
        if (!(al === 1 || keys[203])) break left;
        if (!(Collision & 1)) { dl = 1; if (Collision & 12) break left; }
        if (Collision & 4) { MomRight = 0; break left; }
        MCB &= 254; MomLeft = 0;
        if (MomRight > 30) { wposx = s16(wposx + bx - 5); break left; }
        if (MomRight >= 15 && MomRight <= 30) { wposx = s16(wposx - 2); MomRight = (MomRight + dl + dl) & 255; break left; }
        wposx = s16(wposx - 1); MomRight = (MomRight + dl) & 255;
      }
      right: {
        if (!(al === 2 || keys[205])) break right;
        if (!(Collision & 1)) { dl = 1; if (Collision & 48) break right; }
        if (Collision & 16) { MomLeft = 0; break right; }
        MCB |= 1; MomRight = 0;
        if (MomLeft > 30) { wposx = s16(wposx - (cx - 5)); break right; }
        if (MomLeft >= 15 && MomLeft <= 30) { wposx = s16(wposx + 2); MomLeft = (MomLeft + dl + dl) & 255; break right; }
        wposx = s16(wposx + 1); MomLeft = (MomLeft + dl) & 255;
      }
      // ---- Momentum handler
      MCB &= 191;
      mn: {
        if (al !== 0) break mn;
        if (keys[203] || keys[204] || keys[205] || keys[206]) break mn;
        MCB |= 64;
        dl = 25;
        if (TractionType !== 0) dl = 15;
        if (!(Collision & 1)) dl = 25;
        if (!(Collision & 4)) {
          if (MomRight >= dl) { wposx = s16(wposx - 3); MomRight--; break mn; }
          if (MomRight > 10) { wposx = s16(wposx - 1); MomRight -= 2; break mn; }
        }
        MomRight = 0;
        if (!(Collision & 16)) {
          if (MomLeft >= dl) { wposx = s16(wposx + 3); MomLeft--; break mn; }
          if (MomLeft > 10) { wposx = s16(wposx + 1); MomLeft -= 2; break mn; }
        }
        MomLeft = 0;
      }
      groundVX = groundVX * .5 + s16(wposx - x0) * .5;
    }
    if (keys[24]) await sleep(50);                            // DEV: O = slow motion
    if (keys[25]) await Pause();

    // ---- Fire
    if (Firing !== 0) {
      Firing--;
      if (Firing !== 0 && RapidFire !== 0) Firing--;
    }
    if (keys[56] && IceBallNo !== 0 && Firing === 0) Firing = 20;
    // ---- Jump
    jump: {
      if (!keys[29]) { MCB &= 239; break jump; }
      if (!(Collision & 1) && !(MCB & 16)) break jump;
      MCB |= 16;
      const max = SuperJ !== 0 ? 15 : 10;
      if (JumpStat >= max) { GravStat = 64; MCB &= 239; break jump; }
      JumpStat += 2; MCB |= 8;
    }
    // ---- Jumping
    let falling = false;
    if (MCB & 8) {
      JumpStat = (JumpStat - 1) & 255;
      wposy = s16(wposy - (JumpStat < 5 ? JumpStat : 8));
      if (JumpStat === 0) MCB &= 247;
      if (Collision & 2) { JumpStat = 0; MCB &= 247 & 239; GravStat = 64; falling = true; }
    }
    // ---- Main character frame control
    anim: {
      if (!falling) {
        if (LastStat === 13) {
          if (Frame === 143) LastStat = 14; else Frame++;
          break anim;
        }
        if (MomLeft === 0 && MomRight === 0) {                // standing / teetering
          MCB &= 191;
          if ((Collision & 65) === 65) {
            if (LastStat === 1) { if (Frame === 83) Frame = 68; else Frame++; }
            else { Frame = 68; LastStat = 1; }
          } else if (!(MCB & 2)) {
            if (LastStat === 2) { if (Frame === 3) Frame = 0; else Frame++; }
            else { Frame = 0; LastStat = 2; }
          }
        }
        if (MCB & 4) {                                        // hit
          if (LastStat !== 3) { Frame = 88; LastStat = 3; }
          else if (Frame === 103) Frame = 88; else Frame++;
          HitCount = (HitCount - 1) & 255;
          if (HitCount === 0) MCB &= 251;
          else if ((Collision & 1) && !(MCB & 8)) MCB &= 251;
          break anim;
        }
        if (TractionType !== 0 && (MCB & 64)) {               // slowing
          if (LastStat !== 16) { Frame = 84; LastStat = 16; }
          else if (Frame === 87) Frame = 84; else Frame++;
        } else if (MomLeft !== 0 || MomRight !== 0) {         // walking
          if ((MCB & 64) || !(MomLeft > 30 || MomRight > 30)) {
            if (LastStat !== 4) { Frame = 4; LastStat = 4; }
            else if (Frame === 35) Frame = 4; else Frame = (Frame + HalfFrame) & 255;
          } else {
            if (LastStat !== 4) { Frame = 4; LastStat = 4; }
            else if (Frame === 35) Frame = 4; else Frame++;
          }
        }
        if (MCB & 8) {                                        // jumping
          if (LastStat !== 5) { Frame = 36; LastStat = 5; }
          else if (Frame === 51) Frame = 36; else Frame++;
          break anim;
        }
      }
      if (MCB & 2) {                                          // falling
        if (LastStat !== 6) { Frame = 52; LastStat = 6; }
        else if (Frame === 67) Frame = 52;
        else { if (Frame === 60) ps2(13); Frame++; }
      }
    }

    // ---- Zero time checking
    if (Timer !== 0) {
      Timer = (Timer - 1) & 0xFFFF;
      if (Timer === 0) {
        for (let i = 0; i < 400; i++) {
          if (Bn.b(i * 5) === 7) {
            const c = SEED >>> 31; SEED = (SEED << 1) >>> 0; if (c) SEED = (SEED ^ 0x1D872B41) >>> 0;
            Bn.pb(i * 5, (SEED & 3) + 10);
          }
        }
      }
    }
    // ---- Zero energy checking
    let skipPal = false;
    if (TEnergy === 0) {
      if (LastStat < 13) { LastStat = 13; Frame = 112; }
      if (LastStat === 14) {
        if (Lives === 0) { gameOverFlag = true; return 0; }
        await Fade2Black(0);
        Lives--; Energy = 100; TEnergy = 100;
        wposx = Level.sw(lp); wposy = Level.sw(lp + 2);
        await vsync();
        skipPal = true;
      } else { DXPos = wposx; DYPos = wposy; }
    }
    // ---- End of level
    if (!skipPal) {
      if (LevelEnd !== 0) {
        LevelEnd--;
        if (LevelEnd === 1) { LevelEnd--; await FinishedLevel(); return 1; }
        if (Timer !== 0) Timer = (Timer + 1) & 0xFFFF;
      }
      // ---- Sky colour control (64-95)
      let a = frameinc & 4095;
      if (a === 4095) SkyBrightCt = 1;
      a &= 3;
      if (a === 3 && SkyBrightCt !== 0) {
        for (let k = 0; k < 32; k++) palMain[192 + k * 3 + 1] = (palMain[192 + k * 3 + 1] + (SkyBrightDr === 1 ? 1 : -1)) & 255;
        if (--SkyBright === 0) { SkyBright = 63; SkyBrightDr ^= 1; SkyBrightCt = 0; }
      }
      // ---- Cycle colours (96-103)
      const o = 288, r0 = palMain[o], g0 = palMain[o + 1], b0 = palMain[o + 2];
      for (let k = 0; k < 7; k++) { palMain[o + k * 3] = palMain[o + k * 3 + 3]; palMain[o + k * 3 + 1] = palMain[o + k * 3 + 4]; palMain[o + k * 3 + 2] = palMain[o + k * 3 + 5]; }
      palMain[o + 21] = r0; palMain[o + 22] = g0; palMain[o + 23] = b0;
      // clouds follow the sky: bright by day, dark bluish-grey at night
      const lvl = SkyBrightDr === 0 ? SkyBright / 63 : 1 - SkyBright / 63;
      for (let c = 0; c < 3; c++) {
        palMain[165 + c * 3] = Math.round(cloudBase[c * 3] * (.28 + .72 * lvl) * (.72 + .28 * lvl));
        palMain[166 + c * 3] = Math.round(cloudBase[c * 3 + 1] * (.28 + .72 * lvl) * (.78 + .22 * lvl));
        palMain[167 + c * 3] = Math.round(cloudBase[c * 3 + 2] * (.30 + .70 * lvl));
      }
      await vsync();
      if (!(MCB & 128)) { dac.set(palMain.subarray(192, 192 + 120), 192); dac.set(palMain.subarray(165, 174), 165); }
    } else {
      // NOSCCS: straight to buffer copy
    }
    vram.set(buffer.subarray(0, 64000));
    if (MCB & 128) { await FadeFromBlack(0); MCB &= 127; }

    // ---- New life initialise
    if (LastStat === 14) {
      LastStat = 2; MomLeft = 0; MomRight = 0; SteamCount = 0; Frame = 0;
      TScore = Score; TTimer = Timer; TEnergy = Energy;
      MCB |= 129; MCB &= 247 & 251; airVX = groundVX = airFrac = 0; wasAir = false;
      if (Level.b(lp + 4) === 1) MCB |= 1; else MCB &= 254;
    }
    // ---- Random steam
    { const c = SEED >>> 31; SEED = (SEED << 1) >>> 0; if (c) SEED = (SEED ^ 0x1D872B41) >>> 0; }
    if ((SEED & 0xFFFF) <= 127) { SteamCount = 8; ps2(9); }

    HalfFrame = parity8((frameinc + 1) & 255);
    frameinc = (frameinc + 1) & 0xFFFF;
  }
}

// ===================================================================================
// Outro (CORE.H) and main (FREEZE.CPP)
// ===================================================================================
async function Outro() {
  vram.fill(0); buffer.fill(0);
  dac.fill(0);
  vram.fill(1, 0, 64000);
  for (let bl = 0, n = 0; n < 32; n++, bl += 2) {
    await vsync(); await vsync();
    dac[3] = dac[4] = dac[5] = bl;
  }
  let di = 0, si = 64315;
  for (let n = 0; n < 25; n++) {
    await vsync();
    for (let k = 0; k < 320; k++) { for (let j = 0; j < 4; j++) { vram[(di + j) & 0xFFFF] = 0; vram[(si + j) & 0xFFFF] = 0; } di += 4; si -= 4; }
  }
  di = 32000; si = 32317;
  for (let k = 0; k < 40; k++) {
    await vsync();
    for (let j = 0; j < 4; j++) { vram[(di + j) & 0xFFFF] = 0; vram[(si + j) & 0xFFFF] = 0; }
    di += 4; si -= 4;
  }
  for (let bl = 63; bl > 0; bl--) { await vsync(); dac[3] = dac[4] = dac[5] = bl; }
  await vsync();
  await sleep(1000);
}

async function main() {
  await loadAssets();
  // wait for a key press: browsers only allow audio after a user gesture
  document.getElementById('msg').textContent = 'Tap or press any key to start';
  await new Promise(r => { startResolve = r; document.addEventListener('pointerdown', () => { unlockAudio(); if (startResolve) { const s = startResolve; startResolve = null; s(); } }, { once: true }); });
  document.getElementById('msg').textContent = '';
  UI.init();
  await window.__loadSounds();
  AudioOn = introBuf || SFX.some(Boolean) ? 1 : 0;

  const ticks = Math.floor(((Date.now() - new Date().setHours(0, 0, 0, 0)) / 1000) * 18.2065);
  SEED = ticks >>> 0; SEED2 = (SEED ^ 0x1D872B41) >>> 0;

  await LoadTitle();
  LoadGame();
  if (!AudioOn) GameSetup &= 254;
  for (let i = 0; i < 192 * 3; i++) dac[i] = 0;

  for (;;) {                                                  // STARTMENU
    if (await MainMenu()) break;                              // quit
    Level.a.set(file('ldata.dat').subarray(0, 4680));
    Score = 0; CurrentLevel = 1; Lives = 3;
    gameWon = false; gameOverFlag = false;
    for (;;) {                                                // STARTLEVEL
      lp = CurrentLevel * 468;
      TractionType = Level.b(lp + 5);
      await LoadLevel();
      resetDropsInit();
      wposx = Level.sw(lp); wposy = Level.sw(lp + 2);
      Timer = TTimer = Level.w(lp + 8);
      TScore = Score;
      SkyBright = 63; SkyBrightDr = 0; SkyBrightCt = 0;
      MCB |= 1; MCB |= 128; MCB &= 239 & 247 & 251;
      Frame = 0; SuperJ = 0; MomLeft = 0; MomRight = 0; JumpStat = 0; LastStat = 0; Energy = 100; TEnergy = 100;
      RapidFire = 0; SteamCount = 0; atilecount = 0;
      airVX = groundVX = airFrac = 0; wasAir = false; EnemySFX.reset();
      Music.play('level');
      const r = await GameLoop();
      if (gameWon) break;                                     // win screen already shown
      if (r === 0) {
        await Fade2Black(0);
        if (gameOverFlag) { Music.stop(); await Endings.gameOver({ score: Score }); gameOverFlag = false; }
        break;
      }
    }
  }
  saveSetup();
  Music.stop();
  await Outro();
  document.getElementById('msg').textContent = 'FREEZ (C) Digital Synapse 1996 - press any key to play again';
  await new Promise(r => { startResolve = r; });
  location.reload();
}
main().catch(e => { document.getElementById('msg').textContent = 'Error: ' + e.message; console.error(e); });
