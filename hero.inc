; =====================================================================
; hero.inc - Member 2: keys, Module 9 (player physics) and Module 10
; (tile collision, rectangle overlap).
; Fixed point: positions keep the 8.8 idea but the whole part is a word
; (the level is 960 px wide, more than one byte can hold):
;     hero_x (word, whole pixels) + hero_xf (byte, 1/256 pixel)
; Velocities are signed 8.8 words: high byte = whole px per frame,
; low byte = fraction. ADD_FIX (game.inc) adds one to the other.
; =====================================================================

; ---------------------------------------------------------------------
; ReadKeys
; Purpose : step 2 of the frame. Last frame's snapshot becomes keys_old,
;           then key_table (filled by the keyboard ISR) is copied to
;           keys_cur with REP MOVSB. Edges are found by comparing both.
; Inputs  : none
; Outputs : keys_old, keys_cur updated
; Changes : nothing
; ---------------------------------------------------------------------
ReadKeys:
            push    cx
            push    si
            push    di
            push    es
            cld

            push    ds                   ; ES = DS (same segment)
            pop     es
            mov     si, keys_cur         ; old snapshot <- current snapshot
            mov     di, keys_old
            mov     cx, KEY_TABLE_SIZE
            rep movsb
            mov     si, key_table        ; current snapshot <- live table
            mov     di, keys_cur
            mov     cx, KEY_TABLE_SIZE
            rep movsb

            pop     es
            pop     di
            pop     si
            pop     cx
            ret

; ---------------------------------------------------------------------
; KeyDown(scan)
; Purpose : is the key held in this frame's snapshot?
; Inputs  : [bp+4]=scan code
; Outputs : AX = 1 held, 0 not held
; Changes : AX (ret 2)
; ---------------------------------------------------------------------
KeyDown:
            push    bp
            mov     bp, sp
            push    bx

            mov     bx, [bp+4]
            and     bx, KEY_TABLE_SIZE - 1
            xor     ax, ax
            cmp     byte [keys_cur + bx], 0
            je      .out
            mov     ax, 1
.out:
            pop     bx
            pop     bp
            ret     2

; ---------------------------------------------------------------------
; KeyHit(scan)
; Purpose : was the key pressed this frame (held now, not last frame)?
;           Used for P, Esc, Enter, Up/Down and the jump start, so one
;           long press only acts once.
; Inputs  : [bp+4]=scan code
; Outputs : AX = 1 new press, else 0
; Changes : AX (ret 2)
; ---------------------------------------------------------------------
KeyHit:
            push    bp
            mov     bp, sp
            push    bx

            mov     bx, [bp+4]
            and     bx, KEY_TABLE_SIZE - 1
            xor     ax, ax
            cmp     byte [keys_cur + bx], 0
            je      .out                 ; not held now
            cmp     byte [keys_old + bx], 0
            jne     .out                 ; already held last frame
            mov     ax, 1
.out:
            pop     bx
            pop     bp
            ret     2

; ---------------------------------------------------------------------
; JumpKeyDown
; Purpose : is a jump key (Space or Up) held?
; Inputs  : none
; Outputs : AX = 1 held, else 0
; Changes : AX
; ---------------------------------------------------------------------
JumpKeyDown:
            push    bx
            push    word KEY_SPACE
            call    KeyDown
            mov     bx, ax
            push    word KEY_UP
            call    KeyDown
            or      ax, bx
            pop     bx
            ret

; ---------------------------------------------------------------------
; IsSolidAt(px, py)
; Purpose : is the tile under a world pixel solid? Solid codes are
;           TILE_GROUND..TILE_STAIR; sky, coin and flag pole are not.
;           Uses Member 1's GetTile (outside the map = sky).
; Inputs  : [bp+6]=px  [bp+4]=py (signed)
; Outputs : AX = 1 solid, 0 free
; Changes : AX (ret 4)
; ---------------------------------------------------------------------
IsSolidAt:
            push    bp
            mov     bp, sp

            push    word [bp+6]
            push    word [bp+4]
            call    GetTile              ; AL = tile code
            cmp     al, TILE_GROUND
            jb      .free
            cmp     al, TILE_STAIR
            ja      .free
            mov     ax, 1
            jmp     .out
.free:
            xor     ax, ax
.out:
            pop     bp
            ret     4

; ---------------------------------------------------------------------
; SolidPair(px1, px2, py)
; Purpose : is either of two points on the same pixel row solid?
;           (left and right foot, or both sides of the head)
; Inputs  : [bp+8]=px1  [bp+6]=px2  [bp+4]=py
; Outputs : AX = 1 if one of them is solid, else 0
; Changes : AX (ret 6)
; ---------------------------------------------------------------------
SolidPair:
            push    bp
            mov     bp, sp

            push    word [bp+8]
            push    word [bp+4]
            call    IsSolidAt
            test    ax, ax
            jnz     .out
            push    word [bp+6]
            push    word [bp+4]
            call    IsSolidAt
.out:
            pop     bp
            ret     6

; ---------------------------------------------------------------------
; SolidColumn(px, top, height)
; Purpose : wall test for one side of a box: checks the top, middle and
;           bottom pixel at column px. The points are never more than
;           16 px apart, so no tile in between can be missed.
; Inputs  : [bp+8]=px  [bp+6]=top y  [bp+4]=height (16 or 32)
; Outputs : AX = 1 if a solid tile is touched, else 0
; Changes : AX (ret 6)
; ---------------------------------------------------------------------
SolidColumn:
            push    bp
            mov     bp, sp
            push    bx

            mov     bx, [bp+6]           ; top
            push    word [bp+8]
            push    bx
            call    IsSolidAt
            test    ax, ax
            jnz     .out
            mov     ax, [bp+4]           ; middle = top + height/2
            shr     ax, 1
            add     bx, ax
            push    word [bp+8]
            push    bx
            call    IsSolidAt
            test    ax, ax
            jnz     .out
            mov     bx, [bp+6]           ; bottom = top + height - 1
            add     bx, [bp+4]
            dec     bx
            push    word [bp+8]
            push    bx
            call    IsSolidAt
.out:
            pop     bx
            pop     bp
            ret     6

; ---------------------------------------------------------------------
; Overlap(x1, y1, w1, h1, x2, y2, w2, h2)
; Purpose : axis-aligned rectangle overlap test (signed coordinates).
;           Overlap when x1 < x2+w2, x2 < x1+w1, y1 < y2+h2, y2 < y1+h1.
; Inputs  : [bp+18]=x1 [bp+16]=y1 [bp+14]=w1 [bp+12]=h1
;           [bp+10]=x2 [bp+8]=y2  [bp+6]=w2  [bp+4]=h2
; Outputs : AX = 1 overlap, 0 apart
; Changes : AX (ret 16)
; ---------------------------------------------------------------------
Overlap:
            push    bp
            mov     bp, sp
            push    bx

            xor     ax, ax               ; assume apart
            mov     bx, [bp+10]
            add     bx, [bp+6]           ; x2 + w2
            cmp     [bp+18], bx
            jge     .out
            mov     bx, [bp+18]
            add     bx, [bp+14]          ; x1 + w1
            cmp     [bp+10], bx
            jge     .out
            mov     bx, [bp+8]
            add     bx, [bp+4]           ; y2 + h2
            cmp     [bp+16], bx
            jge     .out
            mov     bx, [bp+16]
            add     bx, [bp+12]          ; y1 + h1
            cmp     [bp+8], bx
            jge     .out
            mov     ax, 1
.out:
            pop     bx
            pop     bp
            ret     16

; ---------------------------------------------------------------------
; UpdateHero
; Purpose : step 3 of the frame (Module 9): walk/run acceleration,
;           friction, faster braking, jump (ground only), variable jump
;           height, gravity with a maximum fall speed, then move one axis
;           at a time with tile collision (MoveHeroX, MoveHeroY).
; Inputs  : none (keys_cur/keys_old snapshot, hero data)
; Outputs : hero data updated
; Changes : nothing
; ---------------------------------------------------------------------
UpdateHero:
            pusha

            mov     ax, [hero_y]         ; feet of the previous frame,
            add     ax, [hero_h]         ; needed by the stomp test
            dec     ax
            mov     [hero_prev_feet], ax

            ; ---- horizontal speed ----
            mov     dx, HERO_WALK_MAX    ; dx = speed limit
            push    word KEY_LSHIFT
            call    KeyDown
            test    ax, ax
            jz      .limit_ok
            mov     dx, HERO_RUN_MAX     ; Left Shift held: run
.limit_ok:
            push    word KEY_LEFT
            call    KeyDown
            mov     bl, al               ; bl = left held
            push    word KEY_RIGHT
            call    KeyDown
            mov     bh, al               ; bh = right held
            mov     ax, [hero_vx]        ; ax = speed being worked on
            cmp     bh, bl
            je      .no_direction        ; neither (or both) held
            test    bh, bh
            jz      .go_left

            ; -- right held --
            mov     byte [hero_facing], FACE_RIGHT
            test    ax, ax
            jge     .accel_right
            add     ax, HERO_BRAKE       ; still moving left: brake hard
            jmp     .speed_done
.accel_right:
            cmp     ax, dx
            jg      .slow_right          ; above the limit (Shift released)
            add     ax, HERO_ACCEL
            cmp     ax, dx
            jle     .speed_done
            mov     ax, dx               ; limit reached
            jmp     .speed_done
.slow_right:
            sub     ax, HERO_FRICTION    ; ease down to the walk limit
            cmp     ax, dx
            jge     .speed_done
            mov     ax, dx
            jmp     .speed_done

            ; -- left held (mirror image, limit is -dx) --
.go_left:
            mov     byte [hero_facing], FACE_LEFT
            neg     dx
            test    ax, ax
            jle     .accel_left
            sub     ax, HERO_BRAKE       ; still moving right: brake hard
            jmp     .speed_done
.accel_left:
            cmp     ax, dx
            jl      .slow_left
            sub     ax, HERO_ACCEL
            cmp     ax, dx
            jge     .speed_done
            mov     ax, dx
            jmp     .speed_done
.slow_left:
            add     ax, HERO_FRICTION
            cmp     ax, dx
            jle     .speed_done
            mov     ax, dx
            jmp     .speed_done

            ; -- no direction: friction slides Mario to a stop --
.no_direction:
            test    ax, ax
            jz      .speed_done
            jl      .friction_left
            sub     ax, HERO_FRICTION
            jge     .speed_done
            xor     ax, ax               ; do not overshoot zero
            jmp     .speed_done
.friction_left:
            add     ax, HERO_FRICTION
            jle     .speed_done
            xor     ax, ax
.speed_done:
            mov     [hero_vx], ax

            ; ---- jump: only from solid ground, needs a new key press ----
            push    word KEY_SPACE
            call    KeyHit
            mov     cx, ax
            push    word KEY_UP
            call    KeyHit
            or      cx, ax
            jz      .no_jump
            cmp     byte [hero_on_ground], 1
            jne     .no_jump
            mov     word [hero_vy], -HERO_JUMP_VEL
            mov     byte [hero_on_ground], 0
.no_jump:
            ; ---- variable height: key released while rising fast ----
            call    JumpKeyDown
            test    ax, ax
            jnz     .no_cut
            cmp     word [hero_vy], -HERO_JUMP_CUT
            jge     .no_cut              ; rising slower than the cut-off
            mov     word [hero_vy], -HERO_JUMP_CUT
.no_cut:
            ; ---- gravity, limited to the maximum fall speed ----
            mov     ax, [hero_vy]
            add     ax, HERO_GRAVITY
            cmp     ax, HERO_MAX_FALL
            jle     .fall_ok
            mov     ax, HERO_MAX_FALL
.fall_ok:
            mov     [hero_vy], ax

            call    MoveHeroX            ; first horizontally,
            call    MoveHeroY            ; then vertically

            popa
            ret

; ---------------------------------------------------------------------
; MoveHeroX
; Purpose : add hero_vx to the position, keep Mario between the left
;           edge of the screen and the right end of the level, and stop
;           him flush against a wall in front (speed 0).
;           |vx| <= 3 px, so he can never jump over a whole tile.
; Inputs  : none (hero data, camera_x)
; Outputs : hero_x, hero_xf, hero_vx
; Changes : nothing
; ---------------------------------------------------------------------
MoveHeroX:
            pusha

            mov     ax, [hero_vx]
            ADD_FIX [hero_x], [hero_xf]

            mov     ax, [hero_x]         ; level edges
            cmp     ax, [camera_x]
            jge     .left_ok
            mov     ax, [camera_x]       ; invisible wall at the screen edge
            jmp     .edge_stop
.left_ok:
            cmp     ax, HERO_MAX_X
            jle     .edges_done
            mov     ax, HERO_MAX_X       ; right end of the level
.edge_stop:
            mov     [hero_x], ax
            mov     byte [hero_xf], 0
            mov     word [hero_vx], 0
.edges_done:
            mov     ax, [hero_vx]
            test    ax, ax
            jz      .done                ; not moving sideways
            jl      .moving_left

            mov     bx, [hero_x]         ; moving right: right hitbox edge
            add     bx, HITBOX_RIGHT
            push    bx
            push    word [hero_y]
            push    word [hero_h]
            call    SolidColumn
            test    ax, ax
            jz      .done
            and     bx, 0FFF0h           ; left side of the wall tile
            sub     bx, HITBOX_RIGHT + 1 ; hitbox ends just before it
            jmp     .blocked

.moving_left:
            mov     bx, [hero_x]         ; moving left: left hitbox edge
            add     bx, HITBOX_LEFT
            push    bx
            push    word [hero_y]
            push    word [hero_h]
            call    SolidColumn
            test    ax, ax
            jz      .done
            and     bx, 0FFF0h
            add     bx, TILE_SIZE - HITBOX_LEFT  ; hitbox starts after the wall
.blocked:
            mov     [hero_x], bx
            mov     byte [hero_xf], 0
            mov     word [hero_vx], 0
.done:
            popa
            ret

; ---------------------------------------------------------------------
; MoveHeroY
; Purpose : add hero_vy, then
;           falling/standing: feet in (or just above) a solid tile ->
;             stand on top, vy = 0, on_ground = 1, stomp chain = 0;
;           rising: head in a solid tile -> placed under it, vy = 0 and
;             TriggerBlock on the tile above the centre of the head.
;           vy <= 8 px per frame, so a floor can never be skipped.
; Inputs  : none (hero data)
; Outputs : hero_y, hero_yf, hero_vy, hero_on_ground, hero_stomp_chain
; Changes : nothing
; ---------------------------------------------------------------------
MoveHeroY:
            pusha

            mov     byte [hero_on_ground], 0
            mov     ax, [hero_vy]
            ADD_FIX [hero_y], [hero_yf]

            mov     si, [hero_x]
            lea     di, [si + HITBOX_RIGHT]  ; di = right edge of the hitbox
            add     si, HITBOX_LEFT          ; si = left edge of the hitbox
            cmp     word [hero_vy], 0
            jl      .rising

            ; ---- falling or standing ----
            mov     bx, [hero_y]
            add     bx, [hero_h]
            dec     bx                   ; bx = lowest pixel row (feet)
            push    si
            push    di
            push    bx
            call    SolidPair
            test    ax, ax
            jnz     .land                ; feet sank into a tile
            inc     bx                   ; first row under the feet
            push    si
            push    di
            push    bx
            call    SolidPair
            test    ax, ax
            jz      .done                ; nothing below: in the air
.land:
            and     bx, 0FFF0h           ; top of that tile row
            sub     bx, [hero_h]
            mov     [hero_y], bx
            mov     byte [hero_yf], 0
            mov     word [hero_vy], 0
            mov     byte [hero_on_ground], 1
            mov     byte [hero_stomp_chain], 0
            jmp     .done

            ; ---- rising: head check ----
.rising:
            mov     bx, [hero_y]
            push    si
            push    di
            push    bx
            call    SolidPair
            test    ax, ax
            jz      .done
            mov     word [hero_vy], 0    ; head bump: start falling
            mov     ax, [hero_x]         ; block = tile above head centre
            add     ax, HERO_CENTER_X
            shr     ax, TILE_SHIFT       ; column
            mov     cx, bx
            shr     cx, TILE_SHIFT       ; row (bx >= 0: it hit a tile)
            push    ax
            push    cx
            call    TriggerBlock
            and     bx, 0FFF0h           ; just below that tile
            add     bx, TILE_SIZE
            mov     [hero_y], bx
            mov     byte [hero_yf], 0
.done:
            popa
            ret

; ---------------------------------------------------------------------
; UpdateHeroCamera
; Purpose : step 8 of the frame: Member 1's UpdateCamera scrolls and
;           returns Mario's x pushed back to the left screen edge; if it
;           changed, Mario hit the invisible wall, so his speed is 0.
; Inputs  : none (hero_x)
; Outputs : camera_x, hero_x, hero_xf, hero_vx
; Changes : nothing
; ---------------------------------------------------------------------
UpdateHeroCamera:
            push    ax
            push    word [hero_x]
            call    UpdateCamera         ; AX = corrected x
            cmp     ax, [hero_x]
            je      .done
            mov     [hero_x], ax
            mov     byte [hero_xf], 0
            mov     word [hero_vx], 0
.done:
            pop     ax
            ret
