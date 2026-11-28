; =====================================================================
; enemies.inc - Member 2: Module 11 (Goombas, Koopas, shells) and the
; hero-versus-entity part of Module 10 (stomp or hurt).
; All moving things (enemies and mushrooms) live in ent_table: MAX_ENTS
; fixed-size records of ENT_SIZE bytes, reached with based addressing
; [si + E_field]. Field offsets are the E_* equates in game.inc.
; =====================================================================

; ---------------------------------------------------------------------
; UpdateEntities
; Purpose : step 4 of the frame: walk through ent_table and update every
;           used record (movement, turning, falling, activation).
; Inputs  : none
; Outputs : ent_table updated
; Changes : nothing
; ---------------------------------------------------------------------
UpdateEntities:
            push    cx
            push    si

            mov     si, ent_table
            mov     cx, MAX_ENTS
.next:
            cmp     byte [si + E_TYPE], ET_NONE
            je      .skip
            push    si
            call    UpdateEntity
.skip:
            add     si, ENT_SIZE
            loop    .next

            pop     si
            pop     cx
            ret

; ---------------------------------------------------------------------
; UpdateEntity(record)
; Purpose : one frame of one entity.
;           mushroom : move, fall, turn; removed when it leaves the
;                      screen on the left.
;           dormant  : wakes up within one tile of the right screen edge
;                      (only while fewer than 8 enemies are active).
;           flat     : counts down, then removed.
;           walking / shell / sliding shell : move, fall, turn; walkers
;                      turn at other enemies, sliding shells kill them.
;           Removed when more than 32 px left of the screen.
; Inputs  : [bp+4]=record address
; Outputs : record updated (E_TYPE = ET_NONE when removed)
; Changes : nothing (ret 2)
; ---------------------------------------------------------------------
UpdateEntity:
            push    bp
            mov     bp, sp
            push    ax
            push    si

            mov     si, [bp+4]
            cmp     byte [si + E_TYPE], ET_MUSHROOM
            jne     .enemy
            push    si                   ; ---- mushroom ----
            call    MoveEntity
            mov     ax, [camera_x]
            sub     ax, SPRITE_W         ; fully left of the screen?
            cmp     [si + E_X], ax
            jg      .done
            mov     byte [si + E_TYPE], ET_NONE
            jmp     .done

.enemy:
            mov     al, [si + E_STATE]
            cmp     al, ES_DORMANT
            jne     .awake
            mov     ax, [camera_x]       ; ---- dormant: wake up? ----
            add     ax, SCREEN_W + TILE_SIZE
            cmp     [si + E_X], ax
            jge     .done                ; still too far right
            call    CountActiveEnemies
            cmp     ax, MAX_ACTIVE_ENEMIES
            jae     .done                ; 8 already active on screen
            mov     byte [si + E_STATE], ES_WALK
            jmp     .done

.awake:
            cmp     al, ES_FLAT
            jne     .moving
            dec     byte [si + E_TIMER]  ; ---- flattened Goomba ----
            jnz     .done
            mov     byte [si + E_TYPE], ET_NONE
            jmp     .done

.moving:
            cmp     byte [si + E_GRACE], 0
            je      .grace_done
            dec     byte [si + E_GRACE]  ; kick grace counts down
.grace_done:
            push    si
            call    MoveEntity
            cmp     byte [si + E_TYPE], ET_NONE
            je      .done                ; fell out of the map
            cmp     byte [si + E_STATE], ES_WALK
            jne     .not_walking
            push    si
            call    TurnAtEnemies
            jmp     .left_check
.not_walking:
            cmp     byte [si + E_STATE], ES_SLIDE
            jne     .left_check
            push    si
            call    ShellHits
.left_check:
            mov     ax, [camera_x]       ; far behind the camera: removed
            sub     ax, BEHIND_LIMIT
            cmp     [si + E_X], ax
            jge     .done
            mov     byte [si + E_TYPE], ET_NONE
.done:
            pop     si
            pop     ax
            pop     bp
            ret     2

; ---------------------------------------------------------------------
; MoveEntity(record)
; Purpose : move a 16x16 entity: E_SPEED in direction E_DIR, turn round
;           at walls and the level ends, Koopas also at platform edges;
;           then gravity and landing on solid tiles. Falling below the
;           map removes it.
; Inputs  : [bp+4]=record address
; Outputs : E_X, E_XF, E_DIR, E_Y, E_YF, E_VY, E_GROUND, E_TYPE
; Changes : nothing (ret 2)
; ---------------------------------------------------------------------
MoveEntity:
            push    bp
            mov     bp, sp
            pusha

            mov     si, [bp+4]
            ; ---- horizontal ----
            mov     ax, [si + E_SPEED]
            test    ax, ax
            jnz     .has_speed
            jmp     .vertical            ; stationary shell
.has_speed:
            cmp     byte [si + E_DIR], DIR_LEFT
            jne     .dir_ok
            neg     ax
.dir_ok:
            ADD_FIX [si + E_X], [si + E_XF]

            mov     bx, [si + E_X]
            cmp     bx, 0
            jl      .end_left            ; left end of the level
            cmp     bx, LEVEL_WIDTH_PX - SPRITE_W
            jg      .end_right           ; right end of the level
            cmp     byte [si + E_DIR], DIR_LEFT
            je      .lead_left

            add     bx, SPRITE_W - 1     ; right edge column
            push    bx
            push    word [si + E_Y]
            push    word SPRITE_H
            call    SolidColumn
            test    ax, ax
            jz      .edge_check
            and     bx, 0FFF0h           ; wall: stand just left of it
            sub     bx, SPRITE_W
            jmp     .turn
.lead_left:
            push    bx                   ; left edge column
            push    word [si + E_Y]
            push    word SPRITE_H
            call    SolidColumn
            test    ax, ax
            jz      .edge_check
            and     bx, 0FFF0h           ; wall: stand just right of it
            add     bx, TILE_SIZE
            jmp     .turn
.end_left:
            xor     bx, bx
            jmp     .turn
.end_right:
            mov     bx, LEVEL_WIDTH_PX - SPRITE_W
.turn:
            mov     [si + E_X], bx
            mov     byte [si + E_XF], 0
            neg     byte [si + E_DIR]    ; +1 <-> -1
            jmp     .vertical

.edge_check:
            cmp     byte [si + E_TYPE], ET_KOOPA  ; only walking Koopas
            jne     .vertical                     ; look for edges
            cmp     byte [si + E_STATE], ES_WALK
            jne     .vertical
            cmp     byte [si + E_GROUND], 1
            jne     .vertical
            mov     bx, [si + E_X]
            cmp     byte [si + E_DIR], DIR_LEFT
            je      .probe_left
            add     bx, SPRITE_W         ; pixel just past the right edge
            jmp     .probe
.probe_left:
            dec     bx                   ; pixel just past the left edge
.probe:
            mov     ax, [si + E_Y]
            add     ax, SPRITE_H         ; row under the feet
            push    bx
            push    ax
            call    IsSolidAt
            test    ax, ax
            jnz     .vertical
            neg     byte [si + E_DIR]    ; no ground ahead: turn round

            ; ---- vertical ----
.vertical:
            mov     byte [si + E_GROUND], 0
            mov     ax, [si + E_VY]
            add     ax, HERO_GRAVITY
            cmp     ax, HERO_MAX_FALL
            jle     .fall_ok
            mov     ax, HERO_MAX_FALL
.fall_ok:
            mov     [si + E_VY], ax
            ADD_FIX [si + E_Y], [si + E_YF]
            cmp     word [si + E_Y], LEVEL_HEIGHT_PX
            jl      .land_test
            mov     byte [si + E_TYPE], ET_NONE  ; fell into a pit
            jmp     .done
.land_test:
            mov     di, [si + E_X]
            lea     dx, [di + HITBOX_RIGHT]      ; feet span like Mario's
            add     di, HITBOX_LEFT
            mov     bx, [si + E_Y]
            add     bx, SPRITE_H - 1     ; lowest pixel row
            push    di
            push    dx
            push    bx
            call    SolidPair
            test    ax, ax
            jnz     .land
            inc     bx                   ; row just under the feet
            push    di
            push    dx
            push    bx
            call    SolidPair
            test    ax, ax
            jz      .done
.land:
            and     bx, 0FFF0h
            sub     bx, SPRITE_H
            mov     [si + E_Y], bx
            mov     byte [si + E_YF], 0
            mov     word [si + E_VY], 0
            mov     byte [si + E_GROUND], 1
.done:
            popa
            pop     bp
            ret     2

; ---------------------------------------------------------------------
; TurnAtEnemies(record)
; Purpose : a walking enemy that runs into another enemy (walking,
;           dormant or a stationary shell) in front of it turns round.
; Inputs  : [bp+4]=record address
; Outputs : E_DIR of that record
; Changes : nothing (ret 2)
; ---------------------------------------------------------------------
TurnAtEnemies:
            push    bp
            mov     bp, sp
            pusha

            mov     si, [bp+4]
            mov     di, ent_table
            mov     cx, MAX_ENTS
.next:
            cmp     di, si
            je      .skip                ; not with itself
            mov     al, [di + E_TYPE]
            cmp     al, ET_GOOMBA
            je      .is_enemy
            cmp     al, ET_KOOPA
            jne     .skip
.is_enemy:
            cmp     byte [di + E_STATE], ES_FLAT
            je      .skip
            cmp     byte [di + E_STATE], ES_SLIDE
            je      .skip                ; a sliding shell kills instead
            push    word [si + E_X]
            push    word [si + E_Y]
            push    word SPRITE_W
            push    word SPRITE_H
            push    word [di + E_X]
            push    word [di + E_Y]
            push    word SPRITE_W
            push    word SPRITE_H
            call    Overlap
            test    ax, ax
            jz      .skip
            mov     ax, [di + E_X]       ; only if the other one is ahead
            cmp     byte [si + E_DIR], DIR_LEFT
            je      .ahead_left
            cmp     ax, [si + E_X]
            jl      .skip
            jmp     .turn
.ahead_left:
            cmp     ax, [si + E_X]
            jg      .skip
.turn:
            neg     byte [si + E_DIR]
            jmp     .done
.skip:
            add     di, ENT_SIZE
            loop    .next
.done:
            popa
            pop     bp
            ret     2

; ---------------------------------------------------------------------
; ShellHits(record)
; Purpose : a sliding shell removes every Goomba or Koopa it touches,
;           +500 points for each one.
; Inputs  : [bp+4]=address of the sliding shell record
; Outputs : hit records get E_TYPE = ET_NONE; score
; Changes : nothing (ret 2)
; ---------------------------------------------------------------------
ShellHits:
            push    bp
            mov     bp, sp
            pusha

            mov     si, [bp+4]
            mov     di, ent_table
            mov     cx, MAX_ENTS
.next:
            cmp     di, si
            je      .skip
            mov     al, [di + E_TYPE]
            cmp     al, ET_GOOMBA
            je      .target
            cmp     al, ET_KOOPA
            jne     .skip
.target:
            cmp     byte [di + E_STATE], ES_FLAT
            je      .skip                ; already defeated
            push    word [si + E_X]
            push    word [si + E_Y]
            push    word SPRITE_W
            push    word SPRITE_H
            push    word [di + E_X]
            push    word [di + E_Y]
            push    word SPRITE_W
            push    word SPRITE_H
            call    Overlap
            test    ax, ax
            jz      .skip
            mov     byte [di + E_TYPE], ET_NONE
            push    word SHELL_KILL_POINTS
            call    AddScore
.skip:
            add     di, ENT_SIZE
            loop    .next

            popa
            pop     bp
            ret     2

; ---------------------------------------------------------------------
; CountActiveEnemies
; Purpose : number of awake Goombas/Koopas that are on screen.
; Inputs  : none
; Outputs : AX = count
; Changes : AX
; ---------------------------------------------------------------------
CountActiveEnemies:
            push    bx
            push    cx
            push    si

            xor     bx, bx
            mov     si, ent_table
            mov     cx, MAX_ENTS
.next:
            mov     al, [si + E_TYPE]
            cmp     al, ET_GOOMBA
            je      .enemy
            cmp     al, ET_KOOPA
            jne     .skip
.enemy:
            cmp     byte [si + E_STATE], ES_DORMANT
            je      .skip
            push    word [si + E_X]
            call    IsOnScreen           ; AX = 1 if visible
            add     bx, ax
.skip:
            add     si, ENT_SIZE
            loop    .next

            mov     ax, bx
            pop     si
            pop     cx
            pop     bx
            ret

; ---------------------------------------------------------------------
; ResolveHeroEntities
; Purpose : step 5 of the frame: test Mario's hitbox against every
;           entity and apply the collision outcomes (HeroTouches).
;           "Moving down" is decided once before the loop, so a bounce
;           from the first stomp does not turn a second stomp in the
;           same frame into side contact.
; Inputs  : none
; Outputs : entities, hero, score changed
; Changes : nothing
; ---------------------------------------------------------------------
ResolveHeroEntities:
            pusha

            xor     dx, dx               ; dx = 1 when Mario moves down
            cmp     word [hero_vy], 0
            jle     .dir_known
            inc     dx
.dir_known:
            mov     si, ent_table
            mov     cx, MAX_ENTS
.next:
            cmp     byte [death_pending], 0
            jne     .done                ; Mario already died this frame
            cmp     byte [si + E_TYPE], ET_NONE
            je      .skip
            cmp     byte [si + E_STATE], ES_FLAT
            je      .skip                ; flat Goombas are harmless
            mov     ax, [hero_x]
            add     ax, HITBOX_LEFT
            push    ax                   ; Mario's hitbox
            push    word [hero_y]
            push    word HITBOX_W
            push    word [hero_h]
            push    word [si + E_X]      ; the entity
            push    word [si + E_Y]
            push    word SPRITE_W
            push    word SPRITE_H
            call    Overlap
            test    ax, ax
            jz      .skip
            push    si
            push    dx
            call    HeroTouches
.skip:
            add     si, ENT_SIZE
            loop    .next
.done:
            popa
            ret

; ---------------------------------------------------------------------
; HeroTouches(record, moving_down)
; Purpose : outcome of Mario overlapping one entity (Section 5.6).
;           Stomp = moving down AND last frame's feet above the upper
;           half of the enemy.
;           mushroom          : collected (+1000, grow).
;           Goomba/Koopa      : stomp -> flat / shell, chain points,
;                               bounce; otherwise HurtHero.
;           stationary shell  : kicked away from Mario (+400), 8 frame
;                               grace; bounce if it was a stomp.
;           sliding shell     : stomp stops it (chain points), else hurt.
;           A shell inside its grace frames is ignored.
; Inputs  : [bp+6]=record address  [bp+4]=1 if Mario moves down
; Outputs : record, hero, score
; Changes : nothing (ret 4)
; ---------------------------------------------------------------------
HeroTouches:
            push    bp
            mov     bp, sp
            push    ax
            push    bx
            push    cx
            push    si

            mov     si, [bp+6]
            cmp     byte [si + E_TYPE], ET_MUSHROOM
            jne     .enemy
            mov     byte [si + E_TYPE], ET_NONE  ; ---- mushroom ----
            push    word MUSHROOM_POINTS
            call    AddScore
            call    GrowHero
            jmp     .done

.enemy:
            xor     bx, bx               ; bx = 1 for a stomp
            cmp     word [bp+4], 0
            je      .stomp_known
            mov     ax, [si + E_Y]
            add     ax, SPRITE_H / 2     ; middle of the enemy
            cmp     [hero_prev_feet], ax
            jge     .stomp_known
            mov     bx, 1
.stomp_known:
            mov     al, [si + E_STATE]
            cmp     al, ES_SHELL
            je      .still_shell
            cmp     al, ES_SLIDE
            je      .sliding_shell

            test    bx, bx               ; ---- walking enemy ----
            jz      .hurt
            cmp     byte [si + E_TYPE], ET_GOOMBA
            jne     .koopa_stomp
            mov     byte [si + E_STATE], ES_FLAT
            mov     byte [si + E_TIMER], FLAT_FRAMES
            jmp     .stomped
.koopa_stomp:
            mov     byte [si + E_STATE], ES_SHELL
            mov     word [si + E_SPEED], 0
            mov     byte [si + E_GRACE], KICK_GRACE
.stomped:
            call    StompPoints
            call    BounceHero
            jmp     .done

.still_shell:
            cmp     byte [si + E_GRACE], 0
            jne     .done
            mov     cx, [hero_x]         ; kick away from Mario
            add     cx, HERO_CENTER_X
            mov     ax, [si + E_X]
            add     ax, SPRITE_W / 2
            mov     byte [si + E_DIR], DIR_RIGHT
            cmp     ax, cx
            jge     .kick_dir_ok
            mov     byte [si + E_DIR], DIR_LEFT
.kick_dir_ok:
            mov     byte [si + E_STATE], ES_SLIDE
            mov     word [si + E_SPEED], SHELL_SPEED
            mov     byte [si + E_GRACE], KICK_GRACE
            push    word KICK_POINTS
            call    AddScore
            test    bx, bx
            jz      .done
            call    BounceHero
            jmp     .done

.sliding_shell:
            cmp     byte [si + E_GRACE], 0
            jne     .done
            test    bx, bx
            jz      .hurt
            mov     byte [si + E_STATE], ES_SHELL  ; stomp stops it
            mov     word [si + E_SPEED], 0
            mov     byte [si + E_GRACE], KICK_GRACE
            jmp     .stomped

.hurt:
            call    HurtHero             ; ignored while invulnerable
.done:
            pop     si
            pop     cx
            pop     bx
            pop     ax
            pop     bp
            ret     4

; ---------------------------------------------------------------------
; BounceHero
; Purpose : Mario bounces up after a stomp; higher while a jump key is
;           held.
; Inputs  : none
; Outputs : hero_vy, hero_on_ground
; Changes : nothing
; ---------------------------------------------------------------------
BounceHero:
            push    ax
            call    JumpKeyDown
            test    ax, ax
            mov     ax, -STOMP_BOUNCE
            jz      .set
            mov     ax, -STOMP_BOUNCE_HIGH
.set:
            mov     [hero_vy], ax
            mov     byte [hero_on_ground], 0
            pop     ax
            ret
