; =====================================================================
; items.inc - Member 2: Module 12 (blocks, coins, mushroom power-up).
; Only the working map (level_work) is changed, always through Member
; 1's GetTile / SetTile. ResetMap copies level_orig back on restart.
; =====================================================================

; ---------------------------------------------------------------------
; TriggerBlock(col, row)
; Purpose : Mario's head hit this tile from below (Section 5.9).
;           brick      : Super Mario breaks it (sky, +50); Small: nothing
;           ? coin     : empty block, coin pops out, +200, coins +1
;           ? mushroom : empty block, mushroom appears on top
;           anything else (empty block, ground, pipe ...): nothing
;           Each ? block becomes empty, so it can only pay once.
; Inputs  : [bp+6]=column  [bp+4]=row
; Outputs : level_work, score, coins, ent_table
; Changes : nothing (ret 4)
; ---------------------------------------------------------------------
TriggerBlock:
            push    bp
            mov     bp, sp
            push    ax
            push    bx
            push    dx

            mov     dx, [bp+6]           ; dx = x of the tile
            shl     dx, TILE_SHIFT
            mov     bx, [bp+4]           ; bx = y of the tile
            shl     bx, TILE_SHIFT
            push    dx
            push    bx
            call    GetTile              ; AL = tile code
            cmp     al, TILE_BRICK
            je      .brick
            cmp     al, TILE_QCOIN
            je      .coin_block
            cmp     al, TILE_QMUSHROOM
            je      .mushroom_block
            jmp     .done

.brick:
            cmp     byte [hero_form], FORM_SUPER
            jne     .done                ; Small Mario only bumps it
            push    word [bp+6]
            push    word [bp+4]
            push    word TILE_SKY
            call    SetTile
            push    word BRICK_POINTS
            call    AddScore
            jmp     .done

.coin_block:
            push    word [bp+6]
            push    word [bp+4]
            push    word TILE_EMPTY
            call    SetTile
            push    word COIN_POINTS
            call    AddScore
            call    AddCoin
            mov     [pop_x], dx          ; coin pops out above the block
            sub     bx, TILE_SIZE
            mov     [pop_y], bx
            mov     byte [pop_timer], POP_FRAMES
            jmp     .done

.mushroom_block:
            push    word [bp+6]
            push    word [bp+4]
            push    word TILE_EMPTY
            call    SetTile
            sub     bx, TILE_SIZE        ; mushroom sits on top of the block
            push    dx
            push    bx
            call    SpawnMushroom
.done:
            pop     dx
            pop     bx
            pop     ax
            pop     bp
            ret     4

; ---------------------------------------------------------------------
; SpawnMushroom(x, y)
; Purpose : put a mushroom into the first free ent_table record. It
;           moves right at 1 px per frame (UpdateEntity / MoveEntity).
;           If the table is full nothing happens.
; Inputs  : [bp+6]=world x  [bp+4]=y
; Outputs : ent_table
; Changes : nothing (ret 4)
; ---------------------------------------------------------------------
SpawnMushroom:
            push    bp
            mov     bp, sp
            push    ax
            push    cx
            push    di
            push    es

            mov     di, ent_table        ; find a free record
            mov     cx, MAX_ENTS
.find:
            cmp     byte [di + E_TYPE], ET_NONE
            je      .found
            add     di, ENT_SIZE
            loop    .find
            jmp     .done                ; table full
.found:
            push    di                   ; clear the record (REP STOSB)
            push    ds
            pop     es
            cld
            xor     al, al
            mov     cx, ENT_SIZE
            rep stosb
            pop     di

            mov     byte [di + E_TYPE], ET_MUSHROOM
            mov     byte [di + E_STATE], ES_WALK
            mov     ax, [bp+6]
            mov     [di + E_X], ax
            mov     ax, [bp+4]
            mov     [di + E_Y], ax
            mov     byte [di + E_DIR], DIR_RIGHT
            mov     word [di + E_SPEED], MUSHROOM_SPEED
.done:
            pop     es
            pop     di
            pop     cx
            pop     ax
            pop     bp
            ret     4

; ---------------------------------------------------------------------
; CollectCoinAt(px, py)
; Purpose : if the tile under this point is a coin: +200, coins +1, and
;           the tile becomes sky in the working map (so it cannot be
;           collected twice).
; Inputs  : [bp+6]=px  [bp+4]=py
; Outputs : level_work, score, coins
; Changes : nothing (ret 4)
; ---------------------------------------------------------------------
CollectCoinAt:
            push    bp
            mov     bp, sp
            push    ax
            push    bx
            push    cx

            push    word [bp+6]
            push    word [bp+4]
            call    GetTile
            cmp     al, TILE_COIN
            jne     .done                ; (outside the map GetTile = sky)
            mov     bx, [bp+6]
            shr     bx, TILE_SHIFT       ; column
            mov     cx, [bp+4]
            shr     cx, TILE_SHIFT       ; row
            push    bx
            push    cx
            push    word TILE_SKY
            call    SetTile
            push    word COIN_POINTS
            call    AddScore
            call    AddCoin
.done:
            pop     cx
            pop     bx
            pop     ax
            pop     bp
            ret     4

; ---------------------------------------------------------------------
; CollectCoins
; Purpose : step 5: collect map coins under Mario's hitbox. Tests the
;           left and right edge at the top, middle and bottom row, so a
;           16 or 32 px tall Mario touches every coin tile he overlaps.
; Inputs  : none (hero data)
; Outputs : see CollectCoinAt
; Changes : nothing
; ---------------------------------------------------------------------
CollectCoins:
            pusha

            mov     si, [hero_x]
            lea     di, [si + HITBOX_RIGHT]
            add     si, HITBOX_LEFT
            mov     bx, [hero_y]         ; top row
            mov     dx, [hero_h]
            shr     dx, 1                ; dx = half height
            mov     cx, 3                ; top, middle, bottom
.row:
            push    si
            push    bx
            call    CollectCoinAt
            push    di
            push    bx
            call    CollectCoinAt
            add     bx, dx               ; next test row
            cmp     cx, 2
            jne     .next
            dec     bx                   ; bottom row is top + height - 1
.next:
            loop    .row

            popa
            ret

; ---------------------------------------------------------------------
; HeroOnPole
; Purpose : does Mario's hitbox overlap a flag pole tile? Same six test
;           points as CollectCoins.
; Inputs  : none (hero data)
; Outputs : AX = 1 touching the pole, else 0
; Changes : AX
; ---------------------------------------------------------------------
HeroOnPole:
            push    bx
            push    cx
            push    dx
            push    si
            push    di

            mov     si, [hero_x]
            lea     di, [si + HITBOX_RIGHT]
            add     si, HITBOX_LEFT
            mov     bx, [hero_y]
            mov     dx, [hero_h]
            shr     dx, 1
            mov     cx, 3
.row:
            push    si
            push    bx
            call    GetTile
            cmp     al, TILE_FLAGPOLE
            je      .yes
            push    di
            push    bx
            call    GetTile
            cmp     al, TILE_FLAGPOLE
            je      .yes
            add     bx, dx
            cmp     cx, 2
            jne     .next
            dec     bx
.next:
            loop    .row
            xor     ax, ax
            jmp     .out
.yes:
            mov     ax, 1
.out:
            pop     di
            pop     si
            pop     dx
            pop     cx
            pop     bx
            ret

; ---------------------------------------------------------------------
; UpdatePop
; Purpose : step 4: the coin that popped out of a ? block rises for a
;           few frames, then disappears.
; Inputs  : none
; Outputs : pop_y, pop_timer
; Changes : nothing
; ---------------------------------------------------------------------
UpdatePop:
            cmp     byte [pop_timer], 0
            je      .done
            dec     byte [pop_timer]
            sub     word [pop_y], POP_RISE
.done:
            ret
