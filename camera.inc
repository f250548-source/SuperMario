; =====================================================================
; camera.inc - horizontal scrolling camera
; Coordinates are signed whole pixels, so signed jumps (jg/jl) are used.
; =====================================================================

; ---------------------------------------------------------------------
; UpdateCamera(hero_world_x)
; Purpose : scroll right when the hero passes screen x 160. The camera
;           never moves left and is clamped to 640. The left edge of the
;           view acts as a wall.
; Inputs  : [bp+4]=hero_world_x (whole pixels)
; Outputs : AX = corrected hero x (>= camera_x)
; Changes : AX (camera_x is updated)
; ---------------------------------------------------------------------
UpdateCamera:
            push    bp
            mov     bp, sp
            push    bx

            mov     ax, [bp+4]           ; hero world x
            mov     bx, ax
            sub     bx, [camera_x]       ; bx = hero screen x
            cmp     bx, CAMERA_TRIGGER_X
            jle     .wall_check          ; not past the trigger line
            mov     bx, ax
            sub     bx, CAMERA_TRIGGER_X  ; camera that keeps hero at 160
            cmp     bx, MAX_CAMERA_X
            jle     .clamped
            mov     bx, MAX_CAMERA_X     ; do not scroll past the level end
.clamped:
            cmp     bx, [camera_x]       ; never scroll backwards
            jle     .wall_check
            mov     [camera_x], bx
.wall_check:
            cmp     ax, [camera_x]       ; hero left of the view?
            jge     .out
            mov     ax, [camera_x]       ; push him back to the left wall
.out:
            pop     bx
            pop     bp
            ret     2

; ---------------------------------------------------------------------
; WorldToScreen(world_x)
; Purpose : convert a world x to a screen x.
; Inputs  : [bp+4]=world_x
; Outputs : AX = world_x - camera_x
; Changes : AX
; ---------------------------------------------------------------------
WorldToScreen:
            push    bp
            mov     bp, sp
            mov     ax, [bp+4]
            sub     ax, [camera_x]
            pop     bp
            ret     2

; ---------------------------------------------------------------------
; IsOnScreen(world_x)
; Purpose : is a 16 px wide object at world_x at least partly visible?
; Inputs  : [bp+4]=world_x
; Outputs : AX = 1 if -16 < screen_x < 320, else 0
; Changes : AX
; ---------------------------------------------------------------------
IsOnScreen:
            push    bp
            mov     bp, sp
            mov     ax, [bp+4]
            sub     ax, [camera_x]       ; ax = screen x
            cmp     ax, -16
            jle     .no
            cmp     ax, SCREEN_W
            jge     .no
            mov     ax, 1
            jmp     .out
.no:
            xor     ax, ax
.out:
            pop     bp
            ret     2
