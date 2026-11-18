# AGENTS.md

Single-file Arduino sketch (`maqueta_interactiva.ino`) for an interactive physical model of Chile's
electric grid (generation → transmission → distribution). No package manager, tests, linter, or CI:
the only real verification is that the sketch compiles.

## Build / verify

- Dependency: **FastLED** (not vendored). Install with `arduino-cli lib install FastLED`.
- Arduino requires the main `.ino` to sit in a folder of the same name. This repo is
  `maqueta-energia/maqueta_interactiva.ino`, so IDE and `arduino-cli` refuse it as-is. Build from a
  directory named `maqueta_interactiva/` (copy or link the file there), then
  `arduino-cli compile --fqbn <board-fqbn> maqueta_interactiva`.
- No board/FQBN is recorded anywhere; ask before assuming one. Pin usage implies a 5V AVR-class board.
- There are no test, lint or format commands. Don't invent them.

## Hardware contract (changing this breaks the physical model)

- 197 × WS2812B on data pin 6, `COLOR_ORDER GRB`, brightness 96.
- Buttons on pins 3/4/5 with `INPUT_PULLUP` → pressed reads `LOW`.
  Button1 = generation (red), Button2 = transmission (green), Button3 = distribution (blue).
- Relay on pin 2 is **active-low** and driven inverted from Button1: `LOW` while generation is lit,
  `HIGH` otherwise (windmill).
- LED indices are physical positions on the printed model. Never renumber or reorder the segment
  arrays (`HIDROLINERA`, `EOLICO`, `GNL`, …) without the layout.

## Code quirks that look like bugs but are intentional

- Segment arrays are `{startPixel, pixelCount}` for solid areas and `{startPixel, pixelCount, cursor}`
  for animated paths. The third element is **mutable animation state**, so those arrays cannot be
  `const` and must not be shared between calls.
- `drawCaminos*(int tramo[2])` really receives 3-element arrays and uses `tramo[2]`; C ignores the
  `[2]`. The `*I` variants decrement and reuse `tramo[1]` (the length) as the cursor — that is the
  reverse animation, not an off-by-one.
- `clean()` erases only one random pixel per call; the repeated `clean()` calls in `loop()` make
  segments fade out statistically. Don't "fix" it to clear whole ranges.
- `loop()` is non-blocking on purpose: animations advance on `millis()` (`intervalo = 201`) with one
  `FastLED.show()` per iteration. Inserting `delay()` (including the commented-out `FastLED.delay()`)
  changes animation speed.
- Per mode the pattern is: draw regions while the button is held, erase them in the `else` branch,
  then the timed block advances one path step.

## Style

- Comments, identifiers and commit messages are Spanish; keep it that way
  (`GENERACION`/`TRANSMISION`/`DISTRIBUCION`, `Boton1..3`).
