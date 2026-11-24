; =====================================================================
; map.inc - tile map access and drawing (60 x 12 tiles, 1 byte each)
; Index of a tile = row*60 + col, computed with shifts (60 = 64 - 4).
; =====================================================================

; ---------------------------------------------------------------------
; GetTile(px, py)
; Purpose : tile code under a world pixel position.
;           Anything outside the map counts as sky (0).
; Inputs  : [bp+6]=px  [bp+4]=py (signed words)
; Outputs : AL = tile code (AH unchanged)
; Changes : AL only (ret 4)
; ---------------------------------------------------------------------
GetTile:
            push    bp
            mov     bp, sp
            push    bx
            push    cx
            push    si

            mov     bx, [bp+6]           ; px
            cmp     bx, LEVEL_WIDTH_PX   ; unsigned: negatives are rejected too
            jae     .sky
            mov     cx, [bp+4]           ; py
            cmp     cx, LEVEL_HEIGHT_PX
            jae     .sky

            shr     bx, TILE_SHIFT       ; bx = column
            shr     cx, TILE_SHIFT       ; cx = row
            mov     si, cx
            shl     cx, 6                ; row*64
            shl     si, 2                ; row*4
            sub     cx, si               ; row*60
            add     bx, cx               ; bx = row*60 + column
            mov     al, [level_work + bx]
            jmp     .out
.sky:
            xor     al, al
.out:
            pop     si
            pop     cx
            pop     bx
            pop     bp
            ret     4

; ---------------------------------------------------------------------
; SetTile(col, row, code)
; Purpose : change one tile of the working map. Out of range is ignored.
; Inputs  : [bp+8]=col  [bp+6]=row  [bp+4]=code (low byte used)
; Outputs : none
; Changes : nothing (ret 6)
; ---------------------------------------------------------------------
SetTile:
            push    bp
            mov     bp, sp
            push    ax
            push    bx
            push    si

            mov     bx, [bp+8]           ; column
            cmp     bx, LEVEL_COLS
            jae     .done
            mov     ax, [bp+6]           ; row
            cmp     ax, LEVEL_ROWS
            jae     .done

            mov     si, ax
            shl     ax, 6                ; row*60 = row*64 - row*4
            shl     si, 2
            sub     ax, si
            add     bx, ax
            mov     al, [bp+4]
            mov     [level_work + bx], al
.done:
            pop     si
            pop     bx
            pop     ax
            pop     bp
            ret     6

; ---------------------------------------------------------------------
; DrawTile(x, y, code)
; Purpose : draw one 16x16 tile at a screen position (sky is skipped
;           because ClearBuffer already painted the sky).
; Inputs  : [bp+8]=x  [bp+6]=y  [bp+4]=code
; Outputs : none
; Changes : nothing (ret 6)
; ---------------------------------------------------------------------
DrawTile:
            push    bp
            mov     bp, sp
            push    ax
            push    bx

            mov     bx, [bp+4]           ; tile code
            cmp     bx, TILE_FLAGPOLE    ; unknown codes are ignored
            ja      .done
            test    bx, bx               ; sky: nothing to draw
            jz      .done
            shl     bx, 1                ; table of words
            mov     ax, [tile_address_table + bx]
            push    word [bp+8]          ; DrawSprite(x, y, bitmap, 0)
            push    word [bp+6]
            push    ax
            push    word 0
            call    DrawSprite
.done:
            pop     bx
            pop     ax
            pop     bp
            ret     6

; ---------------------------------------------------------------------
; DrawMap
; Purpose : draw the visible part of the map (21 columns, rows 1-11).
;           First column = camera_x >> 4, its x = -(camera_x & 15).
; Inputs  : none (uses camera_x and level_work)
; Outputs : none
; Changes : nothing
; ---------------------------------------------------------------------
DrawMap:
            pusha
            mov     ax, [camera_x]
            mov     si, ax
            shr     si, TILE_SHIFT       ; si = first map column
            and     ax, TILE_SIZE - 1
            neg     ax                   ; ax = screen x of that column
            mov     cx, DRAW_COLS        ; columns still to draw
.column_loop:
            cmp     si, LEVEL_COLS       ; past the end of the map: skip
            jae     .next_column
            mov     bx, 1                ; row 0 is the HUD, start at row 1
.row_loop:
            mov     di, bx               ; di = row*60 + column
            shl     di, 6
            mov     dx, bx
            shl     dx, 2
            sub     di, dx
            add     di, si
            mov     dl, [level_work + di]
            xor     dh, dh               ; dx = tile code
            push    ax                   ; x
            mov     di, bx
            shl     di, TILE_SHIFT       ; y = row * 16
            push    di
            push    dx                   ; code
            call    DrawTile
            inc     bx
            cmp     bx, LEVEL_ROWS
            jb      .row_loop
.next_column:
            add     ax, TILE_SIZE        ; next column is 16 px to the right
            inc     si
            dec     cx
            jnz     .column_loop
            popa
            ret

; ---------------------------------------------------------------------
; ResetMap
; Purpose : restore level_work from level_orig (REP MOVSB, 720 bytes).
; Inputs  : none
; Outputs : none
; Changes : nothing
; ---------------------------------------------------------------------
ResetMap:
            push    cx
            push    si
            push    di
            push    es
            cld

            push    ds                   ; ES = DS (both in our segment)
            pop     es
            mov     si, level_orig
            mov     di, level_work
            mov     cx, LEVEL_SIZE
            rep movsb

            pop     es
            pop     di
            pop     si
            pop     cx
            ret
