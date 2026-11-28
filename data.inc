; =====================================================================
; data.inc - variables owned by Member 1 and all screen strings
; =====================================================================

; ---- shared contract variables (do not rename) ----
camera_x            dw 0                     ; left edge of the view in world pixels
draw_seg            dw 0                     ; segment every Draw* routine writes to
buffer_seg          dw 0                     ; segment of the off-screen frame buffer
key_table           times KEY_TABLE_SIZE db 0  ; 1 = key held, index = scan code
tick_count          dw 0                     ; timer ticks since start
last_tick           dw 0                     ; tick_count seen by WaitTick
one_second_flag     db 0                     ; set once per second, Member 2 clears
blink_toggle        db 0                     ; flips a few times per second

; ---- interrupt bookkeeping ----
; old_*_off and old_*_seg MUST stay next to each other: the timer ISR
; does "jmp far [old_timer_off]" which reads offset then segment.
old_kbd_off         dw 0
old_kbd_seg         dw 0
old_timer_off       dw 0
old_timer_seg       dw 0
tick_in_second      dw 0                     ; fractional second accumulator

; ---- messages ----
msg_no_memory       db 'Not enough memory for the frame buffer.', 13, 10, '$'

; ---- screen strings (zero terminated, font has capitals only) ----
str_title           db 'SUPER MARIO ENGINE', 0
str_start           db 'START GAME', 0
str_exit            db 'EXIT', 0
str_team            db '   BY AAYAN AND HAZIQ', 0  ; 3 spaces keep it centred at x=64
str_paused          db 'PAUSED', 0
str_resume          db 'PRESS P TO RESUME', 0
str_world           db 'WORLD 1-1', 0
str_times           db 'X', 0
str_lives_left      db 'LIVES LEFT', 0
str_level_complete  db 'LEVEL COMPLETE!', 0
str_score           db 'SCORE', 0
str_coins           db 'COINS', 0
str_time_left       db 'TIME LEFT', 0
str_bonus           db 'BONUS', 0
str_final           db 'FINAL SCORE', 0
str_press_enter     db 'PRESS ENTER TO CONTINUE', 0
str_game_over       db 'GAME OVER', 0
str_press_enter_s   db 'PRESS ENTER', 0
