; =====================================================================
; constants.inc - all EQU constants for Super Mario Engine
; Only EQUs live here, so assembling this file emits no bytes.
; =====================================================================

; ---------------- Screen and video ----------------
VIDEO_SEG           equ 0A000h               ; VGA Mode 13h display segment
SCREEN_W            equ 320                  ; pixels per row
SCREEN_H            equ 200                  ; pixel rows
VISIBLE_H           equ 192                  ; game picture height (bottom 8 px black)
BUFFER_BYTES        equ 64000                ; 320 * 200
BUFFER_WORDS        equ 32000                ; BUFFER_BYTES / 2 for REP STOSW / MOVSW
BUFFER_PARAS        equ 4000                 ; BUFFER_BYTES / 16 for INT 21h AH=48h
PROGRAM_PARAS       equ 1000h                ; 64 KB kept for the .COM (INT 21h AH=4Ah)
HUD_HEIGHT          equ 16                   ; black strip at the top (row 0)
BOTTOM_STRIP_Y      equ 192                  ; first y of the black bottom strip
BOTTOM_STRIP_H      equ 8

; ---------------- Tiles and level ----------------
TILE_SIZE           equ 16                   ; a tile is 16x16 pixels
TILE_SHIFT          equ 4                    ; px >> 4 = tile index
LEVEL_COLS          equ 60
LEVEL_ROWS          equ 12
LEVEL_SIZE          equ 720                  ; LEVEL_COLS * LEVEL_ROWS bytes
LEVEL_WIDTH_PX      equ 960                  ; LEVEL_COLS * 16
LEVEL_HEIGHT_PX     equ 192                  ; LEVEL_ROWS * 16
VIEW_COLS           equ 20                   ; columns fully visible
DRAW_COLS           equ 21                   ; columns drawn (one extra for scrolling)
MAX_CAMERA_X        equ 640                  ; LEVEL_WIDTH_PX - SCREEN_W
CAMERA_TRIGGER_X    equ 160                  ; hero screen x where scrolling starts
GROUND_ROW          equ 10                   ; first ground row
HERO_STAND_ROW      equ 9                    ; row Mario stands on

%if LEVEL_COLS * LEVEL_ROWS != LEVEL_SIZE
    %error "LEVEL_SIZE does not match LEVEL_COLS * LEVEL_ROWS"
%endif
%if BUFFER_PARAS * 16 != BUFFER_BYTES
    %error "BUFFER_PARAS does not match BUFFER_BYTES"
%endif

; ---------------- Tile codes (shared with Member 2) ----------------
TILE_SKY            equ 0
TILE_GROUND         equ 1
TILE_BRICK          equ 2
TILE_QCOIN          equ 3                    ; ? block holding a coin
TILE_QMUSHROOM      equ 4                    ; ? block holding a mushroom
TILE_EMPTY          equ 5                    ; used-up block
TILE_PIPE_TL        equ 6
TILE_PIPE_TR        equ 7
TILE_PIPE_BL        equ 8
TILE_PIPE_BR        equ 9
TILE_STAIR          equ 10
TILE_COIN           equ 11                   ; non-solid
TILE_FLAGPOLE       equ 12                   ; non-solid

; ---------------- Keyboard scan codes ----------------
KEY_ESC             equ 01h
KEY_ENTER           equ 1Ch
KEY_P               equ 19h
KEY_LSHIFT          equ 2Ah
KEY_SPACE           equ 39h
KEY_UP              equ 48h
KEY_LEFT            equ 4Bh
KEY_RIGHT           equ 4Dh
KEY_DOWN            equ 50h
KEY_EXTENDED        equ 0E0h                 ; prefix sent before arrow keys: ignore
KEY_RELEASE_BIT     equ 80h                  ; release code = press code | 80h
KEY_TABLE_SIZE      equ 128

; ---------------- Colour indices (default VGA palette) ----------------
COL_TRANSPARENT     equ 0                    ; never drawn by sprites
COL_BLACK           equ 0
COL_BLUE            equ 1
COL_GREEN           equ 2
COL_CYAN            equ 3
COL_RED             equ 4
COL_MAGENTA         equ 5
COL_BROWN           equ 6
COL_LIGHT_GRAY      equ 7
COL_DARK_GRAY       equ 8
COL_LIGHT_BLUE      equ 9
COL_LIGHT_GREEN     equ 10
COL_LIGHT_CYAN      equ 11
COL_LIGHT_RED       equ 12
COL_PINK            equ 13
COL_YELLOW          equ 14
COL_WHITE           equ 15
COL_OUTLINE         equ 16                   ; a real black (index 0 is transparent)

CLEAR_COLOR         equ COL_LIGHT_BLUE       ; sky colour used by ClearBuffer
CLEAR_WORD          equ CLEAR_COLOR * 0101h  ; two pixels for REP STOSW

; ---------------- Sprite flag bits ----------------
FLAG_MIRROR         equ 01h                  ; bit 0: flip horizontally
FLAG_BLINK          equ 02h                  ; bit 1: skip when blink_toggle = 1
FLAG_TALL           equ 04h                  ; bit 2: 32 rows instead of 16
SPRITE_W            equ 16
SPRITE_H            equ 16
SPRITE_H_TALL       equ 32

; ---------------- Game states ----------------
STATE_TITLE         equ 0
STATE_PLAYING       equ 1
STATE_PAUSED        equ 2
STATE_LIFE_LOST     equ 3
STATE_LEVEL_DONE    equ 4
STATE_GAME_OVER     equ 5
STATE_EXIT          equ 6

; ---------------- Interrupts and timer ----------------
KBD_INT             equ 09h
TIMER_INT           equ 08h
PIC_COMMAND         equ 20h
PIC_EOI             equ 20h
KBD_PORT            equ 60h
TICK_ACC_STEP       equ 100                  ; added to the accumulator every tick
TICK_ACC_LIMIT      equ 1821                 ; 18.21 ticks = one real second

; ---------------- Text ----------------
FONT_GLYPHS         equ 41                   ; characters in font_char_list
FONT_W              equ 8

; ---------------- Game limits (Appendix A placeholders) ----------------
START_LIVES         equ 3
MAX_LIVES           equ 9
COIN_SCORE          equ 100
LEVEL_TIME_SECONDS  equ 300
