; =====================================================================
; timer.inc - timer interrupt (INT 08h, about 18.2 ticks per second)
; =====================================================================

; ---------------------------------------------------------------------
; TimerISR
; Purpose : count ticks, raise one_second_flag once per real second,
;           flip blink_toggle every 4 ticks, then chain to the old BIOS
;           handler (it keeps the DOS clock and sends the EOI itself).
; Inputs  : none (hardware)
; Outputs : tick_count, one_second_flag, blink_toggle updated
; Changes : nothing (registers restored before the far jump)
; ---------------------------------------------------------------------
TimerISR:
            push    ax
            push    ds
            push    cs                   ; DS = CS
            pop     ds

            inc     word [tick_count]
            test    byte [tick_count], 3  ; every 4th tick flip the blink
            jnz     .no_blink
            xor     byte [blink_toggle], 1
.no_blink:
            ; 18.2065 ticks make a real second, so add 100 per tick and
            ; subtract 1821 when the sum reaches it (no drift to speak of).
            add     word [tick_in_second], TICK_ACC_STEP
            cmp     word [tick_in_second], TICK_ACC_LIMIT
            jb      .chain
            sub     word [tick_in_second], TICK_ACC_LIMIT
            mov     byte [one_second_flag], 1
.chain:
            pop     ds
            pop     ax
            jmp     far [cs:old_timer_off]  ; old handler ends with IRET

; ---------------------------------------------------------------------
; InstallTimer
; Purpose : save the old INT 08h vector and install TimerISR.
; Inputs  : DS = our segment
; Outputs : old_timer_off/old_timer_seg filled
; Changes : nothing
; ---------------------------------------------------------------------
InstallTimer:
            push    ax
            push    bx
            push    dx
            push    es

            mov     ax, 3500h | TIMER_INT
            int     21h
            mov     [old_timer_off], bx
            mov     [old_timer_seg], es
            mov     dx, TimerISR
            mov     ax, 2500h | TIMER_INT
            int     21h

            pop     es
            pop     dx
            pop     bx
            pop     ax
            ret

; ---------------------------------------------------------------------
; RestoreTimer
; Purpose : put the original INT 08h vector back (safe if not installed).
; Inputs  : none
; Outputs : none
; Changes : nothing
; ---------------------------------------------------------------------
RestoreTimer:
            push    ax
            push    dx
            push    ds

            mov     dx, [old_timer_off]
            mov     ax, [old_timer_seg]
            test    ax, ax
            jz      .done
            mov     ds, ax
            mov     ax, 2500h | TIMER_INT
            int     21h
.done:
            pop     ds
            pop     dx
            pop     ax
            ret

; ---------------------------------------------------------------------
; WaitTick
; Purpose : wait until the timer ISR has advanced tick_count. This is
;           the game speed limiter (no fixed delay loops).
; Inputs  : none
; Outputs : last_tick = tick_count
; Changes : nothing
; ---------------------------------------------------------------------
WaitTick:
            push    ax
.spin:
            mov     ax, [tick_count]
            cmp     ax, [last_tick]
            je      .spin                ; interrupts keep running meanwhile
            mov     [last_tick], ax
            pop     ax
            ret
