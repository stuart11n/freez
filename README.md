# FREEZ

Browser port of FREEZ (c) Digital Synapse 1996, a DOS platformer originally written in Borland C with inline x86 asm.
This is a direct JavaScript translation of the source (no emulator): VGA mode 13h is a canvas, the DAC palette and
retrace waits are emulated at 70Hz, and the asm routines are rewritten function by function.

## Run

Serve this folder with any static HTTP server and open `index.html`:

    python -m http.server

## Controls

Arrows move, Ctrl / Z / Up jump, Alt / X / Shift fire, Enter / Space select, P pause, Esc quit to menu.
F1 sound, F2 weather, F3 parallax. Debug keys from the original: L end level, T timer, A steam, S die.

## Notes

- Only level 1 data exists in the shipped game; finishing it returns to the menu.
- Sound is hooked up to the original WAVs via Web Audio (the shipped DOS build had it compiled out).

## Original source

The original Borland C / TASM source is in `original/` (CP437 encoded; `FREEZE.CPP` is the entry point).

## Mobile version

`mobile/` is a separate build: the frame is upscaled to 640x480 (Scale2x, stretched to 4:3), it has on-screen touch
controls (d-pad, fire, jump, OK / pause / menu), works in portrait and landscape, and has an optional CRT scanline
effect (CRT button, or F6 on a keyboard). Serve the repo and open `mobile/index.html`.
