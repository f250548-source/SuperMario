; =====================================================================
; gamedata.inc - Member 2 variables and tables, grouped by purpose.
; Shared variables owned by Member 1 (camera_x, key_table, tick_count,
; one_second_flag, blink_toggle, level_work ...) live in data.inc.
; =====================================================================

; ---- state machine ----
game_state          dw STATE_TITLE
title_sel           dw 0                     ; 0 START GAME, 1 EXIT
state_timer         dw 0                     ; frames spent in a timed state
pause_drawn         db 0
win_phase           db 0                     ; 0 slide down, 1 walk away
death_pending       db 0                     ; DEATH_HIT set by HurtHero

; jump table, index = state number (STATE_* in consts.inc and game.inc)
state_table         dw StateTitle            ; 0 TITLE
                    dw StatePlaying          ; 1 PLAYING
                    dw StatePaused           ; 2 PAUSED
                    dw StateLifeLost         ; 3 LIFE LOST
                    dw StateLevelDone        ; 4 LEVEL COMPLETE
                    dw StateGameOver         ; 5 GAME OVER
                    dw StateExit             ; 6 EXIT
                    dw StateDying            ; 7 DYING
                    dw StateWin              ; 8 WIN SEQUENCE

; ---- keys: snapshot of key_table taken once per frame ----
keys_cur            times KEY_TABLE_SIZE db 0
keys_old            times KEY_TABLE_SIZE db 0

; ---- hero (Module 9): position = whole word + fraction byte ----
hero_x              dw HERO_START_X          ; world x of the sprite
hero_xf             db 0
hero_y              dw HERO_STAND_ROW * TILE_SIZE  ; top of the sprite
hero_yf             db 0
hero_vx             dw 0                     ; signed 8.8 px/frame
hero_vy             dw 0                     ; signed 8.8, + = down
hero_h              dw SPRITE_H              ; 16 Small, 32 Super
hero_prev_feet      dw 0                     ; feet y last frame (stomp test)
hero_on_ground      db 1
hero_form           db FORM_SMALL
hero_hp             db 1                     ; always 1 or 2
hero_invuln         db 0                     ; frames left
hero_stomp_chain    db 0                     ; stomps since last landing
hero_facing         db FACE_RIGHT
hero_anim           db 0                     ; walking animation counter
hero_visible        db 1

; pose table (stand, run, jump) for each form
hero_small_sprites  dw spr_mario_stand, spr_mario_run, spr_mario_jump
hero_super_sprites  dw spr_super_stand, spr_super_run, spr_super_jump

; ---- score, coins, lives, clock (Module 13) ----
score_lo            dw 0                     ; 32-bit score, low word
score_hi            dw 0                     ;               high word
coins               dw 0
lives               dw START_LIVES
clock               dw LEVEL_TIME_SECONDS

; stomp chain: points for stomp 1..10 (11th and later = +1 life)
stomp_points        dw 100, 200, 400, 500, 800, 1000, 2000, 4000, 5000, 8000

; flag pole points by map row of Mario's feet (rows 0..8)
flag_points         dw 5000, 5000, 5000, 5000, 2000, 800, 400, 400, 100

; level complete summary
ld_score_lo         dw 0                     ; score before the time bonus
ld_score_hi         dw 0
ld_time             dw 0
ld_bonus            dw 0

; ---- items (Module 12) ----
pop_x               dw 0                     ; coin popping out of a ? block
pop_y               dw 0
pop_timer           db 0
flag_y              dw POLE_FLAG_TOP

; ---- enemies (Module 11) ----
; start list: type, column, row (enemies stand on row 9), 0 = end.
; G = Goomba, K = Koopa. Columns refer to the map in mapdata.inc:
;   G12, G15  a pair walking towards the start: stomp one, bounce onto
;             the other in the same jump (stomp chain 100 + 200), or
;             walk into one (hurt / death)
;   K30  turns at the pit edge (Koopas turn at platform edges)
;   G32  next to that Koopa: kick its shell into it (+500)
;   K45  after the second pit, under the bricks
;   G48  meets K45: enemies turn at each other
enemy_start         db ET_GOOMBA, 12, HERO_STAND_ROW
                    db ET_GOOMBA, 15, HERO_STAND_ROW
                    db ET_KOOPA,  30, HERO_STAND_ROW
                    db ET_GOOMBA, 32, HERO_STAND_ROW
                    db ET_KOOPA,  45, HERO_STAND_ROW
                    db ET_GOOMBA, 48, HERO_STAND_ROW
                    db 0

; all moving entities: enemies, shells, mushrooms (ENT_SIZE each)
ent_table           times MAX_ENTS * ENT_SIZE db 0

; ---- HUD cache: the HUD strip is drawn here only when a value changes
; and copied into the frame buffer every frame (score.inc DrawHUD) ----
hud_last_score_lo   dw 0FFFFh                ; values the cache shows,
hud_last_score_hi   dw 0FFFFh                ; FFFFh = never drawn
hud_last_coins      dw 0FFFFh
hud_last_clock      dw 0FFFFh
hud_last_lives      dw 0FFFFh
hud_last_hp         db 0FFh
            align   16                       ; paragraph boundary, so a
hud_cache           times SCREEN_W * HUD_CACHE_ROWS db 0  ; segment can start here

; ---- HUD strings (font has capitals, digits, ' !-:>') ----
str_hud_world       db 'WORLD', 0
str_hud_level       db '1-1', 0
str_hud_time        db 'TIME', 0
str_hud_lives       db 'LIVES', 0
