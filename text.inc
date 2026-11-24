; =====================================================================
; text.inc - 8x8 font drawing: DrawChar, DrawString, DrawNumber, DrawHeart
; =====================================================================

; ---------------------------------------------------------------------
; DrawChar(x, y, char, color)
; Purpose : draw one 8x8 character. Lowercase is shown as capitals,
;           unknown characters are drawn as a blank.
; Inputs  : [bp+10]=x  [bp+8]=y  [bp+6]=char (low byte)  [bp+4]=color
; Outputs : none
; Changes : nothing (ret 8)
; ---------------------------------------------------------------------
DrawChar:
            push    bp
            mov     bp, sp
            pusha
            push    es
            cld

            mov     ax, [bp+6]           ; al = character
            cmp     al, 'a'              ; fold lowercase onto capitals
            jb      .case_done
            cmp     al, 'z'
            ja      .case_done
            sub     al, 32
.case_done:
            push    ds                   ; SCASB looks at ES:DI, so ES = DS
            pop     es
            mov     di, font_char_list   ; search the character list
            mov     cx, FONT_GLYPHS
            repne scasb
            jne     .done                ; not in the font: draw nothing
            sub     di, font_char_list + 1  ; di = glyph number
            shl     di, 3                ; 8 bytes per glyph
            lea     si, [font_glyphs + di]  ; si = glyph bitmap

            xor     bx, bx               ; bx = glyph row 0..7
.row_loop:
            mov     dl, [si + bx]        ; one row, bit 7 = leftmost pixel
            mov     cx, FONT_W           ; eight columns
            xor     di, di               ; di = column 0..7
.col_loop:
            test    dl, 80h
            jz      .no_pixel
            mov     ax, [bp+10]          ; PutPixel(x + col, y + row, color)
            add     ax, di
            push    ax
            mov     ax, [bp+8]
            add     ax, bx
            push    ax
            push    word [bp+4]
            call    PutPixel
.no_pixel:
            shl     dl, 1                ; next pixel of the row
            inc     di
            loop    .col_loop
            inc     bx
            cmp     bx, 8
            jb      .row_loop
.done:
            pop     es
            popa
            pop     bp
            ret     8

; ---------------------------------------------------------------------
; DrawString(x, y, addr, color)
; Purpose : draw a zero-terminated string, 8 pixels per character.
; Inputs  : [bp+10]=x  [bp+8]=y  [bp+6]=addr  [bp+4]=color
; Outputs : none
; Changes : nothing (ret 8)
; ---------------------------------------------------------------------
DrawString:
            push    bp
            mov     bp, sp
            push    ax
            push    bx
            push    si
            cld

            mov     si, [bp+6]           ; string address
            mov     bx, [bp+10]          ; current x
.next_char:
            lodsb                        ; al = next character
            test    al, al
            jz      .done                ; zero terminator
            xor     ah, ah
            push    bx                   ; DrawChar(x, y, char, color)
            push    word [bp+8]
            push    ax
            push    word [bp+4]
            call    DrawChar
            add     bx, FONT_W
            jmp     .next_char
.done:
            pop     si
            pop     bx
            pop     ax
            pop     bp
            ret     8

; ---------------------------------------------------------------------
; DrawNumber(x, y, value, digits, color)
; Purpose : draw an unsigned number, zero padded to a fixed number of
;           digits. Digits are split with repeated DIV 10, pushed on the
;           stack (least significant first) and popped (most first).
; Inputs  : [bp+12]=x [bp+10]=y [bp+8]=value [bp+6]=digits [bp+4]=color
; Outputs : none
; Changes : nothing (ret 10)
; ---------------------------------------------------------------------
DrawNumber:
            push    bp
            mov     bp, sp
            pusha

            mov     cx, [bp+6]           ; cx = number of digits
            test    cx, cx
            jz      .done
            mov     ax, [bp+8]           ; value
            mov     bx, 10
            mov     si, cx               ; remember the digit count
.split:
            xor     dx, dx               ; dx:ax / 10 -> ax = quotient
            div     bx                   ;               dx = digit
            push    dx
            loop    .split

            mov     cx, si
            mov     si, [bp+12]          ; si = current x
.print:
            pop     ax                   ; most significant digit first
            add     al, '0'
            xor     ah, ah
            push    si                   ; DrawChar(x, y, digit, color)
            push    word [bp+10]
            push    ax
            push    word [bp+4]
            call    DrawChar
            add     si, FONT_W
            loop    .print
.done:
            popa
            pop     bp
            ret     10

; ---------------------------------------------------------------------
; DrawHeart(x, y, filled)
; Purpose : draw a heart sprite, full when filled <> 0 else empty.
; Inputs  : [bp+8]=x  [bp+6]=y  [bp+4]=filled
; Outputs : none
; Changes : nothing (ret 6)
; ---------------------------------------------------------------------
DrawHeart:
            push    bp
            mov     bp, sp
            push    ax

            mov     ax, spr_heart_empty  ; assume empty
            cmp     word [bp+4], 0
            je      .draw
            mov     ax, spr_heart_full
.draw:
            push    word [bp+8]
            push    word [bp+6]
            push    ax
            push    word 0
            call    DrawSprite

            pop     ax
            pop     bp
            ret     6
