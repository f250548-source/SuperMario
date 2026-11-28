# Game logic (Member 2): Modules 9–15

## Files
| File | Contents |
|---|---|
| `game.inc` | Game EQUs, `ADD_FIX` macro, `GameLoop`, state handlers and jump table, `ResetLevel`, `NewGame`, win/death sequences, world drawing |
| `hero.inc` | `ReadKeys`/`KeyDown`/`KeyHit`, `UpdateHero` (physics), `MoveHeroX`/`MoveHeroY` (tile collision), `IsSolidAt`, `SolidPair`, `SolidColumn`, `Overlap`, `UpdateHeroCamera` |
| `enemies.inc` | Entity table update, `MoveEntity`, Goomba/Koopa/shell rules, `ResolveHeroEntities`/`HeroTouches` (stomp or hurt), `BounceHero` |
| `items.inc` | `TriggerBlock`, `SpawnMushroom`, map coins, flag-pole test, coin pop-up |
| `score.inc` | `AddScore` (32-bit ADD/ADC, clamped to 999,999), `AddCoin`, `AddLife`, `StompPoints`, `HurtHero`, `GrowHero`/`ShrinkHero`, `UpdateClock`, `DrawNumber32`, `DrawHUD` |
| `gamedata.inc` | Every Member 2 variable and table |

`mario.asm` includes `game.inc` in place of `gamestub.inc`; that is the only change to Member 1's files. Build with `build.bat` as before.

## Conventions (same as Member 1)
- Parameters are pushed left to right, read from `[bp+4]` upwards, and the callee removes them (`ret n`).
- Every procedure keeps all registers except the documented output (always `AX`).
- Positions use a whole-pixel word plus a fraction byte (8.8 with a 16-bit whole part, because the level is 960 px wide). Velocities are signed 8.8 words. `ADD_FIX` adds one to the other with `ADD`, `CBW` and `ADC`.

## State machine
`game_state` indexes `state_table` (one word per state). `GameLoop` calls `WaitTick`, runs the handler for the current state once, and returns when the state is EXIT. `main` then restores INT 08h/09h and text mode, so there is only one exit path.

| # | State | Behaviour |
|---|---|---|
| 0 | TITLE | Up/Down choose; Enter on START → `NewGame` → PLAYING; Enter on EXIT or Esc → EXIT |
| 1 | PLAYING | Runs the 10-step frame of Section 5.13. P → PAUSED, Esc → TITLE |
| 2 | PAUSED | Member 1's pause box is drawn once over the frozen picture. P → PLAYING, Esc → TITLE |
| 7 | DYING | Hop of about 48 px (none for a pit), then a fall with no collision for about 1.5 s. Then lives − 1 → LIFE LOST or GAME OVER |
| 3 | LIFE LOST | Screen shown for about 2 s, then `ResetLevel` (score and coins kept) → PLAYING |
| 8 | WIN SEQUENCE | Flag points are added, Mario slides down the pole (the flag goes with him), walks for about 1 s and disappears. Time × 50 is added once → LEVEL COMPLETE |
| 4 | LEVEL COMPLETE | Enter → TITLE |
| 5 | GAME OVER | Enter → TITLE |
| 6 | EXIT | `GameLoop` returns |

The clock and `one_second_flag` are frozen in every state except PLAYING. Menu keys and the jump use edge detection (`KeyHit`), so holding a key acts only once.

## Entity record (`ent_table`, 10 × 16 bytes)
`E_TYPE`(b) `E_STATE`(b) `E_X`(w) `E_XF`(b) `E_DIR`(b, +1/−1) `E_Y`(w) `E_YF`(b) `E_TIMER`(b) `E_VY`(w) `E_SPEED`(w) `E_GRACE`(b) `E_GROUND`(b).
Enemies start from `enemy_start` (type, column, row): 4 Goombas and 2 Koopas. Mushrooms use free records.

## Tuning (about 18.2 frames/s, the BIOS timer rate)
Walk 2.0, run 3.0, acceleration 0.1875, friction 0.125, braking 0.375, gravity 0.75, jump 10.0 (about 61 px, roughly 3.8 tiles), cut-off 3.0, maximum fall 8 px/frame (less than one tile, so Mario cannot tunnel through tiles). At walking speed Mario can cross the 3-tile pit, clear the 3-tile pipe, hit a `?` block from the ground and climb the four stairs.

## Decisions on unclear rules
- Every stomp from the 11th onwards gives +1 life (still capped at 99); the stomp table is never read past entry 10.
- Lives are capped at `LIVES_LIMIT` = 99 (Section 5.4). `consts.inc` contains `MAX_LIVES equ 9` and `COIN_SCORE equ 100`, which are placeholders and are not used; coins are worth `COIN_POINTS` = 200.
- `DrawLevelComplete` and `DrawGameOver` print 16-bit, 5-digit numbers. The score fields are therefore printed again on top as 6 digits from the 32-bit score (`DrawNumber32`).
- An enemy is removed once its x is more than 32 px left of `camera_x`.

## HUD cache (speed)
Drawing HUD text pixel by pixel took about a third of a frame at DOSBox's default 3000 cycles. The HUD is therefore drawn into `hud_cache` with Member 1's own routines, by pointing `draw_seg` at the paragraph-aligned cache for a moment. That redraw happens only when the score, coins, time, lives or HP change. Every frame the 16 rows are copied into the frame buffer with `REP MOVSW` (about 0.01 tick).

Measured at 3000 cycles: `DrawMap` takes about 0.75 tick per frame, and `ClearBuffer` + `PresentBuffer` take about 0.38 tick. Together that is more than one tick, so reaching 18 fps at default cycles needs a faster `DrawMap`/`DrawSprite` (Member 1). It runs at full speed with higher cycles.

## Bonus features
- Walking animation (run and stand frames alternate).
- A coin pops out of `?` blocks.
- The flag slides down with Mario.
