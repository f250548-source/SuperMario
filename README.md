# Super Mario Engine (EL2003 COAL semester project)

16-bit x86 assembly (NASM, `.COM`), VGA Mode 13h, runs in DOSBox.

## Members
- Member 1: engine (skeleton, video, frame buffer, map, sprites, camera, text, screens, keyboard and timer interrupts)
- Member 2: game logic (`GameLoop`, physics, enemies, states) in `game.inc`

## Modules (files)
| File | Contents |
|---|---|
| `mario.asm` | org, includes, `main`, exit path that always restores vectors |
| `constants.inc` | all EQUs (sizes, tile codes, scan codes, colours, flags, states) |
| `data.inc` | shared variables, interrupt bookkeeping, screen strings |
| `video.inc` | SetMode13, SetTextMode, PutPixel, FillRect, ColorTestScreen |
| `buffer.inc` | AllocFrameBuffer, ClearBuffer, PresentBuffer |
| `mapdata.inc`, `map.inc` | level as text, GetTile, SetTile, DrawTile, DrawMap, ResetMap |
| `spritedata.inc`, `sprites.inc` | original artwork, DrawSprite, DrawSpriteCamera |
| `camera.inc` | UpdateCamera, WorldToScreen, IsOnScreen |
| `fontdata.inc`, `text.inc` | 8x8 font, DrawChar, DrawString, DrawNumber, DrawHeart |
| `screens.inc` | DrawTitle, DrawPause, DrawLifeLost, DrawLevelComplete, DrawGameOver, DrawHUDFrame, DrawBorder |
| `keyboard.inc`, `timer.inc` | INT 09h and INT 08h handlers, install/restore, WaitTick |
| `gamestub.inc` | temporary demo `GameLoop`; replaced by Member 2's `game.inc` |

## Build and run
```
build.bat                (runs: nasm -f bin mario.asm -o MARIO.COM)
```
In DOSBox: `mount c <project folder>`, `c:`, `MARIO`.

## Controls
Left/Right arrows move, Up/Space jump, L-Shift run, Down crouch/pipe, P pause, Enter confirm, Esc quit.
(The demo stub only uses Left, Right and Esc.)

## Memory choice (required statement)
The frame buffer (64000 bytes) lives in a **second segment**. The program first shrinks its own DOS block with
`INT 21h AH=4Ah` (keeping 1000h paragraphs = 64 KB), then allocates **4000 paragraphs** with `INT 21h AH=48h`.
The returned segment is stored in `buffer_seg` (and `draw_seg`). `ClearBuffer` uses `REP STOSW`,
`PresentBuffer` uses `REP MOVSW`, `ResetMap` uses `REP MOVSB`.

## Game states
0 title, 1 playing, 2 paused, 3 life lost, 4 level complete, 5 game over, 6 exit (`STATE_*` in constants.inc).

## Deviations and design notes
- Colour 0 is transparent for sprites; `PutPixel` and `FillRect` still draw it (needed for black screens).
  A real black for artwork is palette index 16 (`COL_OUTLINE`).
- The font has capitals, digits, `: - ! >` and space. Lowercase letters are shown as capitals.
- `ClearBuffer` paints sky blue; `DrawHUDFrame` paints the black top 16 px and bottom 8 px.
- Arrow keys send E0h first: the handler ignores that byte, so keypad 4/6/8/2 also act as arrows.
- The keyboard handler does not chain to the BIOS handler (the BIOS key buffer is not used).
- Extra helpers beyond the contract: DrawBorder, WorldToScreen, IsOnScreen.

## Bonus
- Level is drawn from editable ASCII art (`mapdata.inc`), sprites from ASCII art (`spritedata.inc`).
- Second-accurate timer: a fractional accumulator turns 18.2065 ticks/s into a true one-second flag.

## Limitations
- Game speed equals the BIOS timer rate (about 18.2 frames per second).
- Ctrl+Break / Ctrl+C is not handled specially; always leave with Esc so vectors are restored.
- Only palette indices 0-31 of the default VGA palette are used.
