# Debugger test plan (one short test per procedure)

Find every address in `mario.lst` (build.bat creates it; addresses there start at 0, add 100h).
Use a debugger that can run a `.COM` (DOSBox debugger, or DEBUG: `debug MARIO.COM`).
To call a procedure with stack parameters, assemble a tiny caller at `CS:0200` with the `a` command, e.g.
`push 20 / push 10 / push 4 / call PutPixel / int 3`, then `g 200`.
For drawing tests first run SetMode13 (`AX=0013h`, `int 10h`) and set `draw_seg` to A000h so you see the result
in video memory (`d A000:offset`).

| Procedure | Set | Check |
|---|---|---|
| SetMode13 / SetTextMode | run it | screen changes mode; AX unchanged |
| PutPixel | draw_seg=A000; push 20,10,4 | `A000:0C94` = 04 (10*320+20 = 3220 = 0C94h); then push -1,10,4 and 400,10,4: nothing changes |
| FillRect | push 316,196,10,10,7 | only the 4x4 corner (rows 196-199, cols 316-319) = 07; row 195 untouched |
| ColorTestScreen | run | 16x16 grid, cell n has colour n; row 0 is colours 0-15 |
| AllocFrameBuffer | run | CF=0; `buffer_seg` = `draw_seg` = new segment (e.g. 3000h) |
| ClearBuffer | draw_seg unchanged | `d buffer_seg:0` and `:F9FE` show 09 09 ...; byte at `:FA00` untouched |
| PresentBuffer | fill buffer with 05 by hand | A000:0 and A000:F9FF = 05; DS value after = before |
| GetTile | ResetMap first; push 144,144 (col 9,row 9) then 0,-1 and 960,0 | AL = tile at level_work[9*60+9]; out of range gives AL = 0 |
| SetTile | push 5,3,9 then GetTile(80,48) | AL = 9; push 60,3,9 changes nothing |
| DrawTile | push 100,100,2 (brick) | 16x16 brick appears at (100,100); code 0 draws nothing |
| DrawMap | set camera_x = 20 | first column starts at x = -4; 21 columns drawn; rows 1-11 only |
| ResetMap | SetTile something, run it | level_work equals level_orig (`c` compare 720 bytes) |
| DrawSprite | push -8,50,spr_coin,0 / 312,50,..,0 / 100,-8,.. / 100,196,.. | clipped on every edge, no pixel appears on the opposite side; flags=1 mirrors; flags=4 draws 32 rows |
| DrawSpriteCamera | camera_x=100; push 110,20,spr_coin,0 | drawn at x=10; world x 84 (=camera-16) draws nothing |
| UpdateCamera | camera_x=0; push 200 then 100 then 900 | camera 40 then 40 then 640; AX = 200, then 100, then 900 |
| UpdateCamera left wall | camera_x=300; push 250 | AX = 300 (pushed back), camera stays 300 |
| DrawChar / DrawString | push 10,10,'A',15 / push 10,30,str_title,14 | 8x8 glyph; string 8 px per char; lowercase shows as capitals |
| DrawNumber | push 10,10,42,5,15 | shows 00042 |
| DrawHeart | push 8,4,1 then 8,4,0 | full red heart, then outline heart |
| Screens | call DrawTitle(0/1), DrawPause, DrawLifeLost(3), DrawLevelComplete(1200,7,85,850,2050), DrawGameOver(1200,7) | text and numbers as in the specification |
| KbdISR | `e key_table+4D 00`, press Right with the handler installed | `d key_table+4D` = 01 while held, 00 after release; E0 byte changes nothing |
| TimerISR / WaitTick | install, `d tick_count` twice | tick_count grows ~18 per second; one_second_flag becomes 1 about once a second |
| RestoreKbd / RestoreTimer | run main, quit with Esc | `INT 08h` and `INT 09h` vectors in the IVT (0000:0020, 0000:0024) equal their original values |

Before the interrupts exist, fill `key_table` by hand: `e <key_table>+4D 01` means Right is held.
