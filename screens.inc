; =====================================================================
; screens.inc - title, pause, life lost, level complete, game over, HUD
; Layout uses absolute pixel positions; characters are 8 px wide.
; =====================================================================

; Small helpers so the screen code reads like a list of drawing steps.
%macro FILLRECT 5
            push    word %1
            push    word %2
            push    word %3
            push    word %4
            push    word %5
            call    FillRect
%endmacro
%macro BORDER 5
            push    word %1
            push    word %2
            push    word %3
            push    word %4
            push    word %5
            call    DrawBorder
%endmacro
%macro PUTTEXT 4
            push    word %1
            push    word %2
            push    word %3
            push    word %4
            call    DrawString
%endmacro
%macro PUTNUM 5
            push    word %1
            push    word %2
            push    word %3
            push    word %4
            push    word %5
            call    DrawNumber
%endmacro

; ---------------------------------------------------------------------
; DrawBorder(x, y, w, h, color)
; Purpose : draw a 2 px thick rectangle outline.
; Inputs  : [bp+12]=x [bp+10]=y [bp+8]=w [bp+6]=h [bp+4]=color
; Outputs : none
; Changes : nothing (ret 10)
; ---------------------------------------------------------------------
DrawBorder:
            push    bp
            mov     bp, sp
            push    ax

            push    word [bp+12]         ; top edge
            push    word [bp+10]
            push    word [bp+8]
            push    word 2
            push    word [bp+4]
            call    FillRect

            push    word [bp+12]         ; bottom edge
            mov     ax, [bp+10]
            add     ax, [bp+6]
            sub     ax, 2
            push    ax
            push    word [bp+8]
            push    word 2
            push    word [bp+4]
            call    FillRect

            push    word [bp+12]         ; left edge
            push    word [bp+10]
            push    word 2
            push    word [bp+6]
            push    word [bp+4]
            call    FillRect

            mov     ax, [bp+12]          ; right edge
            add     ax, [bp+8]
            sub     ax, 2
            push    ax
            push    word [bp+10]
            push    word 2
            push    word [bp+6]
            push    word [bp+4]
            call    FillRect

            pop     ax
            pop     bp
            ret     10

; ---------------------------------------------------------------------
; DrawHUDFrame
; Purpose : paint the black strips: top 16 px (HUD) and bottom 8 px.
;           Call it after DrawMap and before drawing HUD contents.
; Inputs  : none
; Outputs : none
; Changes : nothing
; ---------------------------------------------------------------------
DrawHUDFrame:
            FILLRECT 0, 0, SCREEN_W, HUD_HEIGHT, COL_BLACK
            FILLRECT 0, BOTTOM_STRIP_Y, SCREEN_W, BOTTOM_STRIP_H, COL_BLACK
            ret

; ---------------------------------------------------------------------
; DrawTitle(selection)
; Purpose : title screen. selection 0 = START GAME, 1 = EXIT; a '>'
;           marker is drawn beside the chosen entry.
; Inputs  : [bp+4]=selection
; Outputs : none
; Changes : nothing (ret 2)
; ---------------------------------------------------------------------
DrawTitle:
            push    bp
            mov     bp, sp
            push    ax
            push    cx

            FILLRECT 0, 0, SCREEN_W, SCREEN_H, COL_BLACK
            BORDER   4, 4, 312, 192, COL_YELLOW
            BORDER   10, 10, 300, 180, COL_RED
            FILLRECT 88, 44, 144, 2, COL_YELLOW  ; line above title
            PUTTEXT  88, 52, str_title, COL_YELLOW
            FILLRECT 88, 66, 144, 2, COL_YELLOW  ; line below title
            PUTTEXT  120, 110, str_start, COL_WHITE
            PUTTEXT  120, 130, str_exit, COL_WHITE
            PUTTEXT  64, 170, str_team, COL_LIGHT_GREEN

            mov     ax, [bp+4]           ; marker y = 110 + selection * 20
            mov     cx, ax
            shl     ax, 4
            shl     cx, 2
            add     ax, cx
            add     ax, 110
            push    word 104             ; DrawChar(104, y, '>', yellow)
            push    ax
            push    word '>'
            push    word COL_YELLOW
            call    DrawChar

            pop     cx
            pop     ax
            pop     bp
            ret     2

; ---------------------------------------------------------------------
; DrawPause
; Purpose : "PAUSED" box on top of the frozen picture (nothing erased
;           outside the box).
; Inputs  : none
; Outputs : none
; Changes : nothing
; ---------------------------------------------------------------------
DrawPause:
            FILLRECT 72, 72, 176, 56, COL_BLACK
            BORDER   72, 72, 176, 56, COL_WHITE
            PUTTEXT  136, 84, str_paused, COL_YELLOW
            PUTTEXT  92, 104, str_resume, COL_WHITE
            ret

; ---------------------------------------------------------------------
; DrawLifeLost(lives)
; Purpose : "WORLD 1-1", Mario icon, "X lives", "LIVES LEFT".
; Inputs  : [bp+4]=lives
; Outputs : none
; Changes : nothing (ret 2)
; ---------------------------------------------------------------------
DrawLifeLost:
            push    bp
            mov     bp, sp

            FILLRECT 0, 0, SCREEN_W, SCREEN_H, COL_BLACK
            BORDER   40, 40, 240, 120, COL_LIGHT_RED
            PUTTEXT  124, 60, str_world, COL_WHITE

            push    word 116             ; Mario icon
            push    word 84
            push    word spr_mario_stand
            push    word 0
            call    DrawSprite

            PUTTEXT  140, 88, str_times, COL_WHITE
            push    word 156             ; number of lives left
            push    word 88
            push    word [bp+4]
            push    word 1
            push    word COL_YELLOW
            call    DrawNumber
            PUTTEXT  120, 116, str_lives_left, COL_WHITE

            pop     bp
            ret     2

; ---------------------------------------------------------------------
; DrawLevelComplete(score, coins, time_left, bonus, final)
; Purpose : end-of-level summary with a continue prompt.
; Inputs  : [bp+12]=score [bp+10]=coins [bp+8]=time_left
;           [bp+6]=bonus [bp+4]=final score
; Outputs : none
; Changes : nothing (ret 10)
; ---------------------------------------------------------------------
DrawLevelComplete:
            push    bp
            mov     bp, sp

            FILLRECT 0, 0, SCREEN_W, SCREEN_H, COL_BLACK
            BORDER   16, 16, 288, 168, COL_YELLOW
            PUTTEXT  100, 28, str_level_complete, COL_YELLOW

            PUTTEXT  48, 60, str_score, COL_WHITE
            PUTNUM   224, 60, [bp+12], 5, COL_WHITE
            PUTTEXT  48, 76, str_coins, COL_WHITE
            PUTNUM   224, 76, [bp+10], 2, COL_WHITE
            PUTTEXT  48, 92, str_time_left, COL_WHITE
            PUTNUM   224, 92, [bp+8], 3, COL_WHITE
            PUTTEXT  48, 108, str_bonus, COL_WHITE
            PUTNUM   224, 108, [bp+6], 5, COL_WHITE
            FILLRECT 48, 124, 216, 2, COL_YELLOW
            PUTTEXT  48, 132, str_final, COL_YELLOW
            PUTNUM   224, 132, [bp+4], 5, COL_YELLOW

            PUTTEXT  68, 164, str_press_enter, COL_LIGHT_GREEN

            pop     bp
            ret     10

; ---------------------------------------------------------------------
; DrawGameOver(score, coins)
; Purpose : game over summary.
; Inputs  : [bp+6]=score  [bp+4]=coins
; Outputs : none
; Changes : nothing (ret 4)
; ---------------------------------------------------------------------
DrawGameOver:
            push    bp
            mov     bp, sp

            FILLRECT 0, 0, SCREEN_W, SCREEN_H, COL_BLACK
            BORDER   40, 40, 240, 120, COL_RED
            PUTTEXT  124, 56, str_game_over, COL_LIGHT_RED
            PUTTEXT  88, 92, str_score, COL_WHITE
            PUTNUM   200, 92, [bp+6], 5, COL_WHITE
            PUTTEXT  88, 112, str_coins, COL_WHITE
            PUTNUM   200, 112, [bp+4], 2, COL_WHITE
            PUTTEXT  116, 140, str_press_enter_s, COL_YELLOW

            pop     bp
            ret     4
