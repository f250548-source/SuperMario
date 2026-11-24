; =====================================================================
; gamestub.inc - TEMPORARY GameLoop so Member 1 can test the engine.
; Right/Left move a Mario, camera follows, ESC quits. Member 2 replaces
; this file with the real game.inc (same GameLoop entry, ends with ret).
; =====================================================================

; ---------------------------------------------------------------------
; GameLoop
; Purpose : demo loop: map, scrolling, sprite, HUD, text, hearts.
; Inputs  : none
; Outputs : returns when ESC is pressed
; Changes : nothing
; ---------------------------------------------------------------------
GameLoop:
            pusha
            mov     si, 16               ; hero world x
.frame:
            call    WaitTick             ; timer interrupt sets the pace
            cmp     byte [key_table + KEY_ESC], 1
            jne     .go1
            jmp     .quit
.go1:

            cmp     byte [key_table + KEY_RIGHT], 1
            jne     .not_right
            add     si, 2
.not_right:
            cmp     byte [key_table + KEY_LEFT], 1
            jne     .not_left
            sub     si, 2
.not_left:
            test    si, si               ; keep the hero inside the level
            jge     .low_ok
            xor     si, si
.low_ok:
            cmp     si, LEVEL_WIDTH_PX - 16
            jle     .high_ok
            mov     si, LEVEL_WIDTH_PX - 16
.high_ok:
            push    si
            call    UpdateCamera         ; may scroll, returns corrected x
            mov     si, ax

            call    ClearBuffer          ; sky
            call    DrawMap              ; tiles
            call    DrawHUDFrame         ; black strips

            push    si                   ; Mario on the ground (row 9)
            push    word HERO_STAND_ROW * TILE_SIZE
            push    word spr_mario_stand
            push    word 0
            call    DrawSpriteCamera

            push    word 8               ; three hearts, last one empty
            push    word 4
            push    word 1
            call    DrawHeart
            push    word 26
            push    word 4
            push    word 1
            call    DrawHeart
            push    word 44
            push    word 4
            push    word 0
            call    DrawHeart

            push    word 120             ; "SCORE" and a number
            push    word 4
            push    word str_score
            push    word COL_WHITE
            call    DrawString
            push    word 168
            push    word 4
            push    si
            push    word 5
            push    word COL_YELLOW
            call    DrawNumber

            call    PresentBuffer
            jmp     .frame
.quit:
            popa
            ret
