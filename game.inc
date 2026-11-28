; =====================================================================
; game.inc - Member 2: game logic. Replaces gamestub.inc (same entry:
; GameLoop, returns with ret so main can restore everything and exit).
;   game.inc     Module 14/15: state machine, frame order, resets, draw
;   hero.inc     Module 9/10 : keys, physics, tile collision, overlap
;   enemies.inc  Module 11   : Goombas, Koopas, shells, stomp or hurt
;   items.inc    Module 12   : blocks, coins, mushrooms
;   score.inc    Module 13   : score, coins, lives, health, HUD
;   gamedata.inc             : Member 2 variables and tables
; Only EQUs that Member 1's consts.inc does not have are defined here.
; =====================================================================

; ---------------- Extra game states (0-6 are in consts.inc) ----------------
STATE_DYING         equ 7
STATE_WIN           equ 8                    ; win sequence (slide + walk)

; ---------------- Game limits (Section 5) ----------------
LIVES_LIMIT         equ 99                   ; lives never exceed 99
COINS_FOR_LIFE      equ 100
COIN_POINTS         equ 200                  ; map coin or ? coin
BRICK_POINTS        equ 50
MUSHROOM_POINTS     equ 1000
KICK_POINTS         equ 400                  ; kick a stationary shell
SHELL_KILL_POINTS   equ 500                  ; enemy hit by a sliding shell
TIME_BONUS_PER_SEC  equ 50
SCORE_MAX_HI        equ 000Fh                ; 999,999 = 000F:423Fh
SCORE_MAX_LO        equ 423Fh
STOMP_TABLE_LEN     equ 10                   ; stomps 1..10 give points

; ---------------- Hero physics, 8.8 fixed point, per frame ----------------
; The BIOS timer gives ~18.2 frames/s, so values are tuned for that rate:
; full jump ~61 px (3.8 tiles), clears a 3-tile pipe and a 3-tile pit.
HERO_ACCEL          equ 0030h                ; 0.1875 px/frame^2
HERO_FRICTION       equ 0020h                ; 0.125  (slides a little)
HERO_BRAKE          equ 0060h                ; 0.375  (opposite key)
HERO_WALK_MAX       equ 0200h                ; 2.0 px/frame
HERO_RUN_MAX        equ 0300h                ; 3.0 px/frame (Left Shift)
HERO_GRAVITY        equ 00C0h                ; 0.75 px/frame^2
HERO_JUMP_VEL       equ 0A00h                ; 10.0 px/frame upwards
HERO_JUMP_CUT       equ 0300h                ; 3.0 upward cut-off
HERO_MAX_FALL       equ 0800h                ; 8 px/frame (< one tile)
STOMP_BOUNCE        equ 0500h                ; bounce after a stomp
STOMP_BOUNCE_HIGH   equ 0800h                ; ... with the jump key held
DEATH_HOP_VEL       equ 0900h                ; death hop, about 48 px

; ---------------- Hero geometry ----------------
HITBOX_LEFT         equ 2                    ; 12 px wide hitbox, centred
HITBOX_RIGHT        equ 13                   ; in the 16 px sprite
HITBOX_W            equ 12
HERO_CENTER_X       equ 8                    ; column used for head hits
HERO_START_X        equ 32
HERO_MAX_X          equ LEVEL_WIDTH_PX - SPRITE_W  ; 944: right end
PIT_Y               equ VISIBLE_H            ; top y >= 192: fell in a pit
FORM_SMALL          equ 0
FORM_SUPER          equ 1
FACE_RIGHT          equ 0
FACE_LEFT           equ 1
INVULN_FRAMES       equ 36                   ; about 2 s of timer ticks

; ---------------- Entity records (ent_table) ----------------
ENT_SIZE            equ 16                   ; bytes per record
MAX_ENTS            equ 10                   ; 6 enemies + mushrooms
E_TYPE              equ 0                    ; byte ET_*
E_STATE             equ 1                    ; byte ES_*
E_X                 equ 2                    ; word world x, whole px
E_XF                equ 4                    ; byte x fraction
E_DIR               equ 5                    ; byte +1 right / -1 left
E_Y                 equ 6                    ; word y, whole px
E_YF                equ 8                    ; byte y fraction
E_TIMER             equ 9                    ; byte flat timer
E_VY                equ 10                   ; word 8.8 vertical speed
E_SPEED             equ 12                   ; word 8.8 walking speed
E_GRACE             equ 14                   ; byte kick grace frames
E_GROUND            equ 15                   ; byte 1 = on solid ground
ET_NONE             equ 0                    ; free record
ET_GOOMBA           equ 1
ET_KOOPA            equ 2
ET_MUSHROOM         equ 3
ES_DORMANT          equ 0                    ; waits until near the screen
ES_WALK             equ 1
ES_FLAT             equ 2                    ; stomped Goomba
ES_SHELL            equ 3                    ; stationary shell
ES_SLIDE            equ 4                    ; sliding shell
DIR_RIGHT           equ 1
DIR_LEFT            equ 0FFh                 ; -1 as a byte
GOOMBA_SPEED        equ 0080h                ; 0.5 px/frame
KOOPA_SPEED         equ 00C0h                ; 0.75 px/frame
SHELL_SPEED         equ 0300h                ; 3 px/frame
MUSHROOM_SPEED      equ 0100h                ; 1 px/frame
FLAT_FRAMES         equ 18                   ; flat Goomba for about 1 s
KICK_GRACE          equ 8                    ; frames a kicked shell ignores Mario
MAX_ACTIVE_ENEMIES  equ 8
BEHIND_LIMIT        equ 32                   ; removed this far left of the camera

; ---------------- Death, win, timing ----------------
DEATH_HIT           equ 1                    ; enemy or time: hop, then fall
DEATH_PIT           equ 2                    ; pit: just keep falling
DYING_FRAMES        equ 27                   ; about 1.5 s
DYING_STOP_Y        equ 220                  ; stop moving once off screen
LIFE_LOST_FRAMES    equ 36                   ; about 2 s
POLE_COL            equ 56                   ; flag pole column in mapdata
POLE_SNAP_X         equ POLE_COL * TILE_SIZE - 8   ; Mario's x on the pole
POLE_BASE_Y         equ HERO_STAND_ROW * TILE_SIZE ; top of the base block
POLE_FLAG_X         equ POLE_COL * TILE_SIZE - 9   ; flag cloth left of pole
POLE_FLAG_TOP       equ 2 * TILE_SIZE              ; flag starts at row 2
POLE_FLAG_LOW       equ 8 * TILE_SIZE              ; lowest flag y
FLAG_ROWS           equ 9                    ; entries in flag_points
WIN_SLIDE_SPEED     equ 4                    ; px per frame down the pole
WIN_WALK_STEP       equ 2                    ; px per frame to the castle
WIN_WALK_FRAMES     equ 18                   ; about 1 s, then he disappears
WIN_DROP_X          equ (POLE_COL + 1) * TILE_SIZE - HITBOX_LEFT  ; off the base
POP_FRAMES          equ 8                    ; ? coin pop-up
POP_RISE            equ 3

; ---------------- HUD layout ----------------
HUD_SCORE_X         equ 8
HUD_COINS_X         equ 72
HUD_WORLD_X         equ 128
HUD_TIME_X          equ 184
HUD_LIVES_X         equ 224
HUD_HEART_X         equ 272
HUD_HEART_Y         equ 2
HUD_HEART_GAP       equ 18
HUD_COPY_WORDS      equ SCREEN_W * HUD_HEIGHT / 2  ; 16 rows for REP MOVSW
HUD_CACHE_ROWS      equ HUD_HEART_Y + SPRITE_H     ; a heart bitmap reaches
                                                   ; row 17 (blank there)

; ---------------------------------------------------------------------
; ADD_FIX whole, fraction
; Adds the signed 8.8 velocity in AX to a position stored as a whole-
; pixel word plus a fraction byte: fractions are added first, the carry
; goes into the whole part together with the sign-extended high byte.
; Destroys AX.
; ---------------------------------------------------------------------
%macro ADD_FIX 2
            add     byte %2, al          ; fractions
            mov     al, ah               ; whole pixels of the velocity
            cbw                          ; sign-extend (MOV/CBW keep CF)
            adc     word %1, ax          ; whole part + carry
%endmacro

%include "hero.inc"
%include "enemies.inc"
%include "items.inc"
%include "score.inc"

; ---------------------------------------------------------------------
; GameLoop
; Purpose : main loop. Step 1 of every frame is WaitTick (timer paced),
;           then the handler of the current state runs once through the
;           jump table state_table. Returns when the state is EXIT, so
;           main restores the vectors and text mode (one exit path).
; Inputs  : none
; Outputs : returns on exit
; Changes : nothing
; ---------------------------------------------------------------------
GameLoop:
            pusha
            mov     word [game_state], STATE_TITLE
            mov     word [title_sel], 0
.frame:
            call    WaitTick             ; frame step 1
            mov     bx, [game_state]
            shl     bx, 1                ; word table
            call    word [state_table + bx]
            cmp     word [game_state], STATE_EXIT
            jne     .frame
            popa
            ret

; ---------------------------------------------------------------------
; StateTitle
; Purpose : TITLE: Up/Down move the marker, Enter on START begins a new
;           game, Enter on EXIT or Esc leaves.
; Inputs  : none
; Outputs : game_state, title_sel
; Changes : nothing
; ---------------------------------------------------------------------
StateTitle:
            pusha
            call    ReadKeys
            mov     byte [one_second_flag], 0

            push    word KEY_UP
            call    KeyHit
            test    ax, ax
            jz      .not_up
            mov     word [title_sel], 0
.not_up:
            push    word KEY_DOWN
            call    KeyHit
            test    ax, ax
            jz      .not_down
            mov     word [title_sel], 1
.not_down:
            push    word KEY_ESC
            call    KeyHit
            test    ax, ax
            jnz     .exit
            push    word KEY_ENTER
            call    KeyHit
            test    ax, ax
            jz      .draw
            cmp     word [title_sel], 0
            jne     .exit
            call    NewGame              ; START GAME
            mov     word [game_state], STATE_PLAYING
            jmp     .out
.exit:
            mov     word [game_state], STATE_EXIT
            jmp     .out
.draw:
            push    word [title_sel]
            call    DrawTitle
            call    PresentBuffer
.out:
            popa
            ret

; ---------------------------------------------------------------------
; StatePlaying
; Purpose : PLAYING: one frame in the order of Section 5.13
;           (step 1 WaitTick is done by GameLoop).
; Inputs  : none
; Outputs : everything in the game can change
; Changes : nothing
; ---------------------------------------------------------------------
StatePlaying:
            pusha
            call    ReadKeys             ; 2. key-state table
            push    word KEY_P
            call    KeyHit
            test    ax, ax
            jz      .no_pause
            mov     word [game_state], STATE_PAUSED
            mov     byte [pause_drawn], 0
            jmp     .out                 ; freeze: last picture stays
.no_pause:
            push    word KEY_ESC
            call    KeyHit
            test    ax, ax
            jz      .no_escape
            mov     word [game_state], STATE_TITLE
            jmp     .out
.no_escape:
            call    UpdateHero           ; 3. Mario, one axis at a time
            call    UpdateEntities       ; 4. enemies, shells, mushrooms
            call    UpdatePop
            call    ResolveHeroEntities  ; 5. stomp, hurt, kick, collect
            call    CollectCoins
            call    UpdateClock          ; 6. clock and invulnerability
            call    CheckDeathAndFlag    ; 7. may change the state
            call    UpdateHeroCamera     ; 8. camera
            call    DrawWorld            ; 9. draw into the frame buffer
            call    PresentBuffer        ; 10. copy it to the screen
.out:
            popa
            ret

; ---------------------------------------------------------------------
; StatePaused
; Purpose : PAUSED: the last picture stays with Member 1's pause box on
;           top (drawn once). Nothing updates, the clock is frozen.
;           P resumes, Esc goes to the title.
; Inputs  : none
; Outputs : game_state, pause_drawn
; Changes : nothing
; ---------------------------------------------------------------------
StatePaused:
            pusha
            call    ReadKeys
            mov     byte [one_second_flag], 0  ; seconds do not count
            cmp     byte [pause_drawn], 0
            jne     .keys
            call    DrawPause            ; on top of the frozen frame
            call    PresentBuffer
            mov     byte [pause_drawn], 1
.keys:
            push    word KEY_P
            call    KeyHit
            test    ax, ax
            jz      .no_resume
            mov     word [game_state], STATE_PLAYING
            jmp     .out
.no_resume:
            push    word KEY_ESC
            call    KeyHit
            test    ax, ax
            jz      .out
            mov     word [game_state], STATE_TITLE
.out:
            popa
            ret

; ---------------------------------------------------------------------
; StateDying
; Purpose : DYING (Section 5.8): enemies, clock and controls frozen;
;           Mario (jump pose) moves with gravity and no collision until
;           he is below the screen and ~1.5 s have passed. Then lives-1
;           and LIFE LOST, or GAME OVER when no lives are left.
; Inputs  : none
; Outputs : hero_y, hero_vy, lives, game_state
; Changes : nothing
; ---------------------------------------------------------------------
StateDying:
            pusha
            call    ReadKeys             ; keep the snapshot fresh only
            mov     byte [one_second_flag], 0
            inc     word [state_timer]

            cmp     word [hero_y], DYING_STOP_Y
            jge     .moved               ; already off the screen
            mov     ax, [hero_vy]
            add     ax, HERO_GRAVITY
            cmp     ax, HERO_MAX_FALL
            jle     .fall_ok
            mov     ax, HERO_MAX_FALL
.fall_ok:
            mov     [hero_vy], ax
            ADD_FIX [hero_y], [hero_yf]
.moved:
            call    DrawWorld
            call    PresentBuffer

            cmp     word [state_timer], DYING_FRAMES
            jb      .out
            cmp     word [hero_y], SCREEN_H
            jl      .out                 ; still visible
            dec     word [lives]         ; animation over: one life less
            jz      .game_over
            mov     word [game_state], STATE_LIFE_LOST
            mov     word [state_timer], 0
            jmp     .out
.game_over:
            mov     word [game_state], STATE_GAME_OVER
.out:
            popa
            ret

; ---------------------------------------------------------------------
; StateLifeLost
; Purpose : LIFE LOST: Member 1's screen (world, Mario, lives left) for
;           about 2 s, then ResetLevel (score and coins kept) -> PLAYING.
; Inputs  : none
; Outputs : game_state, state_timer
; Changes : nothing
; ---------------------------------------------------------------------
StateLifeLost:
            pusha
            call    ReadKeys
            mov     byte [one_second_flag], 0
            push    word [lives]
            call    DrawLifeLost
            call    PresentBuffer
            inc     word [state_timer]
            cmp     word [state_timer], LIFE_LOST_FRAMES
            jb      .out
            call    ResetLevel
            mov     word [game_state], STATE_PLAYING
.out:
            popa
            ret

; ---------------------------------------------------------------------
; StateWin
; Purpose : WIN SEQUENCE (Section 5.11): clock and controls frozen.
;           Phase 0: Mario slides down the pole (the flag comes with
;           him) to the base block. Phase 1: he walks right for ~1 s,
;           then disappears and the level is finished (FinishLevel).
; Inputs  : none
; Outputs : hero data, flag_y, win_phase, game_state
; Changes : nothing
; ---------------------------------------------------------------------
StateWin:
            pusha
            call    ReadKeys
            mov     byte [one_second_flag], 0
            cmp     byte [win_phase], 0
            jne     .walk

            mov     ax, [hero_y]         ; ---- slide down ----
            add     ax, WIN_SLIDE_SPEED
            mov     bx, POLE_BASE_Y
            sub     bx, [hero_h]         ; standing on the base block
            cmp     ax, bx
            jl      .slide_ok
            mov     ax, bx
            mov     byte [win_phase], 1
            mov     word [state_timer], 0
            mov     byte [hero_on_ground], 1
            mov     word [hero_vx], WIN_WALK_STEP * 256  ; for the run pose
.slide_ok:
            mov     [hero_y], ax
            cmp     ax, [flag_y]         ; the flag follows him down
            jle     .draw
            cmp     ax, POLE_FLAG_LOW
            jle     .flag_ok
            mov     ax, POLE_FLAG_LOW
.flag_ok:
            mov     [flag_y], ax
            jmp     .draw

.walk:
            add     word [hero_x], WIN_WALK_STEP  ; ---- walk away ----
            cmp     word [hero_x], WIN_DROP_X
            jl      .walk_time
            mov     ax, GROUND_ROW * TILE_SIZE    ; stepped off the base block
            sub     ax, [hero_h]
            mov     [hero_y], ax
.walk_time:
            inc     word [state_timer]
            cmp     word [state_timer], WIN_WALK_FRAMES
            jb      .draw
            call    FinishLevel
            jmp     .out
.draw:
            call    DrawWorld
            call    PresentBuffer
.out:
            popa
            ret

; ---------------------------------------------------------------------
; StateLevelDone
; Purpose : LEVEL COMPLETE: summary screen; Enter -> TITLE.
;           Member 1's DrawLevelComplete prints 16-bit numbers with 5
;           digits, so the score and final score are printed again on
;           top with 6 digits from the 32-bit values.
; Inputs  : none
; Outputs : game_state
; Changes : nothing
; ---------------------------------------------------------------------
StateLevelDone:
            pusha
            call    ReadKeys
            mov     byte [one_second_flag], 0

            push    word [ld_score_lo]
            push    word [coins]
            push    word [ld_time]
            push    word [ld_bonus]
            push    word [score_lo]
            call    DrawLevelComplete
            FILLRECT 224, 60, 48, 8, COL_BLACK  ; score, 6 digits
            push    word 224
            push    word 60
            push    word [ld_score_lo]
            push    word [ld_score_hi]
            push    word 6
            push    word COL_WHITE
            call    DrawNumber32
            FILLRECT 224, 132, 48, 8, COL_BLACK ; final score, 6 digits
            push    word 224
            push    word 132
            push    word [score_lo]
            push    word [score_hi]
            push    word 6
            push    word COL_YELLOW
            call    DrawNumber32
            call    PresentBuffer

            push    word KEY_ENTER
            call    KeyHit
            test    ax, ax
            jz      .out
            mov     word [game_state], STATE_TITLE
            mov     word [title_sel], 0
.out:
            popa
            ret

; ---------------------------------------------------------------------
; StateGameOver
; Purpose : GAME OVER: final score (6 digits printed over Member 1's
;           5-digit field) and coins; Enter -> TITLE.
; Inputs  : none
; Outputs : game_state
; Changes : nothing
; ---------------------------------------------------------------------
StateGameOver:
            pusha
            call    ReadKeys
            mov     byte [one_second_flag], 0

            push    word [score_lo]
            push    word [coins]
            call    DrawGameOver
            FILLRECT 200, 92, 48, 8, COL_BLACK
            push    word 200
            push    word 92
            push    word [score_lo]
            push    word [score_hi]
            push    word 6
            push    word COL_WHITE
            call    DrawNumber32
            call    PresentBuffer

            push    word KEY_ENTER
            call    KeyHit
            test    ax, ax
            jz      .out
            mov     word [game_state], STATE_TITLE
            mov     word [title_sel], 0
.out:
            popa
            ret

; ---------------------------------------------------------------------
; StateExit
; Purpose : jump table entry for EXIT. GameLoop leaves before calling
;           it; it only keeps the table complete.
; Inputs  : none
; Outputs : none
; Changes : nothing
; ---------------------------------------------------------------------
StateExit:
            ret

; ---------------------------------------------------------------------
; CheckDeathAndFlag
; Purpose : step 7: death from an enemy (death_pending), a pit
;           (y >= 192, any HP) or the clock at 0 starts DYING; touching
;           the flag pole starts the WIN SEQUENCE (only once, because
;           the state changes).
; Inputs  : none
; Outputs : game_state and the hero for the new state
; Changes : nothing
; ---------------------------------------------------------------------
CheckDeathAndFlag:
            push    ax

            cmp     byte [death_pending], 0
            je      .no_hit
            push    word DEATH_HIT
            call    EnterDying
            jmp     .done
.no_hit:
            cmp     word [hero_y], PIT_Y
            jl      .no_pit
            push    word DEATH_PIT
            call    EnterDying
            jmp     .done
.no_pit:
            cmp     word [clock], 0
            jne     .no_timeout
            push    word DEATH_HIT
            call    EnterDying
            jmp     .done
.no_timeout:
            call    HeroOnPole
            test    ax, ax
            jz      .done
            call    EnterWin
.done:
            pop     ax
            ret

; ---------------------------------------------------------------------
; EnterDying(kind)
; Purpose : switch to DYING. DEATH_HIT: hop up about 48 px first;
;           DEATH_PIT: no hop, Mario keeps falling.
; Inputs  : [bp+4]=DEATH_HIT or DEATH_PIT
; Outputs : game_state, state_timer, hero speed/pose
; Changes : nothing (ret 2)
; ---------------------------------------------------------------------
EnterDying:
            push    bp
            mov     bp, sp

            mov     word [game_state], STATE_DYING
            mov     word [state_timer], 0
            mov     word [hero_vx], 0
            mov     word [hero_vy], 0
            mov     byte [hero_on_ground], 0  ; jump pose
            mov     byte [hero_invuln], 0     ; no blinking
            cmp     word [bp+4], DEATH_PIT
            je      .done
            mov     word [hero_vy], -DEATH_HOP_VEL
.done:
            pop     bp
            ret     2

; ---------------------------------------------------------------------
; EnterWin
; Purpose : first contact with the flag pole: flag points by the map
;           row of Mario's feet (flag_points table), Mario snaps to the
;           pole, state WIN SEQUENCE.
; Inputs  : none
; Outputs : score, hero data, game_state, win_phase, state_timer
; Changes : nothing
; ---------------------------------------------------------------------
EnterWin:
            push    bx

            mov     bx, [hero_y]         ; row of the feet
            add     bx, [hero_h]
            dec     bx
            sar     bx, TILE_SHIFT
            cmp     bx, 0
            jge     .not_above
            xor     bx, bx               ; above the map: top zone
.not_above:
            cmp     bx, FLAG_ROWS - 1
            jle     .row_ok
            mov     bx, FLAG_ROWS - 1    ; lower than row 8: bottom zone
.row_ok:
            shl     bx, 1
            push    word [flag_points + bx]
            call    AddScore

            mov     word [game_state], STATE_WIN
            mov     byte [win_phase], 0
            mov     word [state_timer], 0
            mov     word [hero_x], POLE_SNAP_X
            mov     byte [hero_xf], 0
            mov     byte [hero_yf], 0
            mov     word [hero_vx], 0
            mov     word [hero_vy], 0
            mov     byte [hero_on_ground], 0
            mov     byte [hero_invuln], 0
            mov     byte [hero_facing], FACE_RIGHT

            pop     bx
            ret

; ---------------------------------------------------------------------
; FinishLevel
; Purpose : end of the win sequence: remember score and time for the
;           summary, add time left x 50 exactly once, LEVEL COMPLETE.
;           No extra life is given.
; Inputs  : none
; Outputs : ld_*, score, game_state, hero_visible
; Changes : nothing
; ---------------------------------------------------------------------
FinishLevel:
            push    ax
            push    dx

            mov     ax, [score_lo]
            mov     [ld_score_lo], ax
            mov     ax, [score_hi]
            mov     [ld_score_hi], ax
            mov     ax, [clock]
            mov     [ld_time], ax
            mov     dx, TIME_BONUS_PER_SEC
            mul     dx                   ; ax = time x 50 (<= 15000)
            mov     [ld_bonus], ax
            push    ax
            call    AddScore
            mov     byte [hero_visible], 0
            mov     word [game_state], STATE_LEVEL_DONE

            pop     dx
            pop     ax
            ret

; ---------------------------------------------------------------------
; ResetLevel
; Purpose : restart the level after a lost life (Section 5.8 table):
;           map from the original copy (Member 1's ResetMap, REP MOVSB),
;           enemies back at their starts, mushrooms gone, clock 300,
;           Small Mario (HP 1) at x = 32, camera 0. Score, coins and
;           lives are NOT touched here (NewGame does that).
; Inputs  : none
; Outputs : map, ent_table, hero data, clock, camera_x, flags
; Changes : nothing
; ---------------------------------------------------------------------
ResetLevel:
            pusha
            push    es

            call    ResetMap

            push    ds                   ; clear every entity record
            pop     es
            cld
            mov     di, ent_table
            mov     cx, MAX_ENTS * ENT_SIZE
            xor     al, al
            rep stosb

            mov     si, enemy_start      ; 3 bytes each: type, col, row
            mov     di, ent_table
.next_enemy:
            mov     al, [si]
            test    al, al
            jz      .enemies_done        ; type 0 ends the list
            mov     [di + E_TYPE], al
            mov     byte [di + E_STATE], ES_DORMANT
            mov     byte [di + E_DIR], DIR_LEFT
            mov     al, [si + 1]         ; column -> world x
            xor     ah, ah
            shl     ax, TILE_SHIFT
            mov     [di + E_X], ax
            mov     al, [si + 2]         ; row -> y
            xor     ah, ah
            shl     ax, TILE_SHIFT
            mov     [di + E_Y], ax
            mov     ax, GOOMBA_SPEED
            cmp     byte [di + E_TYPE], ET_KOOPA
            jne     .speed_ok
            mov     ax, KOOPA_SPEED
.speed_ok:
            mov     [di + E_SPEED], ax
            add     si, 3
            add     di, ENT_SIZE
            jmp     .next_enemy
.enemies_done:
            mov     word [hero_x], HERO_START_X   ; Small Mario on row 9
            mov     byte [hero_xf], 0
            mov     word [hero_y], HERO_STAND_ROW * TILE_SIZE
            mov     byte [hero_yf], 0
            mov     word [hero_vx], 0
            mov     word [hero_vy], 0
            mov     word [hero_h], SPRITE_H
            mov     byte [hero_form], FORM_SMALL
            mov     byte [hero_hp], 1
            mov     byte [hero_invuln], 0
            mov     byte [hero_stomp_chain], 0
            mov     byte [hero_facing], FACE_RIGHT
            mov     byte [hero_on_ground], 1
            mov     byte [hero_visible], 1
            mov     byte [hero_anim], 0
            mov     word [hero_prev_feet], HERO_STAND_ROW * TILE_SIZE + SPRITE_H - 1

            mov     word [clock], LEVEL_TIME_SECONDS
            mov     word [camera_x], 0
            mov     word [flag_y], POLE_FLAG_TOP
            mov     byte [pop_timer], 0
            mov     byte [death_pending], 0
            mov     byte [one_second_flag], 0

            pop     es
            popa
            ret

; ---------------------------------------------------------------------
; NewGame
; Purpose : new game from the title: score 0, coins 0, lives 3, then
;           everything ResetLevel does.
; Inputs  : none
; Outputs : score, coins, lives and the level
; Changes : nothing
; ---------------------------------------------------------------------
NewGame:
            mov     word [score_lo], 0
            mov     word [score_hi], 0
            mov     word [coins], 0
            mov     word [lives], START_LIVES
            call    ResetLevel
            ret

; ---------------------------------------------------------------------
; DrawWorld
; Purpose : step 9: clear the frame buffer, then tiles, flag, enemies
;           and items, Mario, and the HUD last (so nothing covers it).
;           The HUD copy brings its own black background, so only the
;           bottom strip of Member 1's DrawHUDFrame is painted here.
; Inputs  : none
; Outputs : frame buffer
; Changes : nothing
; ---------------------------------------------------------------------
DrawWorld:
            call    ClearBuffer
            call    DrawMap
            call    DrawFlag
            call    DrawEntities
            call    DrawPop
            call    DrawHero
            FILLRECT 0, BOTTOM_STRIP_Y, SCREEN_W, BOTTOM_STRIP_H, COL_BLACK
            call    DrawHUD              ; top 16 rows over anything
            ret

; ---------------------------------------------------------------------
; DrawHero
; Purpose : Mario's sprite: Small 16x16 or Super 16x32 (FLAG_TALL);
;           jump pose in the air, run pose (alternating with stand, a
;           simple walking animation) while moving on the ground, stand
;           pose when still. Mirrored when facing left; FLAG_BLINK while
;           invulnerable (Member 1's DrawSprite skips every other phase).
; Inputs  : none (hero data)
; Outputs : frame buffer
; Changes : nothing
; ---------------------------------------------------------------------
DrawHero:
            pusha
            cmp     byte [hero_visible], 0
            je      .done

            mov     bx, 4                ; bx = pose * 2: 0 stand 2 run 4 jump
            cmp     byte [hero_on_ground], 1
            jne     .pose_ok
            xor     bx, bx
            cmp     word [hero_vx], 0
            je      .pose_ok
            inc     byte [hero_anim]
            test    byte [hero_anim], 4  ; switch picture every 4 frames
            jz      .pose_ok
            mov     bx, 2
.pose_ok:
            xor     dx, dx               ; dx = sprite flags
            mov     si, hero_small_sprites
            cmp     byte [hero_form], FORM_SUPER
            jne     .form_ok
            mov     si, hero_super_sprites
            or      dx, FLAG_TALL
.form_ok:
            cmp     byte [hero_facing], FACE_LEFT
            jne     .facing_ok
            or      dx, FLAG_MIRROR      ; the artwork faces right
.facing_ok:
            cmp     byte [hero_invuln], 0
            je      .blink_ok
            or      dx, FLAG_BLINK
.blink_ok:
            push    word [hero_x]
            push    word [hero_y]
            push    word [si + bx]
            push    dx
            call    DrawSpriteCamera
.done:
            popa
            ret

; ---------------------------------------------------------------------
; DrawEntities
; Purpose : draw every used ent_table record with the camera offset:
;           Goomba (walk picture flipped for a walking animation while
;           playing), flat Goomba, Koopa (faces its walking direction),
;           shell, mushroom.
; Inputs  : none
; Outputs : frame buffer
; Changes : nothing
; ---------------------------------------------------------------------
DrawEntities:
            pusha
            mov     si, ent_table
            mov     cx, MAX_ENTS
.next:
            mov     al, [si + E_TYPE]
            cmp     al, ET_NONE
            je      .skip
            xor     dx, dx               ; flags
            cmp     al, ET_MUSHROOM
            jne     .enemy
            mov     bx, spr_mushroom
            jmp     .draw
.enemy:
            mov     ah, [si + E_STATE]
            cmp     ah, ES_SHELL
            je      .shell
            cmp     ah, ES_SLIDE
            je      .shell
            cmp     ah, ES_FLAT
            jne     .walker
            mov     bx, spr_goomba_flat
            jmp     .draw
.shell:
            mov     bx, spr_shell
            jmp     .draw
.walker:
            cmp     al, ET_KOOPA
            je      .koopa
            mov     bx, spr_goomba_walk
            cmp     word [game_state], STATE_PLAYING
            jne     .draw                ; frozen while dying / winning
            cmp     byte [blink_toggle], 0
            je      .draw
            mov     dx, FLAG_MIRROR      ; flip = two walking frames
            jmp     .draw
.koopa:
            mov     bx, spr_koopa        ; the artwork faces left
            cmp     byte [si + E_DIR], DIR_RIGHT
            jne     .draw
            mov     dx, FLAG_MIRROR
.draw:
            push    word [si + E_X]
            push    word [si + E_Y]
            push    bx
            push    dx
            call    DrawSpriteCamera
.skip:
            add     si, ENT_SIZE
            loop    .next
            popa
            ret

; ---------------------------------------------------------------------
; DrawFlag
; Purpose : the flag cloth beside the pole (Member 1's spr_flag); it
;           slides down with Mario in the win sequence.
; Inputs  : none (flag_y)
; Outputs : frame buffer
; Changes : nothing
; ---------------------------------------------------------------------
DrawFlag:
            push    word POLE_FLAG_X
            push    word [flag_y]
            push    word spr_flag
            push    word 0
            call    DrawSpriteCamera
            ret

; ---------------------------------------------------------------------
; DrawPop
; Purpose : the coin rising out of a ? block, while pop_timer > 0.
; Inputs  : none (pop_x, pop_y, pop_timer)
; Outputs : frame buffer
; Changes : nothing
; ---------------------------------------------------------------------
DrawPop:
            cmp     byte [pop_timer], 0
            je      .done
            push    word [pop_x]
            push    word [pop_y]
            push    word spr_coin
            push    word 0
            call    DrawSpriteCamera
.done:
            ret

; Member 2 data. It sits after the last RET, so it is never executed.
%include "gamedata.inc"
