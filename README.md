# Super Mario Engine (EL2003 COAL semester project)

16-bit x86 assembly (NASM, `.COM`), VGA Mode 13h, runs in DOSBox.

## Members
| Name | Roll number | Modules |
|---|---|---|
| Haziq Salman | 25F-XXXX | 1–8: program skeleton and build, video mode and colours, level map and tiles, sprites, frame buffer, camera and scrolling, keyboard interrupt, timer and game clock; text, font and screens |
| Aayan Abdullah | 25F-XXXX | 9–15: player physics, collision, enemies and shells, blocks/coins/mushrooms, score/health/lives/HUD, game states and screens flow, win/lose/restart |

## Modules (files)
| File | Contents |
|---|---|
| `mario.asm` | org, includes, `main`, exit path that always restores vectors |
| `consts.inc` (copy: `constants.inc`) | all engine EQUs (sizes, tile codes, scan codes, colours, flags, states) |
| `data.inc` | shared variables, interrupt bookkeeping, screen strings |
| `video.inc` | SetMode13, SetTextMode, PutPixel, FillRect, ColorTestScreen |
| `buffer.inc` | AllocFrameBuffer, ClearBuffer, PresentBuffer |
| `mapdata.inc`, `map.inc` | level as text, GetTile, SetTile, DrawTile, DrawMap, ResetMap |
| `sprdata.inc` (ASCII-art source: `spritedata.inc`), `sprites.inc` | original artwork, DrawSprite, DrawSpriteCamera |
| `camera.inc` | UpdateCamera, WorldToScreen, IsOnScreen |
| `fontdata.inc`, `text.inc` | 8x8 font, DrawChar, DrawString, DrawNumber, DrawHeart |
| `screens.inc` | DrawTitle, DrawPause, DrawLifeLost, DrawLevelComplete, DrawGameOver, DrawHUDFrame, DrawBorder |
| `keyboard.inc`, `timer.inc` | INT 09h and INT 08h handlers, install/restore, WaitTick |
| `game.inc` | game EQUs, `GameLoop`, state jump table and state handlers, debug keys, death and win sequences, ResetLevel, NewGame, world drawing |
| `hero.inc` | key snapshot (KeyDown/KeyHit), Mario physics, tile collision, Overlap |
| `enemies.inc` | entity table update, Goomba/Koopa/shell rules, stomp or hurt |
| `items.inc` | TriggerBlock, coins, mushroom spawning, flag pole test |
| `score.inc` | AddScore (32-bit), AddCoin, AddLife, StompPoints, HurtHero, clock, DrawNumber32, cached HUD |
| `gamedata.inc` | game variables, enemy start list, entity table, score tables |
| `gamestub.inc` | early demo `GameLoop` used while the engine was built (no longer included) |

`GAMEPLAY.md` has more detail on the game logic.

## Build and run
```
build.bat                (runs: nasm -f bin mario.asm -o MARIO.COM -l mario.lst)
```
In DOSBox:
```
mount c "C:\path\to\SuperMario"
c:
build
MARIO
```
DOSBox's default speed (cycles=auto, 3000 cycles in real mode) is too slow for a steady 18 fps; press
Ctrl+F12 a few times, or set `cycles=max` (or `cycles=20000`) in the DOSBox options file.

## Controls (Appendix A.3)
| Key | Action |
|---|---|
| Left / Right arrow | Move left / right |
| Up / Down arrow | Move the menu highlight (in the game, Up also jumps) |
| Space | Jump (hold = higher jump, higher stomp bounce) |
| Left Shift | Run |
| P | Pause / resume |
| Enter | Select in menus, continue on end screens |
| Esc | Back to the title; at the title: exit to DOS |

### Debug keys (allowed by Section 7.7)
| Key | Action (while playing) |
|---|---|
| T | Set the clock to 5 seconds (time-out death test) |
| C | Set the coins to 99 (the next coin gives the 100-coin life) |

## Memory choice (required statement)
The frame buffer (64000 bytes) lives in a **second segment**. The program first shrinks its own DOS block with
`INT 21h AH=4Ah` (keeping 1000h paragraphs = 64 KB), then allocates **4000 paragraphs** with `INT 21h AH=48h`.
The returned segment is stored in `buffer_seg` (and `draw_seg`). `ClearBuffer` uses `REP STOSW`,
`PresentBuffer` uses `REP MOVSW`, `ResetMap` uses `REP MOVSB`.
The two copies of the level (`level_orig`, never changed, and `level_work`) and all game data live in the
program segment.

## Game states
`game_state` indexes a jump table (`state_table`); `GameLoop` waits for a timer tick and runs the handler of
the current state once per frame.

| # | State | Description |
|---|---|---|
| 0 | TITLE | Title screen. Up/Down move the marker, Enter on START GAME starts a new game, Enter on EXIT or Esc leaves. |
| 1 | PLAYING | One frame in the order of Section 5.13. P pauses, Esc returns to the title, death goes to DYING, the flag goes to WIN SEQUENCE. |
| 2 | PAUSED | The last picture stays with a PAUSED box; nothing moves and the clock stops. P resumes, Esc returns to the title. |
| 7 | DYING | Enemies, clock and controls are frozen. Mario hops up about 48 px (not after a pit) and falls off the screen (about 1.5 s); then one life is taken. |
| 3 | LIFE LOST | World name, Mario and the lives left for about 2 s, then the level restarts (score and coins kept). |
| 8 | WIN SEQUENCE | Flag points by height, Mario slides down the pole with the flag, walks off and disappears; the time bonus is added once. |
| 4 | LEVEL COMPLETE | Score, coins, time left, bonus and final score. Enter returns to the title. |
| 5 | GAME OVER | Final score and coins. Enter returns to the title. |
| 6 | EXIT | `GameLoop` returns; `main` restores INT 08h/09h, switches to text mode and exits to DOS. |

## Deviations and design notes
- Colour 0 is transparent for sprites; `PutPixel` and `FillRect` still draw it (needed for black screens).
  A real black for artwork is palette index 16 (`COL_OUTLINE`).
- The font has capitals, digits, `: - ! >` and space. Lowercase letters are shown as capitals.
- `ClearBuffer` paints sky blue; the top 16 px (HUD) and bottom 8 px are black.
- Arrow keys send E0h first: the handler ignores that byte, so keypad 4/6/8/2 also act as arrows.
- The keyboard handler does not chain to the BIOS handler (the BIOS key buffer is not used).
- Extra helpers beyond the contract: DrawBorder, WorldToScreen, IsOnScreen.
- **Frame rate and physics:** the game runs at the BIOS timer rate, 18.2 frames/s, not 30. As Appendix A.5
  allows, all A.5 values are scaled so movement per second is the same: speeds x 30/18.2 (walk 2.47, run 4.12,
  jump 9.25, cut-off 3.3, max fall 6.6, stomp bounce 4.94, Goomba 0.82, Koopa 1.24, shell 4.94, mushroom 1.65
  px/frame) and accelerations x (30/18.2)^2 (acceleration 0.34, friction 0.17, gravity 0.68). The jump is
  about 58 px high (3.6 tiles).
- Positions are 8.8 fixed point with a 16-bit whole part (a word of whole pixels plus a fraction byte),
  because the level is 960 px wide.
- Up also jumps while playing (an extra to Appendix A.3).
- From the 11th stomp in one chain, every further stomp gives +1 life (lives still capped at 99).
- The score is 32-bit; the Level Complete and Game Over screens print it with 6 digits.
- Enemies are removed when more than 32 px left of the camera; mushrooms when they leave the screen on the left.

## Bonus
- Walking animation (run and stand pictures alternate while moving).
- A coin pops out of a "?" coin block; the flag slides down the pole with Mario.
- Level is drawn from editable ASCII art (`mapdata.inc`), sprites from ASCII art (`spritedata.inc`).
- Second-accurate timer: a fractional accumulator turns 18.2065 ticks/s into a true one-second flag.

## Limitations
- At DOSBox's default 3000 cycles a frame takes about 1.2 timer ticks (map drawing about 0.75, buffer
  clear and copy about 0.38), so the game runs at about 15 frames/s and moves slower. With more cycles it
  runs at a steady 18.2 frames/s.
- Game speed equals the BIOS timer rate (about 18.2 frames per second); the timer is not reprogrammed.
- Ctrl+Break / Ctrl+C is not handled specially; always leave with Esc so vectors are restored.
- Only palette indices 0-31 of the default VGA palette are used.
- No sound.
