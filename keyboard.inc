; =====================================================================
; keyboard.inc - keyboard interrupt (INT 09h) and key_table
; An interrupt is the keyboard saying "a key changed, deal with it now".
; The CPU saves flags and the return address, jumps to our routine,
; and IRET goes back. Our routine must preserve every register it uses.
; =====================================================================

; ---------------------------------------------------------------------
; KbdISR
; Purpose : read the scan code from port 60h and update key_table:
;           press -> 1, release (bit 7 set) -> 0. The E0h prefix that
;           arrow keys send first is ignored. Ends with EOI to the PIC.
; Inputs  : none (hardware)
; Outputs : key_table[scan code] changed
; Changes : nothing (all registers restored, then IRET)
; ---------------------------------------------------------------------
KbdISR:
            push    ax
            push    bx
            push    ds
            push    cs                   ; DS = CS: our variables live here
            pop     ds

            in      al, KBD_PORT         ; scan code of the key event
            cmp     al, KEY_EXTENDED     ; E0h prefix: ignore this byte
            je      .send_eoi
            mov     bl, al
            and     bl, 7Fh              ; strip the release bit -> key number
            xor     bh, bh
            test    al, KEY_RELEASE_BIT
            jnz     .released
            mov     byte [key_table + bx], 1  ; key went down
            jmp     .send_eoi
.released:
            mov     byte [key_table + bx], 0  ; key came up
.send_eoi:
            mov     al, PIC_EOI          ; tell the PIC we are done
            out     PIC_COMMAND, al

            pop     ds
            pop     bx
            pop     ax
            iret

; ---------------------------------------------------------------------
; InstallKbd
; Purpose : save the old INT 09h vector (INT 21h AH=35h) and install
;           KbdISR (AH=25h).
; Inputs  : DS = our segment
; Outputs : old_kbd_off/old_kbd_seg filled
; Changes : nothing
; ---------------------------------------------------------------------
InstallKbd:
            push    ax
            push    bx
            push    dx
            push    es

            mov     ax, 3500h | KBD_INT  ; get vector -> ES:BX
            int     21h
            mov     [old_kbd_off], bx
            mov     [old_kbd_seg], es
            mov     dx, KbdISR           ; DS:DX = new handler
            mov     ax, 2500h | KBD_INT  ; set vector
            int     21h

            pop     es
            pop     dx
            pop     bx
            pop     ax
            ret

; ---------------------------------------------------------------------
; RestoreKbd
; Purpose : put the original INT 09h vector back. Does nothing if the
;           hook was never installed, so it is safe on every exit path.
; Inputs  : none
; Outputs : none
; Changes : nothing
; ---------------------------------------------------------------------
RestoreKbd:
            push    ax
            push    dx
            push    ds

            mov     dx, [old_kbd_off]    ; read both values BEFORE touching DS
            mov     ax, [old_kbd_seg]
            test    ax, ax
            jz      .done                ; never installed
            mov     ds, ax               ; DS:DX = original handler
            mov     ax, 2500h | KBD_INT
            int     21h
.done:
            pop     ds
            pop     dx
            pop     ax
            ret
