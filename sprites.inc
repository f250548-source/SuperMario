; =====================================================================
; sprites.inc - DrawSprite and DrawSpriteCamera
; =====================================================================

; ---------------------------------------------------------------------
; DrawSprite(x, y, addr, flags)
; Purpose : draw a 16x16 (or 16x32) byte bitmap into draw_seg.
;           Colour 0 is transparent. Signed x/y, clipped on all four
;           screen edges, so nothing outside the screen is written.
;           flags: bit0 mirror, bit1 skip when blink_toggle=1, bit2 tall
; Inputs  : [bp+10]=x  [bp+8]=y  [bp+6]=addr  [bp+4]=flags
; Locals  : [bp-2]=first visible column   [bp-4]=visible column count
;           [bp-6]=first visible row      [bp-8]=visible row count
; Outputs : none
; Changes : nothing (ret 8 removes the four parameters)
; ---------------------------------------------------------------------
DrawSprite:
            push    bp
            mov     bp, sp
            sub     sp, 8                ; room for the four locals
            pusha
            push    es

            mov     ax, [bp+4]           ; ax = flags
            test    al, FLAG_BLINK       ; blinking sprites vanish on odd phase
            jz      .not_blinking
            cmp     byte [blink_toggle], 1
            jne     .go1
            jmp     .done
.go1:
.not_blinking:
            mov     dx, SPRITE_H         ; dx = total rows of the bitmap
            test    al, FLAG_TALL
            jz      .height_ok
            mov     dx, SPRITE_H_TALL
.height_ok:
            ; ---- clip columns ----
            mov     si, [bp+10]          ; si = x
            xor     cx, cx               ; cx = first visible column
            test    si, si
            jge     .left_ok
            mov     cx, si               ; x < 0: skip -x columns on the left
            neg     cx
            cmp     cx, SPRITE_W
            jl      .go2
            jmp     .done                ; completely off the left edge
.go2:
.left_ok:
            mov     bx, SCREEN_W         ; bx = last column + 1
            sub     bx, si               ; room to the right edge = 320 - x
            cmp     bx, SPRITE_W
            jle     .right_ok
            mov     bx, SPRITE_W         ; never more than 16 columns
.right_ok:
            cmp     bx, cx
            jg      .go3
            jmp     .done                ; nothing visible horizontally
.go3:
            sub     bx, cx               ; bx = visible column count
            mov     [bp-2], cx
            mov     [bp-4], bx

            ; ---- clip rows ----
            mov     si, [bp+8]           ; si = y
            xor     di, di               ; di = first visible row
            test    si, si
            jge     .top_ok
            mov     di, si               ; y < 0: skip -y rows at the top
            neg     di
            cmp     di, dx
            jl      .go4
            jmp     .done                ; completely above the screen
.go4:
.top_ok:
            mov     ax, SCREEN_H         ; ax = last row + 1
            sub     ax, si               ; room to the bottom edge = 200 - y
            cmp     ax, dx
            jle     .bottom_ok
            mov     ax, dx               ; never more rows than the bitmap has
.bottom_ok:
            cmp     ax, di
            jg      .go5
            jmp     .done                ; nothing visible vertically
.go5:
            sub     ax, di
            mov     [bp-6], di
            mov     [bp-8], ax

            ; ---- destination offset of the first visible pixel ----
            mov     si, [bp+8]
            add     si, [bp-6]           ; clipped y (>= 0)
            mov     di, si
            shl     di, 8                ; (y<<8) + (y<<6) + x, no MUL
            shl     si, 6
            add     di, si
            mov     ax, [bp+10]
            add     ax, [bp-2]           ; clipped x (>= 0)
            add     di, ax

            ; ---- source address of the first visible pixel ----
            mov     si, [bp-6]
            shl     si, 4                ; 16 bytes per bitmap row
            add     si, [bp+6]           ; + bitmap address
            mov     bx, 1                ; bx = step through a bitmap row
            mov     ax, [bp+4]
            test    al, FLAG_MIRROR
            jz      .forward
            add     si, SPRITE_W - 1     ; mirrored: start at the right end
            sub     si, [bp-2]
            mov     bx, -1               ; and walk leftwards
            jmp     .source_ready
.forward:
            add     si, [bp-2]
.source_ready:
            mov     ax, [draw_seg]
            mov     es, ax
            mov     ah, [bp-4]           ; ah = columns per row (fits a byte)
            mov     dx, [bp-8]           ; dx = rows to draw
.row_loop:
            push    si                   ; remember the start of this row
            push    di
            mov     cl, ah               ; cl = columns left (ch is still 0)
            xor     ch, ch
.pixel_loop:
            mov     al, [si]             ; fetch bitmap pixel
            test    al, al
            jz      .skip_pixel          ; colour 0 = transparent
            mov     [es:di], al
.skip_pixel:
            add     si, bx
            inc     di
            dec     cl
            jnz     .pixel_loop
            pop     di
            pop     si
            add     si, SPRITE_W         ; next bitmap row
            add     di, SCREEN_W         ; next screen row
            dec     dx
            jnz     .row_loop
.done:
            pop     es
            popa
            mov     sp, bp               ; drop the locals
            pop     bp
            ret     8

; ---------------------------------------------------------------------
; DrawSpriteCamera(world_x, y, addr, flags)
; Purpose : like DrawSprite but takes a world x. Subtracts camera_x and
;           skips the sprite unless -16 < screen_x < 320.
; Inputs  : [bp+10]=world_x  [bp+8]=y  [bp+6]=addr  [bp+4]=flags
; Outputs : none
; Changes : nothing (ret 8)
; ---------------------------------------------------------------------
DrawSpriteCamera:
            push    bp
            mov     bp, sp
            push    ax

            mov     ax, [bp+10]
            sub     ax, [camera_x]       ; ax = screen x (signed)
            cmp     ax, -16
            jle     .done                ; fully off the left side
            cmp     ax, SCREEN_W
            jge     .done                ; fully off the right side
            push    ax                   ; DrawSprite(screen_x, y, addr, flags)
            push    word [bp+8]
            push    word [bp+6]
            push    word [bp+4]
            call    DrawSprite
.done:
            pop     ax
            pop     bp
            ret     8
