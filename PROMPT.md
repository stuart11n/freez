# Prompt: build "FREEZ 2026" (a 1996 DOS platformer, remade for the browser)

Paste everything below the line into any capable coding LLM. It is written to be self-contained: the model
should not need to see the original source. Numbers are the exact values the finished game uses, so the
result feels the same. Sections 14 and 15 say what to do about art and how to check the result.

---

## 0. Your task

Build a complete, single-page, dependency-free browser game called **FREEZ**: a side-scrolling platformer
where you play a small purple dragon who throws **ice balls** that freeze enemies. It is a faithful remake
of a 1996 DOS game (320x200, 256-colour VGA, 70 Hz) plus a modern "2026" layer: smooth upscaling, a CRT
shader, touch controls, a tracker-style music engine, synthesised sound effects, win and game-over screens,
and a much longer level 1.

Deliver plain files served statically (no build step, no npm, no frameworks): `index.html` plus a few
`.js` files and data assets. It must run from any static web server, on desktop and on phones.

Write real, working code. Do not stub features. When something below gives a number, use that number.

## 1. Look and feel

- Cheerful 90s platformer look: a bright cyan/green sky gradient, rolling brown rock hills with dark-green
  grass tops, trees, clouds, flowers, red mushrooms, white spikes, blue water, a purple dragon, yellow
  hopping blobs ("hoppers"), blue flying birds, gold coins, red/white gems.
- Everything is drawn into a **320x200, 8-bit palettised framebuffer** and then scaled up. Game area is
  320x192; the bottom 8 rows are the status bar.
- Gameplay runs on a fixed **70 ticks per second** clock regardless of display refresh rate (use an
  accumulator on `requestAnimationFrame`; on a 60 Hz display run two ticks in some frames).

## 2. Architecture (files)

```
index.html          page, canvas, touch controls, settings panel, script tags (add ?v=N to every src for cache busting)
tables.js           Circular[256] (int16) and BridgeDynamics[1024] (int16) lookup tables
gfx.js              Settings + WebGL display pipeline (scaling filters, CRT) with a 2D-canvas fallback
music.js            tracker music engine + songs + playlist
enemysfx.js         enemy voices (bird squawk, hopper squish)
ui.js               settings panel + hotkeys (F6 CRT, F7 scaling, F8 music)
endings.js          win screen and game-over screen
freez.js            the game: runtime, loaders, menus, level loop, collision, entities, sound effects
touch.js            on-screen buttons that dispatch synthetic KeyboardEvents
manifest.webmanifest  display=fullscreen, orientation=landscape
assets/             sprite sheets, tile sets, maps, level record, WAV samples (see section 14)
```

The whole game is **async/await around a `vsync()` primitive**: `await vsync()` presents the current
framebuffer with the current palette and then waits for the next 70 Hz tick. All fades, menus and the
title sequence are loops that `await vsync()`.

## 3. Video model

- `vram` (64000 bytes) is what is displayed; `buffer` (64000+ bytes, allocate 65536) is the off-screen
  frame the game draws into. **The buffer is also the collision map** (see 5.1), so draw order matters.
- A 768-byte `dac` palette (6-bit components, 0-63). `present()` expands to 8-bit with `(v<<2)|(v>>4)`.
- Palette fades: `Fade2Black(flag)` (64 steps, or 20 if flag) decrements every component toward 0 once per
  tick; `FadeFromBlack(flag)` (64 or 24 steps) increments toward the target palette. Both wait for vsync
  each step. A saved palette allows pause (fade out 20 steps, wait for key, fade in 24 steps).
- Palette index meaning in game: **index >= 128 is solid ground**; 64-95 is the sky gradient (32
  colours); 96-103 are 8 cycling colours (water/rain) rotated one step every tick; 1 is white (lightning,
  spikes); 0 is transparent in all sprites.

## 4. Title sequence, menus, screens

1. "Tap or press any key to start" (needed for audio unlock), then a TV-tuning intro: ~1 s of grey snow
   fading in (64 ticks), 100 ticks of flickering noise where the picture flashes in at random, 50 ticks of
   the picture with noise bands, 1 s pause, then the greyscale picture ramps to full colour over 240
   ticks, then fade to black. Holding Space skips it. The picture is a **"DIGITAL SYNAPSE" / "presents"**
   card (chrome lettering on dark blue with a glowing neural-network motif).
   *Important:* because the picture first appears through a grey ramp (grey = index/4), **its palette must
   be sorted by brightness.**
2. Main menu: the FREEZ logo and four boxed icons (Play, Options, Info, Quit). Left/Right moves a yellow
   highlight, Enter/Space selects. Info shows an instructions picture. Quit goes to the outro (a CRT
   "switch-off": picture shrinks to a line then a dot, then fades) and offers a restart.
3. Options: five toggles with lights (Sound, Weather, Parallax sky, Widescreen (no-op), Gamepad (no-op)) and
   "main menu". Persist settings in `localStorage`.
4. Before each level a level-intro picture fades in, **holds for one second**, then fades out while the
   level loads.
5. Win screen (after the last level) and game-over screen: see section 13.

## 5. Core gameplay engine

World coordinates are pixels. `wposx, wposy` is the world position of the top-left of the screen. The
dragon is always drawn at screen (144, 84), a 32x32 sprite; the world scrolls under it.

### 5.1 Frame order (every tick)

1. Sky gradient into the whole buffer, then (if weather on and night) lightning and background rain.
2. Parallax sky tiles (optional setting), bridges, land tiles (background layer), animate tiles,
   status bar, then **all enemies, bonuses, ice balls and platforms** (collani).
3. Gravity probe and terrain adjustment (reads the buffer).
4. Draw the dragon; this also performs the collision probes (reads the buffer).
5. Foreground land tiles, foreground rain, steam puffs.
6. Controls (left/right, momentum, fire, jump), frame animation, timers, death and level-end checks, sky
   colour cycle, palette cycle, `await vsync()`, copy buffer to video, fades, respawn setup.

### 5.2 Collision is pixel based

The buffer holds palette indices; **any pixel with index >= 128 is solid**. The dragon probes fixed buffer
offsets around its sprite:
- ground: offsets `37918`, `37922` (feet), teetering probes `42074-1600-320` and `42086-1600-320`
- ceiling: `27024+20-6*320`, `27024+12-6*320`
- left wall: base `28308`; upper probes (bit 4): `28308`, `+2560`, `+3840`; lower probes (bit 8): `+5120`, `+6400`, `+8964` and `35988`
- right wall: base `28332`; upper probes (bit 16): `28332`, `+2560`, `+3840`; lower probes (bit 32): `+5120`, `+6400`, `+8956` and `36012`
The result is a bit mask: 1 ground, 2 ceiling, 4/8 left wall upper/lower, 16/32 right wall upper/lower, 64
edge/teetering.
Enemy "gravity" and ice-ball wall hits use the same trick: read the pixel at the sprite's position.

### 5.3 Gravity

Probe straight down from screen (160, 100) up to 255 rows; `dist` = rows until a solid pixel.
On ground if `dist <= 19`. If not on ground (or `dist >= 19` and not jumping): fall by `GravStat >> 5`
px/frame where `GravStat` starts at 64 on landing and is incremented each frame up to 129. If on ground and
`dist <= 16`, `GravStat = 64`. Terrain steps: if `16 < dist < 19` move down `dist-16`; if `4 < dist < 16`
move up `16-dist` (this is how slopes are walked).

### 5.4 Horizontal movement (ground)

Keep one momentum counter per direction. While a direction key is held its counter grows by 1 per frame
(in the original naming the counter for "moving left" is called `MomRight`, which is only a naming quirk);
on slippery traction it grows by the half-frame flag (0/1) instead. Speed by counter: `< 15` -> 1 px/frame; `15..30` -> 2 px/frame; `> 30` -> 5 px/frame (3 on a
slope). Releasing keys decelerates: counters decay (-1 or -2 per frame) while moving 3 px/frame (>= 25) or
1 px/frame (>10). Slippery levels use deceleration thresholds 15 instead of 25. Wall bits block movement
and zero the counter.

### 5.5 Jumping

On the ground, pressing jump sets the jump flag and adds 2 to `JumpStat` per frame while held, up to 10
(15 with the super-jump bonus). Every frame while jumping: `JumpStat--`, rise `JumpStat` px if it is < 5
else 8 px. Net rise is about 100 px (3 tiles). Hitting the ceiling ends the jump. Gravity is suppressed
while jumping.

### 5.6 2026 additions to jumping (keep exactly)

- **Air inertia.** On the first airborne frame store `airVX` = smoothed ground velocity (average of the
  last frames, positive = right). In the air, ignore the classic momentum code and do: steering target is
  +/-1.7 px/frame; if no key is held `airVX` drifts toward 0 by 0.012 per frame; if a key is held, move
  `airVX` toward the target by at most **0.04 per frame** (but never slow a run that is already faster than
  1.7 in that direction). Wall bits zero it. Accumulate a fractional position and move whole pixels. On
  landing convert back: `|airVX| >= 3.5 -> momentum 35`, `>= 1.5 -> 20`, `> 0.3 -> 10`, else 0, in the
  matching direction. A hit during a jump sets `airVX` to +/-4.
- **Wing flap.** A *new* jump key press while airborne (edge detect; once per airtime; not while hit or
  dying) sets `JumpStat = 12`, sets the jumping flag, resets `GravStat = 64`, plays the flap sample, and
  shows the wings-out frames (the falling animation frames 52..67 mapped over 22 frames). Measured result:
  a flap near the top of a normal jump adds about 20-25 px.

### 5.7 Character frames (36 sprite images, frame number = sprite index * 4)

0-3 standing (loop), 4-35 walking (run uses all, walk steps by the half-frame flag), 36-51 jumping, 52-67
falling/wings, 68-83 teetering at an edge, 84-87 slowing on slippery ground, 88-103 hit, 112-143 death
(then life lost). Facing is a bit; left-facing sprites are drawn mirrored. While firing, a separate 16x16
"open mouth" head sprite is drawn over the head.

### 5.8 Firing

Fire: if fewer than 4 ice balls are active and the fire delay is 0, set delay 20; on the next frame spawn an
ice ball in the facing direction at (wposx+168 or +136, wposy+87), speed 6 px/frame (+3 with rapid fire).
The delay counts down 1 per frame (2 with rapid fire). Ice balls animate 32 frames, die on solid pixels or
when 16 px off screen.

## 6. Entities (fixed-size records, 200 enemies, 400 bonuses)

Enemy record (13 bytes): `ctrl, status, fcount, x(int16), y(int16), type, frame, disp, maxdisp,
speed(int8), dodge`.
`ctrl`: 0 off, 1 ground walker, 2 flyer, 3 invisible acid (deadly), 4 invisible spring, 7 restart flag, 8
end-of-level flag, 9 horizontal platform, 10 circling platform. `type` picks a sprite sheet (0 bird,
1 hopper) or a tile id for platforms and flags.

- Walkers/flyers patrol: move by `speed` each frame, `disp` accumulates `speed` (byte arithmetic) and the
  direction flips when `disp >= maxdisp` or `disp == 0`. Use positive speeds with `maxdisp` for a patrol
  `[x, x+maxdisp]`. They reverse to face an ice ball when lined up with it and the dragon is firing.
  Flyers dodge ice balls (move up/down by the half-frame flag while an ice ball is Y-aligned).
- Animation: the frame number increments every other tick and the drawn image is `frame >> 2` (each sheet
  has 19 images). Frame ranges: 0-31 normal movement loop, 32-43 being hit, 44-47 frozen loop, 48-59
  thawing, 60-75 dying/vanishing. Sprites with negative speed are drawn mirrored.
- Status machine: 0 normal. Ice ball hit -> 1 (hit) -> plays frames 32-43 -> 2 (frozen) -> status 5
  (frame loop 44-47, freeze counter counts to 255) -> 3 (thaw, **-100 score**) -> back to 0. Frozen
  flyers fall 3 px/frame; landing on solid sets 4 (shatter). Status 4 -> 6 (dying animation, **+100
  score**, kill sound) -> removed at frame 75. Jumping on an enemy from above (dragon rising JumpStat 0,
  enemy y below the dragon's centre) also sets status 4 and bounces the dragon (`JumpStat=8`).
- Contact damage (enemy status 0, dragon not already hit): Energy -5, knockback (momentum 80 away from the
  enemy's direction), `JumpStat=10`, hit flag, `HitCount=35` (flash/invulnerable until it expires or you
  land). Acid: Energy -25 and `JumpStat=15` bounce. Spring (ctrl 4): `JumpStat = maxdisp` (e.g. 24-30), jump
  flag, spring sound. Range test for contact: enemy centre within x 139..184 and y 76..124 of the screen.
- Restart flag (ctrl 7): when an ice ball hits it, store `Sx = x-112, Sy = y-84` and the facing into the
  level record, set that flag's tile to its "lit" id and reset all other flags (**only the first 100 enemy
  records are scanned, so keep flags in those**). End flag (ctrl 8): ice ball hit sets `LevelEnd = 128`
  frames; when it reaches 1 the level ends.
- Platforms (9/10) are drawn as tiles and so are solid through the pixel collision. Circling platforms
  move by `Circular[((frameinc + x) & 127)*2 .. +1] >> 1` px.

Bonus record (5 bytes): `ctrl/type, x, y`. Types: 1 gem +150, 2 coin +50, 3 energy +40 (cap 100, +100
score, ignored at full), 6 rapid fire (2048 ticks, +50), 7 time +3240 ticks (+100, only if a timer is
running), 8 extra life (+150), 9 super jump (2048 ticks, +50), 10-13 misc +50, 14 collect sparkle. Bonuses
bob by animating `(x + BonusFrame) & 31`, 4 frames per image, 8 images per type, 16x16. Collected bonuses
play 32 frames of the type-14 animation then disappear. If the timer reaches zero every time bonus is
converted into a random misc bonus.
Score is a 16-bit unsigned value shown with 6 digits; display counts toward it (+10 up / -2 down per frame).

## 7. Level data and rendering

- **Tile map**: 128x64 tiles of 32x32 px (a 4096x2048 px world), one byte per tile, row-major.
  Value 0 = empty. Bit 7 set = foreground tile (drawn after the dragon), low 7 bits are the tile id.
  Tile ids: 0-63 from sheet A (transparent, colour 0 = see-through), 64-95 from sheet B (transparent), 96-127
  from sheet B opaque. A tile image is 1024 bytes; sheet A is 64 tiles, sheet B is 64 tiles. Row skip
  optimisation: if the first byte of a tile row is 255 the row is empty.
- **Sky tile map** (parallax, optional): 64x32 tiles, scrolls at half speed, drawn behind the land.
- Animated tiles: the level record lists 32 tile-map byte offsets; every 8th tick advance them (+1 each
  step, -3 on the 4th step, a 4-frame water/leaf cycle).
- Level record (468 bytes, 10 levels; index = level number): `Sx(u16), Sy(u16), Face(u8), Traction(u8),
  Gems, Coins, Time(u16 ticks), ATiles[32](u16), BPos[16](x,y)` plus filename strings (unused).
- **Bridges**: up to 16 records `(x, y)` of 32 segments of 8x8 px spaced 8 px apart; each segment's y is
  offset by a `BridgeDynamics[row*32 + i]` value that depends on where the dragon stands, giving a sagging
  rope bridge.
- **Sky gradient**: palette indices 64-95 form a 32-colour gradient; it scrolls vertically with the camera
  in 3-row bands.
- **Day/night clock**: a tick counter advances once per simulation tick. Every 4096 ticks (about a minute) the
  world flips between day and night with a smooth fade of a brightness value `lvl` (1 = day, 0 = night) over
  252 ticks, never a hard cut; start in daytime. Night is `lvl < 0.5`. Each tick the sky palette (from its saved
  original colours) is multiplied per channel: R x (0.12 + 0.88 lvl), G x (0.06 + 0.94 lvl), B x (0.30 + 0.70
  lvl), so the sky goes deep blue rather than grey. Clouds use private palette entries 55-57 (remapped at load)
  multiplied by about `0.28 + 0.72 lvl` with a slightly bluer tint.
- **Night storm** (rain, lightning, thunder) runs only when it is night AND the Weather option is on AND the
  player is outdoors (world y of the player above tile row 33, i.e. not in the caves, hall or lower lane).
  - Rain: two pools of 200 drops, one drawn behind the world (right after the sky) and one in front (after
    the foreground tiles). Drops live in screen space: each tick move about 7 px down and 1 px sideways, then
    subtract the camera movement since the last frame (so rain does not slide when the view scrolls); wrap
    with modulo so respawns/teleports cannot break it. Draw each as a 3 px vertical streak in one of the
    cycling colours 96-103 so it shimmers.
  - Lightning: each tick with probability about 0.5% and at least 70 ticks (1 s) since the last strike, start a
    bolt: a random start x (32..288) plus 40 random horizontal offsets within +/-4.5 px. Draw it from the top
    of the screen as 40 segments of 5 px, 2 px wide, pure white (palette 1), behind the foreground terrain,
    for 5 ticks. While it is visible raise `lvl` by 0.6 (clamped) for the sky and clouds to flash them.
  - Thunder: on each strike (rate-limited, see 9.4).
- **Status bar** (8 rows at y=192): timer (hourglass icon + seconds = ticks>>6), energy (heart + 3 digits),
  lives, level number, score (6 digits), and icons for rapid fire and super jump that blink in their last 5
  seconds. Timer and energy icons blink when low.
- **Level 1 content**: the original map is a long winding level of stacked horizontal tiers connected by
  slopes, pits with spikes and acid, circling platforms, a tree canopy, a cave section and springs. It had **no
  end flag and an unfinished, floorless hall** in the lower left. The 2026 version completes it (section 12).

## 8. Display pipeline (gfx.js)

- Convert `vram` through the palette to a 320x200 RGBA texture each present, upload to WebGL, draw a
  full-screen quad with a fragment shader. Size the canvas to its on-screen pixel size x devicePixelRatio
  (4:3 display aspect), re-fit on resize. Provide a 2D canvas fallback (smoothed drawImage + dark scanline
  rects).
- **Scaling modes** (Settings.scale): 0 Pixel (nearest), 1 Sharp (sharp-bilinear: crisp texel centres, one
  output pixel of blend at the edges), 2 Smooth (Scale2x/EPX edge smoothing evaluated per output pixel).
- **CRT modes** (Settings.crt): 0 off; 1 scanlines (beam profile `exp(-((fy-.5)/.38)^2)`, strength .6, small
  gain); 2 full CRT: barrel curvature, full-strength beam scanlines, RGB phosphor mask (only when there are
  at least ~2.2 output pixels per texel), horizontal bloom, vignette, rounded corners, a faint flicker.
- Defaults: **CRT = Full, scaling = Smooth.** Persist in `localStorage`.

## 9. Audio

All audio is synthesised with Web Audio; the only audio files are the original WAV effects. Create the
AudioContext on the first key/tap. Everything respects the Music and Sound-FX toggles.

### 9.1 Sample effects
14 one-shot WAVs mapped to events: hurry up, coin/gem, bonus, rapid/super jump, extra life, time, energy,
enemy killed, restart flag, steam, spring, low energy, ice-ball fire, wing flap. Plus the intro jingle.

### 9.2 Tracker music engine (music.js)
A real tracker player, not a note list:
- Song = order list of patterns; pattern = rows x channels (use 5 channels: lead, arp, bass, drums, pad);
  32 rows per pattern; **speed 6 ticks per row; tick length = 2.5 / BPM seconds** (like a .MOD player).
- Cell syntax in text patterns: `NOTE[i<n>][v<0-64>][a<xyz hex>|w<xy>|u<xx>|d<xx>]`, `.` = nothing, `^^` =
  note cut. Effects run per tick: `a0xy` arpeggio (0,x,y semitones), `wxy` vibrato, `uxx/dxx` pitch slides.
- Instruments are tiny 8-bit looped single-cycle waveforms (quantise to 8 bits for tracker crunch) pitched
  by `playbackRate`: saw lead (64), 25% pulse (32), sub bass (64), pad (128, volume swells in over ~8 ticks),
  50% square, bell (additive), strings; one-shot drum samples (kick, snare, hat, tom) synthesised into
  AudioBuffers. Volume is stepped per tick with a per-instrument decay factor (the "zipper" envelope).
- Hard-panned Amiga stereo: lead L, arp R, bass R, drums L, pad slightly L. Master gain 0.2.
- Songs: **five in-game songs** (A minor 125 BPM; D minor 138 BPM four-on-the-floor; E minor 108 BPM bells
  over strings; C major 150 BPM bouncy square lead; F# minor 126 BPM syncopated breakbeat), a slow menu
  song, a win fanfare and a game-over jingle (the last two play once). Build songs from compact specs:
  chord progression per pattern (4 chords x 8 rows), lead from 10 rhythm templates indexed over chord
  tones, bass styles (octave, pump, walk, sync), drum styles (four, rock, break, hop) with a fill on the
  last block, optional chord stabs or sustained arpeggio, optional pad.
- **Playlist**: in-game, start on a **random** song, then every **60 seconds** fade out (1.6 s), switch to a
  *different* random song and fade in (1.6 s). Music stops on game over/win/quit.

### 9.3 Enemy voices (enemysfx.js)
Birds (ctrl 2, type 0): a two-chirp **squawk** (sawtooth, falling pitch ~1500->750 Hz with 42 Hz wobble,
band-passed ~1.7 kHz, plus breath noise), every 2.5-6 s per bird while near. Hoppers (ctrl 1, type 1): a wet
**squish** (triangle pitch bend 95->340->60 Hz plus resonant band-passed noise) at the moment the sprite
frame reaches the landing squat (frame >> 2 == 7). Active within **600 px** of the dragon (including
off-screen), quieter and panned by distance; only for enemies in status 0.

### 9.4 Thunder
On a lightning strike, 0.2-1 s later and rate-limited to one rumble per 9-18 s:
brown noise through a low-pass sweeping 1100->140 Hz, passed through a tanh waveshaper (so small speakers
reproduce it), with a crack, two rolling swells, 3-4.5 s long, peak gain about 0.3.

## 10. Input

Keyboard (set-1 style indexes; extended keys +128): arrows move; **jump = Ctrl, Z or Up; fire = Alt, X or
Shift**; Enter/Space select in menus; P pause (fade out, wait for any key, fade in); F1 sound, F2 weather,
F3 parallax, F4 widescreen (no-op), F5 gamepad (no-op); Esc/Q quit to menu; F6 CRT, F7 scaling, F8 music.
Original debug keys stay: L end level, T timer to 1500, A steam, S lose energy, O slow motion.
A very short keypress must still register: keep a key logically down for at least ~2 ticks after release.
Touch: left/right arrow buttons, FIRE, JUMP, OK, PAUSE, MENU, CRT buttons dispatch synthetic KeyboardEvents;
multi-touch; haptic tick; fullscreen button; gear button opens a panel (CRT, scaling, music, sound FX).
Portrait puts the screen on top with controls below; landscape puts the d-pad left and fire/jump right.
Prevent page scroll/zoom/context menu; honour safe-area insets.

## 11. Game flow and rules

- 3 lives, energy 100, a level timer in ticks (displayed `>>6` seconds; flashes under ~20 s). Energy 0 starts
  the death animation (frames 112-143), then respawn at the last restart flag (or the level start) with
  full energy and a fade-in; no lives -> game over screen.
- Level end: ice ball on the end flag -> 128 ticks later -> level-complete handling. After the last level
  show the win screen, then the menu. Score carries across levels.
- Win screen: night sky with twinkling stars, fireworks (particle bursts every ~38 frames), bobbing
  "CONGRATULATIONS!" title, "FREEZ COMPLETE", and a panel that tallies score, lives and a **time bonus =
  (remaining ticks >> 6) x 10** counting up, then FINAL score; "PRESS ANY KEY" blinks. Game over: dark red
  radial gradient, glitchy "GAME OVER" and the score. Both are drawn on a 2D canvas and quantised to the
  game's 8-bit palette (6x6x6 colour cube + 40 greys, ordered 4x4 dither) so they use the same fades.

## 12. The extended level 1 (2026 content)

Keep the original map, then:
1. Fill the unfinished hall (tile columns 6-71, floor row 37) with a grass-topped floor (tile 126 over dirt
   104, 3 rows thick), with a restart flag, pits with spikes (tile 55) and acid enemies and a circling
   platform, a short staircase of blocks, flyers and walkers, coins and a time bonus.
2. End the hall with a 2-tile shaft at columns 7-8 that drops into a new **long lower lane** (floor row 57,
   columns 6-126, ceiling = the underside of the existing mass at rows 40-42): landing zone, pit with a
   platform, a spring that launches onto a 2-high then a 4-high block (gems and a rapid-fire bonus on top),
   a double pit, a halfway restart flag, a spike corridor, a spring to a floating shelf with an extra life,
   a gauntlet of walkers/flyers, a last pit, a time bonus and **the end flag** at column 123.
3. Level time becomes 42000 ticks. Block edges use face tiles 1/2 (left) and 3/4 (right); pits use a spike row
   plus a floor row two tiles down so you can never fall out of the world.
Verify it is completable: a simple bot that holds right, jumps over gaps/walls and fires repeatedly must
reach the flag.

## 13. Persistence and polish

`localStorage`: option bits and display/music settings. Add `manifest.webmanifest` (fullscreen, landscape)
and version query strings on every script tag. Suspend audio when the tab is hidden. The fullscreen button
must request fullscreen on pointer-up, report errors in a toast, and on iOS say to use "Add to Home Screen".

## 14. Assets (read this if you do not have the original data files)

The game needs these assets. **If you have the original files, load them as raw data with the layouts
below. If you do not, generate equivalents (procedurally or with a script) matching the same layouts;
do not leave placeholders.** All images are 8-bit indexed with their own 256-colour palette; index 0 is
transparent in sprites and tiles.
- Raster files: a 18-byte TGA header, a 768-byte BGR palette at 0x12, raw top-down pixels at 0x312.
- `dragon` (main palette + 36 frames of 32x32 = 36864 bytes at 0x312), `tset1a`/`tset1b` (64 tiles of 32x32 =
  65536 bytes each), `enemy1a` (bird) and `enemy1b` (hopper): 19 frames of 32x32 each, `bonuses` (15+
  types x 8 frames of 16x16 = 31488 bytes; type 0 = ice ball, 4 = steam puff, 5 = sweat drop, 14 = sparkle,
  15 = open-mouth head), `statgfx` (17 glyphs of 8x8: digits 0-9 then energy, lives, timer, score, level,
  super jump, rapid fire icons), `brdg1` (one 8x8 rope segment), full-screen 320x200 pictures: title card
  (see section 4), main menu, options, info, level intro, and a "LEVEL COMPLETE" fallback.
- Maps: `tmap1` 8192 bytes, `smap1` 2048 bytes, `emap1` 200 x 13 bytes, `bmap1` 400 x 5 bytes, `ldata`
  10 x 468 bytes.
- 14 short 8-bit WAV effects + the intro jingle (or synthesise them).
Colour layout constraints (the engine depends on them): terrain pixels **must** use indices >= 128;
non-solid scenery < 128; sky gradient 64-95; cycling colours 96-103; white = 1; clouds use private entries
55-57 after the remap.

## 15. Acceptance tests (the result must pass all of these)

1. Opens with no console errors; "tap to start" -> intro card tunes in through grey then colour -> menu.
2. Level 1 starts at the correct position; the dragon stands still when no key is pressed, walks 1, then 2,
   then 5 px/frame as speed builds, and stops with deceleration.
3. A standing jump rises about 100 px; a flap near the apex adds roughly 20 px; air steering is gentle
   (pressing the opposite direction for half a second changes velocity by roughly 1.3 px/frame).
4. Ice balls freeze a walker (frames 32-47), the freeze times out after 255 ticks with a -100 score
   penalty, jumping on a frozen enemy kills it for +100.
5. Spikes/acid hurt for 25, enemies for 5, springs launch, restart flags and the end flag work with ice balls.
6. Night: sky fades, clouds fade to dark slate, rain falls, lightning flashes with a rumble a moment later.
7. 70 ticks/s on a 60 Hz display; fades take ~0.9 s; scaling and CRT modes switch live and persist.
8. Music: random first song; 60 s later a fade to a different song; Music off silences it instantly.
9. A bot reaches the end flag of the extended level; the win screen tallies the time bonus; game over works
   when lives run out.
10. Works with touch controls in portrait and landscape and with the keyboard; no page scrolling or zoom.

When done, give a short note on file layout, how to run it (any static server), and anything you could not
verify.
