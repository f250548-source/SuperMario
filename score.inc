; =====================================================================
; score.inc - Member 2: Module 13 (score, coins, lives, health, HUD).
; Score is 32-bit (score_lo, score_hi) added with ADD/ADC and clamped
; to 999,999. Coins, lives, HP and time are plain binary numbers.
; =====================================================================

; ---------------------------------------------------------------------
; AddScore(points)
; Purpose : the one scoring procedure: 32-bit add (ADD low word, ADC
;           high word), then clamp to 999,999 (000F:423Fh).
; Inputs  : [bp+4]=points (0..65535)
; Outputs : score_lo, score_hi
; Changes : nothing (ret 2)
; ---------------------------------------------------------------------
AddScore:
            push    bp
            mov     bp, sp
            push    ax

            mov     ax, [bp+4]
            add     [score_lo], ax       ; low word, may set carry
            adc     word [score_hi], 0   ; carry into the high word
            cmp     word [score_hi], SCORE_MAX_HI
            jb      .ok
            ja      .clamp
            cmp     word [score_lo], SCORE_MAX_LO
            jbe     .ok
.clamp:
            mov     word [score_hi], SCORE_MAX_HI
            mov     word [score_lo], SCORE_MAX_LO
.ok:
            pop     ax
            pop     bp
            ret     2

; ---------------------------------------------------------------------
; AddLife
; Purpose : one extra life, never more than 99.
; Inputs  : none
; Outputs : lives
; Changes : nothing
; ---------------------------------------------------------------------
AddLife:
            cmp     word [lives], LIVES_LIMIT
            jae     .done
            inc     word [lives]
.done:
            ret

; ---------------------------------------------------------------------
; AddCoin
; Purpose : coins + 1; at 100 the counter goes back to 0 and Mario gets
;           a life (no points for that).
; Inputs  : none
; Outputs : coins, lives
; Changes : nothing
; ---------------------------------------------------------------------
AddCoin:
            inc     word [coins]
            cmp     word [coins], COINS_FOR_LIFE
            jb      .done
            mov     word [coins], 0
            call    AddLife
.done:
            ret

; ---------------------------------------------------------------------
; StompPoints
; Purpose : one more stomp without landing: chain + 1, points from the
;           word table stomp_points (100 .. 8000). From the 11th stomp
;           on every stomp gives +1 life instead (the counter stops at
;           11, so the table is never indexed past its end).
; Inputs  : none
; Outputs : hero_stomp_chain, score or lives
; Changes : nothing
; ---------------------------------------------------------------------
StompPoints:
            push    bx

            mov     bl, [hero_stomp_chain]
            cmp     bl, STOMP_TABLE_LEN
            jae     .one_up
            inc     bl
            mov     [hero_stomp_chain], bl
            xor     bh, bh
            dec     bx                   ; stomp n -> entry n-1
            shl     bx, 1                ; word table
            push    word [stomp_points + bx]
            call    AddScore
            jmp     .done
.one_up:
            mov     byte [hero_stomp_chain], STOMP_TABLE_LEN + 1
            call    AddLife
.done:
            pop     bx
            ret

; ---------------------------------------------------------------------
; GrowHero
; Purpose : mushroom: Small -> Super (HP 2), keeping the feet fixed
;           (y_top = y_feet - 32). Already Super: nothing (points only).
; Inputs  : none
; Outputs : hero_form, hero_hp, hero_h, hero_y
; Changes : nothing
; ---------------------------------------------------------------------
GrowHero:
            cmp     byte [hero_form], FORM_SUPER
            je      .done
            mov     byte [hero_form], FORM_SUPER
            mov     byte [hero_hp], 2
            mov     word [hero_h], SPRITE_H_TALL
            sub     word [hero_y], SPRITE_H_TALL - SPRITE_H
.done:
            ret

; ---------------------------------------------------------------------
; ShrinkHero
; Purpose : Super -> Small (HP 1), keeping the feet fixed
;           (y_top = y_feet - 16).
; Inputs  : none
; Outputs : hero_form, hero_hp, hero_h, hero_y
; Changes : nothing
; ---------------------------------------------------------------------
ShrinkHero:
            cmp     byte [hero_form], FORM_SMALL
            je      .done
            mov     byte [hero_form], FORM_SMALL
            mov     byte [hero_hp], 1
            mov     word [hero_h], SPRITE_H
            add     word [hero_y], SPRITE_H_TALL - SPRITE_H
.done:
            ret

; ---------------------------------------------------------------------
; HurtHero
; Purpose : Mario touched an enemy without stomping it (Section 5.7).
;           invulnerable : nothing
;           HP 2         : becomes Small at once, 2 s invulnerability
;           HP 1         : dies (death_pending; step 7 starts DYING)
; Inputs  : none
; Outputs : hero data, death_pending
; Changes : nothing
; ---------------------------------------------------------------------
HurtHero:
            cmp     byte [hero_invuln], 0
            jne     .done
            cmp     byte [hero_hp], 2
            jb      .dies
            call    ShrinkHero
            mov     byte [hero_invuln], INVULN_FRAMES
            jmp     .done
.dies:
            mov     byte [death_pending], DEATH_HIT
.done:
            ret

; ---------------------------------------------------------------------
; UpdateClock
; Purpose : step 6: the timer ISR raises one_second_flag once per real
;           second; each time, the level clock goes down by one. The
;           invulnerability timer goes down every frame.
; Inputs  : none
; Outputs : clock, one_second_flag, hero_invuln
; Changes : nothing
; ---------------------------------------------------------------------
UpdateClock:
            cmp     byte [one_second_flag], 0
            je      .invuln
            mov     byte [one_second_flag], 0
            cmp     word [clock], 0
            je      .invuln
            dec     word [clock]
.invuln:
            cmp     byte [hero_invuln], 0
            je      .done
            dec     byte [hero_invuln]
.done:
            ret

; ---------------------------------------------------------------------
; DrawNumber32(x, y, low, high, digits, color)
; Purpose : draw a 32-bit unsigned number, zero padded. Decimal digits
;           come from repeated division by 10: the high word is divided
;           first, its remainder goes in front of the low word for the
;           second DIV. Remainders are pushed (last digit first) and
;           popped (first digit first) for drawing.
; Inputs  : [bp+14]=x [bp+12]=y [bp+10]=low word [bp+8]=high word
;           [bp+6]=digits [bp+4]=color
; Outputs : none
; Changes : nothing (ret 12)
; ---------------------------------------------------------------------
DrawNumber32:
            push    bp
            mov     bp, sp
            pusha

            mov     cx, [bp+6]           ; digit count
            jcxz    .done
            mov     si, [bp+8]           ; si:di = number
            mov     di, [bp+10]
            mov     bx, 10
.split:
            xor     dx, dx
            mov     ax, si               ; high / 10
            div     bx
            mov     si, ax
            mov     ax, di               ; (remainder:low) / 10
            div     bx
            mov     di, ax
            push    dx                   ; dx = next digit
            loop    .split

            mov     cx, [bp+6]
            mov     si, [bp+14]          ; si = current x
.print:
            pop     ax                   ; most significant digit first
            add     al, '0'
            xor     ah, ah
            push    si                   ; DrawChar(x, y, digit, color)
            push    word [bp+12]
            push    ax
            push    word [bp+4]
            call    DrawChar
            add     si, FONT_W
            loop    .print
.done:
            popa
            pop     bp
            ret     12

; ---------------------------------------------------------------------
; DrawHUD
; Purpose : put the HUD (top 16 rows, not scrolled) into the frame
;           buffer. Drawing text pixel by pixel is slow, so the HUD is
;           kept ready in hud_cache and only redrawn (RenderHUD) when
;           score, coins, time, lives or HP changed. Every frame the
;           16 rows are copied into the frame buffer with REP MOVSW.
; Inputs  : none
; Outputs : frame buffer rows 0-15
; Changes : nothing
; ---------------------------------------------------------------------
DrawHUD:
            push    ax
            push    cx
            push    si
            push    di
            push    es
            cld

            call    HUDChanged
            test    ax, ax
            jz      .copy
            call    RenderHUD            ; values changed: redraw the cache
.copy:
            mov     ax, [buffer_seg]     ; ES:DI = frame buffer row 0
            mov     es, ax
            xor     di, di
            mov     si, hud_cache        ; DS:SI = cached HUD
            mov     cx, HUD_COPY_WORDS
            rep movsw

            pop     es
            pop     di
            pop     si
            pop     cx
            pop     ax
            ret

; ---------------------------------------------------------------------
; HUDChanged
; Purpose : compare the HUD values with the ones last drawn into
;           hud_cache (hud_last_*). The hud_last_* start as FFFFh, so
;           the first call always reports a change.
; Inputs  : none
; Outputs : AX = 1 if the HUD must be redrawn, else 0
; Changes : AX
; ---------------------------------------------------------------------
HUDChanged:
            mov     ax, [score_lo]
            cmp     ax, [hud_last_score_lo]
            jne     .yes
            mov     ax, [score_hi]
            cmp     ax, [hud_last_score_hi]
            jne     .yes
            mov     ax, [coins]
            cmp     ax, [hud_last_coins]
            jne     .yes
            mov     ax, [clock]
            cmp     ax, [hud_last_clock]
            jne     .yes
            mov     ax, [lives]
            cmp     ax, [hud_last_lives]
            jne     .yes
            mov     al, [hero_hp]
            cmp     al, [hud_last_hp]
            jne     .yes
            xor     ax, ax
            ret
.yes:
            mov     ax, 1
            ret

; ---------------------------------------------------------------------
; RenderHUD
; Purpose : draw the HUD into hud_cache with Member 1's routines:
;           draw_seg is pointed at the cache for the time being (the
;           cache is paragraph aligned, so offset y*320+x of that
;           segment is inside hud_cache), then set back.
;           Black strip, SCORE 6 digits, COINS x 2 digits, WORLD 1-1,
;           TIME 3 digits, LIVES x number, two hearts for health.
;           Labels on row y=0, values on row y=8.
; Inputs  : none (score, coins, clock, lives, hero_hp)
; Outputs : hud_cache, hud_last_*
; Changes : nothing
; ---------------------------------------------------------------------
RenderHUD:
            push    ax
            push    bx
            push    word [draw_seg]      ; remember the normal target

            mov     ax, hud_cache        ; segment whose offset 0 is
            shr     ax, 4                ; the start of hud_cache
            mov     bx, ds
            add     ax, bx
            mov     [draw_seg], ax

            FILLRECT 0, 0, SCREEN_W, HUD_HEIGHT, COL_BLACK
            PUTTEXT  HUD_SCORE_X, 0, str_score, COL_WHITE
            push    word HUD_SCORE_X
            push    word 8
            push    word [score_lo]
            push    word [score_hi]
            push    word 6
            push    word COL_YELLOW
            call    DrawNumber32

            PUTTEXT  HUD_COINS_X, 0, str_coins, COL_WHITE
            PUTTEXT  HUD_COINS_X, 8, str_times, COL_YELLOW
            PUTNUM   HUD_COINS_X + FONT_W, 8, [coins], 2, COL_YELLOW

            PUTTEXT  HUD_WORLD_X, 0, str_hud_world, COL_WHITE
            PUTTEXT  HUD_WORLD_X + FONT_W, 8, str_hud_level, COL_YELLOW

            PUTTEXT  HUD_TIME_X, 0, str_hud_time, COL_WHITE
            PUTNUM   HUD_TIME_X, 8, [clock], 3, COL_YELLOW

            PUTTEXT  HUD_LIVES_X, 0, str_hud_lives, COL_WHITE
            PUTTEXT  HUD_LIVES_X, 8, str_times, COL_YELLOW
            mov     ax, 1                ; 1 digit below 10, else 2
            cmp     word [lives], 10
            jb      .digits_ok
            mov     ax, 2
.digits_ok:
            push    word HUD_LIVES_X + FONT_W
            push    word 8
            push    word [lives]
            push    ax
            push    word COL_YELLOW
            call    DrawNumber

            push    word HUD_HEART_X     ; first heart: always full
            push    word HUD_HEART_Y
            push    word 1
            call    DrawHeart
            xor     ax, ax               ; second heart: full only at HP 2
            cmp     byte [hero_hp], 2
            jb      .second
            mov     ax, 1
.second:
            push    word HUD_HEART_X + HUD_HEART_GAP
            push    word HUD_HEART_Y
            push    ax
            call    DrawHeart

            mov     ax, [score_lo]       ; remember what the cache shows
            mov     [hud_last_score_lo], ax
            mov     ax, [score_hi]
            mov     [hud_last_score_hi], ax
            mov     ax, [coins]
            mov     [hud_last_coins], ax
            mov     ax, [clock]
            mov     [hud_last_clock], ax
            mov     ax, [lives]
            mov     [hud_last_lives], ax
            mov     al, [hero_hp]
            mov     [hud_last_hp], al

            pop     word [draw_seg]      ; drawing goes to the buffer again
            pop     bx
            pop     ax
            ret
