; =====================================================================
; mario.asm - Super Mario Engine (EL2003 COAL project), .COM program
; Build : nasm -f bin mario.asm -o MARIO.COM
; Layout: org, constants, jump over code, procedures, main, data last
; =====================================================================
            org     100h

%include "consts.inc"

            jmp     main                 ; skip over all procedure code

%include "video.inc"
%include "buffer.inc"
%include "sprites.inc"
%include "map.inc"
%include "camera.inc"
%include "text.inc"
%include "screens.inc"
%include "keyboard.inc"
%include "timer.inc"
%include "game.inc"                          ; Member 2: game logic (was gamestub.inc)

; ---------------------------------------------------------------------
; main
; Purpose : set everything up, run the game, undo everything, exit.
; Inputs  : none (DOS entry, CS=DS=ES=SS)
; Outputs : returns to DOS
; Changes : everything (program end)
; ---------------------------------------------------------------------
main:
            cld                          ; string instructions count upward
            call    AllocFrameBuffer     ; second segment for the frame buffer
            jnc     .memory_ok
            mov     dx, msg_no_memory    ; allocation failed: say so and quit
            mov     ah, 09h
            int     21h
            mov     ax, 4C01h            ; nothing installed yet, safe to exit
            int     21h

.memory_ok:
            call    ResetMap             ; copy pristine level into level_work
            call    InstallKbd           ; hook keyboard interrupt (INT 09h)
            call    InstallTimer         ; hook timer interrupt (INT 08h)
            call    SetMode13            ; switch to 320x200x256

            call    GameLoop             ; Member 2's game runs until it returns

; The one and only exit path: always undo hooks, then leave graphics mode.
.exit:
            call    RestoreTimer
            call    RestoreKbd
            call    SetTextMode
            mov     ax, 4C00h            ; terminate, DOS frees our memory
            int     21h

; Data goes last so that code never runs into it.
%include "data.inc"
%include "mapdata.inc"
%include "sprdata.inc"
%include "fontdata.inc"
