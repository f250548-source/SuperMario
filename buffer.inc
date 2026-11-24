; =====================================================================
; buffer.inc - off-screen frame buffer in its own segment
; =====================================================================

; ---------------------------------------------------------------------
; AllocFrameBuffer
; Purpose : shrink our DOS memory block (INT 21h AH=4Ah), then allocate
;           4000 paragraphs (AH=48h) for the frame buffer.
; Inputs  : none
; Outputs : CF=0 success: buffer_seg and draw_seg = new segment
;           CF=1 failure: nothing changed
; Changes : flags only
; ---------------------------------------------------------------------
AllocFrameBuffer:
            push    ax
            push    bx
            push    es

            mov     ax, cs               ; block to resize = our program block
            mov     es, ax
            mov     bx, PROGRAM_PARAS    ; keep 64 KB for code, data, stack
            mov     ah, 4Ah
            int     21h
            jc      .failed

            mov     bx, BUFFER_PARAS     ; ask DOS for 4000 paragraphs
            mov     ah, 48h
            int     21h                  ; AX = new segment when CF=0
            jc      .failed

            mov     [buffer_seg], ax     ; remember the segment
            mov     [draw_seg], ax       ; normal drawing goes off-screen
            clc
            jmp     .out
.failed:
            stc
.out:
            pop     es                   ; POP does not touch the flags
            pop     bx
            pop     ax
            ret

; ---------------------------------------------------------------------
; ClearBuffer
; Purpose : fill the whole frame buffer with the sky colour (REP STOSW).
; Inputs  : none (uses buffer_seg)
; Outputs : none
; Changes : nothing
; ---------------------------------------------------------------------
ClearBuffer:
            push    ax
            push    cx
            push    di
            push    es
            cld

            mov     ax, [buffer_seg]
            mov     es, ax
            xor     di, di
            mov     ax, CLEAR_WORD       ; two pixels per word
            mov     cx, BUFFER_WORDS     ; 32000 words = 64000 bytes
            rep stosw

            pop     es
            pop     di
            pop     cx
            pop     ax
            ret

; ---------------------------------------------------------------------
; PresentBuffer
; Purpose : copy the frame buffer to video memory (REP MOVSW).
; Inputs  : none
; Outputs : none
; Changes : nothing (DS is restored)
; ---------------------------------------------------------------------
PresentBuffer:
            push    ax
            push    cx
            push    si
            push    di
            push    ds
            push    es
            cld

            mov     ax, VIDEO_SEG        ; destination = screen
            mov     es, ax
            mov     ax, [buffer_seg]     ; read variable BEFORE changing DS
            mov     ds, ax               ; source = frame buffer
            xor     si, si
            xor     di, di
            mov     cx, BUFFER_WORDS
            rep movsw

            pop     es
            pop     ds                   ; our data segment is back
            pop     di
            pop     si
            pop     cx
            pop     ax
            ret
