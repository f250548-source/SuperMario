; =====================================================================
; fontdata.inc - 8x8 font. One byte per row, bit 7 = leftmost pixel.
; Glyph number = position of the character in font_char_list.
; Glyph shapes are simple hand-drawn blocks; redraw any you like.
; =====================================================================
font_char_list      db ' !-:>0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ'

font_glyphs:
            db 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h  ; space
            db 18h, 18h, 18h, 18h, 18h, 00h, 18h, 00h  ; !
            db 00h, 00h, 00h, 7Eh, 00h, 00h, 00h, 00h  ; -
            db 00h, 18h, 18h, 00h, 00h, 18h, 18h, 00h  ; :
            db 60h, 30h, 18h, 0Ch, 18h, 30h, 60h, 00h  ; >
            db 3Ch, 66h, 6Eh, 76h, 66h, 66h, 3Ch, 00h  ; 0
            db 18h, 38h, 18h, 18h, 18h, 18h, 7Eh, 00h  ; 1
            db 3Ch, 66h, 06h, 0Ch, 18h, 30h, 7Eh, 00h  ; 2
            db 3Ch, 66h, 06h, 1Ch, 06h, 66h, 3Ch, 00h  ; 3
            db 0Ch, 1Ch, 3Ch, 6Ch, 7Eh, 0Ch, 0Ch, 00h  ; 4
            db 7Eh, 60h, 7Ch, 06h, 06h, 66h, 3Ch, 00h  ; 5
            db 1Ch, 30h, 60h, 7Ch, 66h, 66h, 3Ch, 00h  ; 6
            db 7Eh, 06h, 0Ch, 18h, 30h, 30h, 30h, 00h  ; 7
            db 3Ch, 66h, 66h, 3Ch, 66h, 66h, 3Ch, 00h  ; 8
            db 3Ch, 66h, 66h, 3Eh, 06h, 0Ch, 38h, 00h  ; 9
            db 18h, 3Ch, 66h, 66h, 7Eh, 66h, 66h, 00h  ; A
            db 7Ch, 66h, 66h, 7Ch, 66h, 66h, 7Ch, 00h  ; B
            db 3Ch, 66h, 60h, 60h, 60h, 66h, 3Ch, 00h  ; C
            db 78h, 6Ch, 66h, 66h, 66h, 6Ch, 78h, 00h  ; D
            db 7Eh, 60h, 60h, 78h, 60h, 60h, 7Eh, 00h  ; E
            db 7Eh, 60h, 60h, 78h, 60h, 60h, 60h, 00h  ; F
            db 3Ch, 66h, 60h, 6Eh, 66h, 66h, 3Ch, 00h  ; G
            db 66h, 66h, 66h, 7Eh, 66h, 66h, 66h, 00h  ; H
            db 3Ch, 18h, 18h, 18h, 18h, 18h, 3Ch, 00h  ; I
            db 1Eh, 0Ch, 0Ch, 0Ch, 0Ch, 6Ch, 38h, 00h  ; J
            db 66h, 6Ch, 78h, 70h, 78h, 6Ch, 66h, 00h  ; K
            db 60h, 60h, 60h, 60h, 60h, 60h, 7Eh, 00h  ; L
            db 63h, 77h, 7Fh, 6Bh, 63h, 63h, 63h, 00h  ; M
            db 66h, 76h, 7Eh, 7Eh, 6Eh, 66h, 66h, 00h  ; N
            db 3Ch, 66h, 66h, 66h, 66h, 66h, 3Ch, 00h  ; O
            db 7Ch, 66h, 66h, 7Ch, 60h, 60h, 60h, 00h  ; P
            db 3Ch, 66h, 66h, 66h, 6Eh, 3Ch, 0Eh, 00h  ; Q
            db 7Ch, 66h, 66h, 7Ch, 6Ch, 66h, 66h, 00h  ; R
            db 3Ch, 66h, 60h, 3Ch, 06h, 66h, 3Ch, 00h  ; S
            db 7Eh, 18h, 18h, 18h, 18h, 18h, 18h, 00h  ; T
            db 66h, 66h, 66h, 66h, 66h, 66h, 3Ch, 00h  ; U
            db 66h, 66h, 66h, 66h, 66h, 3Ch, 18h, 00h  ; V
            db 63h, 63h, 63h, 6Bh, 7Fh, 77h, 63h, 00h  ; W
            db 66h, 66h, 3Ch, 18h, 3Ch, 66h, 66h, 00h  ; X
            db 66h, 66h, 66h, 3Ch, 18h, 18h, 18h, 00h  ; Y
            db 7Eh, 06h, 0Ch, 18h, 30h, 60h, 7Eh, 00h  ; Z
