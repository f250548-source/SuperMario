; =====================================================================
; video.inc - mode switching, PutPixel, FillRect, ColorTestScreen
; All drawing goes to the segment stored in draw_seg.
; =====================================================================

; ---------------------------------------------------------------------
; SetMode13
; Purpose : switch to VGA 320x200, 256 colours.
; Inputs  : none
; Outputs : none
; Changes : nothing (AX saved)
; ---------------------------------------------------------------------
SetMode13:
            push    ax
            mov     ax, 0013h            ; BIOS: set video mode 13h
            int     10h
            pop     ax
            ret

; ---------------------------------------------------------------------
; SetTextMode
; Purpose : switch back to 80x25 text mode.
; Inputs  : none
; Outputs : none
; Changes : nothing (AX saved)
; ---------------------------------------------------------------------
SetTextMode:
            push    ax
            mov     ax, 0003h            ; BIOS: set video mode 3
            int     10h
            pop     ax
            ret

; ---------------------------------------------------------------------
; PutPixel(x, y, color)
; Purpose : write one pixel into draw_seg, ignoring off-screen points.
; Inputs  : [bp+8]=x  [bp+6]=y  [bp+4]=color (low byte used)
;           push x, push y, push color, call
; Outputs : none
; Changes : nothing (ret 6 removes the three parameters)
; ---------------------------------------------------------------------
PutPixel:
            push    bp
            mov     bp, sp
            push    ax
            push    bx
            push    dx
            push    es

            mov     ax, [bp+8]           ; x
            cmp     ax, SCREEN_W         ; unsigned compare: a negative x
            jae     .done                ; looks huge, so it is rejected too
            mov     bx, [bp+6]           ; y
            cmp     bx, SCREEN_H
            jae     .done

            mov     dx, bx               ; offset = (y<<8) + (y<<6) + x
            shl     bx, 8
            shl     dx, 6
            add     bx, dx
            add     bx, ax

            mov     ax, [draw_seg]       ; target segment
            mov     es, ax
            mov     al, [bp+4]           ; colour
            mov     [es:bx], al
.done:
            pop     es
            pop     dx
            pop     bx
            pop     ax
            pop     bp
            ret     6

; ---------------------------------------------------------------------
; FillRect(x, y, w, h, color)
; Purpose : fill a rectangle, clipped on all four screen edges.
;           Colour 0 is drawn (it is a real black here).
; Inputs  : [bp+12]=x [bp+10]=y [bp+8]=w [bp+6]=h [bp+4]=color
; Outputs : none
; Changes : nothing (ret 10 removes the five parameters)
; ---------------------------------------------------------------------
FillRect:
            push    bp
            mov     bp, sp
            pusha
            push    es
            cld

            mov     ax, [bp+12]          ; ax = left edge
            mov     bx, [bp+10]          ; bx = top edge
            mov     cx, ax
            add     cx, [bp+8]           ; cx = right edge (exclusive)
            mov     dx, bx
            add     dx, [bp+6]           ; dx = bottom edge (exclusive)

            test    ax, ax               ; clip left
            jge     .left_ok
            xor     ax, ax
.left_ok:
            cmp     cx, SCREEN_W         ; clip right (signed compare)
            jle     .right_ok
            mov     cx, SCREEN_W
.right_ok:
            cmp     ax, cx               ; nothing left to draw?
            jge     .done

            test    bx, bx               ; clip top
            jge     .top_ok
            xor     bx, bx
.top_ok:
            cmp     dx, SCREEN_H         ; clip bottom
            jle     .bottom_ok
            mov     dx, SCREEN_H
.bottom_ok:
            cmp     bx, dx
            jge     .done

            sub     cx, ax               ; cx = visible width
            sub     dx, bx               ; dx = visible height

            mov     di, bx               ; di = (y<<8) + (y<<6) + x
            shl     di, 8
            mov     si, bx
            shl     si, 6
            add     di, si
            add     di, ax

            mov     ax, [draw_seg]
            mov     es, ax
            mov     al, [bp+4]           ; colour for STOSB
            mov     bx, cx               ; keep the width for every row
.row:
            mov     cx, bx
            push    di                   ; remember start of this row
            rep stosb                    ; paint one row
            pop     di
            add     di, SCREEN_W         ; next row down
            dec     dx
            jnz     .row
.done:
            pop     es
            popa
            pop     bp
            ret     10

; ---------------------------------------------------------------------
; ColorTestScreen
; Purpose : draw all 256 palette colours as a 16x16 grid of 20x12 cells
;           into draw_seg (set draw_seg = VIDEO_SEG to see it directly).
; Inputs  : none
; Outputs : none
; Changes : nothing
; ---------------------------------------------------------------------
ColorTestScreen:
            pusha
            xor     si, si               ; si = colour number 0..255
.next_cell:
            mov     bx, si               ; column = colour & 15
            and     bx, 15
            mov     cx, bx
            shl     bx, 4                ; x = column * 20 = col*16 + col*4
            shl     cx, 2
            add     bx, cx
            mov     dx, si               ; row = colour >> 4
            shr     dx, 4
            mov     cx, dx
            shl     dx, 3                ; y = row * 12 = row*8 + row*4
            shl     cx, 2
            add     dx, cx
            push    bx                   ; x
            push    dx                   ; y
            push    20                   ; w
            push    12                   ; h
            push    si                   ; colour
            call    FillRect
            inc     si
            cmp     si, 256
            jb      .next_cell
            popa
            ret
