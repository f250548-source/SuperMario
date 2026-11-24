; =====================================================================
; spritedata.inc - original 16x16 / 16x32 artwork drawn as text.
; Every ROW line is one bitmap row of exactly 16 letters:
;   .=transparent(0)  K=black(16)  W=white  R=red  r=light red(skin)
;   B=brown  Y=yellow  G=green  g=light green  U=blue  u=light blue
;   D=dark gray  T=light gray  P=pink  M=magenta  C=cyan  c=light cyan
; Colour 0 is transparent, so black outlines use palette index 16.
; =====================================================================

%macro ROW 1
    %strlen _rlen %1
    %if _rlen != 16
        %error "sprite row must be exactly 16 characters"
    %endif
    %assign _rc 1
    %rep 16
        %substr _rch %1 _rc
        %ifidn _rch, "."
            db 0
        %elifidn _rch, "K"
            db COL_OUTLINE
        %elifidn _rch, "W"
            db COL_WHITE
        %elifidn _rch, "R"
            db COL_RED
        %elifidn _rch, "r"
            db COL_LIGHT_RED
        %elifidn _rch, "B"
            db COL_BROWN
        %elifidn _rch, "Y"
            db COL_YELLOW
        %elifidn _rch, "G"
            db COL_GREEN
        %elifidn _rch, "g"
            db COL_LIGHT_GREEN
        %elifidn _rch, "U"
            db COL_BLUE
        %elifidn _rch, "u"
            db COL_LIGHT_BLUE
        %elifidn _rch, "D"
            db COL_DARK_GRAY
        %elifidn _rch, "T"
            db COL_LIGHT_GRAY
        %elifidn _rch, "P"
            db COL_PINK
        %elifidn _rch, "M"
            db COL_MAGENTA
        %elifidn _rch, "C"
            db COL_CYAN
        %elifidn _rch, "c"
            db COL_LIGHT_CYAN
        %else
            %error "unknown sprite letter"
        %endif
        %assign _rc _rc+1
    %endrep
%endmacro

%macro BLANKROWS 1
            times (16 * %1) db 0
%endmacro

; ---- Mario's head, shared by all six Mario bitmaps (rows 0-6) ----
%macro MARIO_HEAD 0
            ROW ".....RRRRRR....."
            ROW "....RRRRRRRRR..."
            ROW "....BBBrrKr....."
            ROW "...BrBrrrKrrr..."
            ROW "...BrBBrrrKrrr.."
            ROW "...BBrrrrKKKK..."
            ROW ".....rrrrrrr...."
%endmacro

; ================= small Mario (16x16) =================
spr_mario_stand:
            MARIO_HEAD
            ROW "....RRURRRR....."
            ROW "...RRRURRURRR..."
            ROW "..RRRRUUUURRRR.."
            ROW "..rrRUYUUYURrr.."
            ROW "..rrrUUUUUUrrr.."
            ROW "..rrUUUUUUUUrr.."
            ROW "....UUU..UUU...."
            ROW "...BBB....BBB..."
            ROW "..BBBB....BBBB.."

spr_mario_run:
            MARIO_HEAD
            ROW "....RRURRRR....."
            ROW "...RRRURRURRR..."
            ROW "..rrRRUUUURRRr.."
            ROW "..rrUUUYUUYUrr.."
            ROW "...UUUUUUUUUU..."
            ROW "...UUUU..UUUU..."
            ROW "..BBUU....UUBB.."
            ROW ".BBBB......BBBB."
            ROW ".BBB..........BB"

spr_mario_jump:
            MARIO_HEAD
            ROW ".rr.RRURRRR.rr.."
            ROW ".rrRRRURRURRRrr."
            ROW "..RRRRUUUURRRR.."
            ROW "...rUUYUUYUUr..."
            ROW "...UUUUUUUUUU..."
            ROW "..UUUU....UUUU.."
            ROW ".BBBU........UBB"
            ROW "BBBB..........BB"
            ROW "BBB............."

; ================= enemies and items (16x16) =================
spr_goomba_walk:
            ROW "......BBBB......"
            ROW "....BBBBBBBB...."
            ROW "...BBBBBBBBBB..."
            ROW "..BBBBBBBBBBBB.."
            ROW ".BBBWWBBBBWWBBB."
            ROW ".BBWKWBBBBWKWBB."
            ROW "BBBWWWBBBBWWWBBB"
            ROW "BBBBBBBBBBBBBBBB"
            ROW ".BBBBBBBBBBBBBB."
            ROW "..BBBBrrrrBBBB.."
            ROW "....rrrrrrrr...."
            ROW "....rrrrrrrr...."
            ROW "...KKKK..KKK...."
            ROW "..KKKKK..KKKK..."
            ROW "..KKKKKK.KKKKK.."
            ROW "................"

spr_goomba_flat:
            BLANKROWS 9
            ROW "......BBBB......"
            ROW "....BBBBBBBB...."
            ROW "...BBWKBBKWBB..."
            ROW "..BBBBBBBBBBBB.."
            ROW ".BBBBBBBBBBBBBB."
            ROW "..KKKKrrrrKKKK.."
            ROW "..KKKK......KKKK"

spr_koopa:
            ROW ".....YYYY......."
            ROW "....YYYYYY......"
            ROW "....YKYYYY......"
            ROW "....YYYYYYY....."
            ROW ".....YYYY......."
            ROW "....GGGGGG......"
            ROW "...GGgGGgGG....."
            ROW "..GGGgGGgGGG...."
            ROW "..GGgGGGGgGG...."
            ROW "..GGGGGGGGGG...."
            ROW "..YGGGGGGGGY...."
            ROW "...YYYYYYYY....."
            ROW "....YYY.YYY....."
            ROW "...YYYY.YYYY...."
            ROW "..BBBB...BBBB..."
            ROW "..BBBB...BBBB..."

spr_shell:
            BLANKROWS 4
            ROW ".....GGGGGG....."
            ROW "...GGGGGGGGGG..."
            ROW "..GGgGGGGGGgGG.."
            ROW ".GGGgGGGGGGgGGG."
            ROW ".GGGGGGggGGGGGG."
            ROW ".GGgGGGggGGGgGG."
            ROW ".YYYYYYYYYYYYYY."
            ROW "..YYYYYYYYYYYY.."
            ROW "...WWWWWWWWWW..."
            BLANKROWS 3

spr_mushroom:
            ROW "......RRRR......"
            ROW "....RRRRRRRR...."
            ROW "...RRWWRRRRWW..."
            ROW "..RWWWWRRRRWWWR."
            ROW ".RRWWWWRRRRWWWRR"
            ROW ".RRRWWRRRRRRWRRR"
            ROW "RRRRRRRRRRRRRRRR"
            ROW "....WWWWWWWW...."
            ROW "....WKKWWWKKW..."
            ROW "....WKKWWWKKW..."
            ROW "....WWWWWWWW...."
            ROW "....WWWWWWWW...."
            ROW ".....WWWWWW....."
            BLANKROWS 3

spr_coin:
            BLANKROWS 1
            ROW ".....YYYYYY....."
            ROW "....YYYYYYYY...."
            ROW "...YYYWWYYYYY..."
            ROW "...YYWYYYYYYY..."
            ROW "...YYWYYYYYYY..."
            ROW "...YYWYYYYYYB..."
            ROW "...YYWYYYYYYB..."
            ROW "...YYWYYYYYYB..."
            ROW "...YYYYYYYYYB..."
            ROW "...YYYYYYYYYB..."
            ROW "....YYYYYYYB...."
            ROW ".....YYYYYB....."
            BLANKROWS 3

spr_heart_full:
            BLANKROWS 1
            ROW "..RRRR....RRRR.."
            ROW ".RRWRRR..RRRRRR."
            ROW ".RWRRRRRRRRRRRR."
            ROW ".RRRRRRRRRRRRRR."
            ROW ".RRRRRRRRRRRRRR."
            ROW "..RRRRRRRRRRRR.."
            ROW "...RRRRRRRRRR..."
            ROW "....RRRRRRRR...."
            ROW ".....RRRRRR....."
            ROW "......RRRR......"
            ROW ".......RR......."
            BLANKROWS 4

spr_heart_empty:
            BLANKROWS 1
            ROW "..DDDD....DDDD.."
            ROW ".D....D..D....D."
            ROW ".D............D."
            ROW ".D............D."
            ROW ".D............D."
            ROW "..D..........D.."
            ROW "...D........D..."
            ROW "....D......D...."
            ROW ".....D....D....."
            ROW "......D..D......"
            ROW ".......DD......."
            BLANKROWS 4

; ================= Super Mario (16x32, use FLAG_TALL) =================
spr_super_stand:
            MARIO_HEAD
            ROW "....RRURRRR....."
            ROW "...RRRURRURRR..."
            ROW "..RRRRUUUURRRR.."
            ROW "..rrRUYUUYURrr.."
            ROW "..rrrUUUUUUrrr.."
            ROW "..rrUUUUUUUUrr.."
            %rep 5
            ROW "...UUUUUUUUUU..."
            %endrep
            %rep 10
            ROW "...UUUU..UUUU..."
            %endrep
            ROW "...BBBB..BBBB..."
            ROW "..BBBBB..BBBBB.."
            ROW "..BBBBB..BBBBB.."
            ROW "..BBBBB..BBBBB.."

spr_super_run:
            MARIO_HEAD
            ROW "....RRURRRR....."
            ROW "...RRRURRURRR..."
            ROW "..rrRRUUUURRRr.."
            ROW "..rrUUUYUUYUrr.."
            ROW "...UUUUUUUUUU..."
            %rep 6
            ROW "...UUUUUUUUUU..."
            %endrep
            ROW "...UUUU..UUUU..."
            ROW "...UUUU...UUUU.."
            ROW "..UUUU....UUUU.."
            ROW "..UUUU.....UUUU."
            ROW "..UUUU.....UUUU."
            ROW ".UUUU......UUUU."
            ROW ".UUUU.......UUUU"
            ROW ".UUUU.......UUUU"
            ROW "BUUUU........UUU"
            ROW "BBBB.........UUU"
            ROW "BBBB.........UUU"
            ROW "BBBB.........BBB"
            ROW "BBBB.........BBB"
            ROW "BBB..........BBB"

spr_super_jump:
            MARIO_HEAD
            ROW ".rr.RRURRRR.rr.."
            ROW ".rrRRRURRURRRrr."
            ROW "..RRRRUUUURRRR.."
            ROW "...rUUYUUYUUr..."
            %rep 7
            ROW "...UUUUUUUUUU..."
            %endrep
            ROW "..UUUUU..UUUUU.."
            ROW "..UUUUU..UUUUU.."
            ROW ".UUUUU....UUUUU."
            ROW ".UUUU......UUUU."
            ROW "UUUUU......UUUUU"
            ROW "UUUU........UUUU"
            ROW "BBBB........BBBB"
            ROW "BBBB........BBBB"
            BLANKROWS 6

; ================= tiles (16x16) =================
spr_tile_ground:
            ROW "gggggggggggggggg"
            ROW "GGGGGGGGGGGGGGGG"
            ROW "BBBBBBBBBBBBBBBB"
            ROW "BBBBBBBBBBKBBBBB"
            ROW "BBBKBBBBBBBBBBBB"
            ROW "BBBBBBBBBBBBBKBB"
            ROW "BBBBKBBBBBBBBBBB"
            ROW "BBBBBBBBBKBBBBBB"
            ROW "BBBBBBBBBBBBBBBB"
            ROW "BKBBBBBBBBBBBKBB"
            ROW "BBBBBBKBBBBBBBBB"
            ROW "BBBBBBBBBBKBBBBB"
            ROW "BBKBBBBBBBBBBBBB"
            ROW "BBBBBBBBBBBBKBBB"
            ROW "BBBBBKBBBBBBBBBB"
            ROW "BBBBBBBBBBBBBBBB"

spr_tile_brick:
            ROW "KKKKKKKKKKKKKKKK"
            %rep 3
            ROW "RRRRRRRKRRRRRRRK"
            %endrep
            ROW "KKKKKKKKKKKKKKKK"
            %rep 3
            ROW "RRRKRRRRRRRKRRRR"
            %endrep
            ROW "KKKKKKKKKKKKKKKK"
            %rep 3
            ROW "RRRRRRRKRRRRRRRK"
            %endrep
            ROW "KKKKKKKKKKKKKKKK"
            %rep 3
            ROW "RRRKRRRRRRRKRRRR"
            %endrep

spr_tile_qblock:
            ROW "KKKKKKKKKKKKKKKK"
            ROW "KYYYYYYYYYYYYYYK"
            ROW "KYKYYYYYYYYYYKYK"
            ROW "KYYYYYKKKKYYYYYK"
            ROW "KYYYYKKYYKKYYYYK"
            ROW "KYYYYKKYYKKYYYYK"
            ROW "KYYYYYYYKKYYYYYK"
            ROW "KYYYYYYKKYYYYYYK"
            ROW "KYYYYYYKKYYYYYYK"
            ROW "KYYYYYYYYYYYYYYK"
            ROW "KYYYYYYKKYYYYYYK"
            ROW "KYYYYYYKKYYYYYYK"
            ROW "KYYYYYYYYYYYYYYK"
            ROW "KYKYYYYYYYYYYKYK"
            ROW "KYYYYYYYYYYYYYYK"
            ROW "KKKKKKKKKKKKKKKK"

spr_tile_empty:
            ROW "KKKKKKKKKKKKKKKK"
            ROW "KBBBBBBBBBBBBBBK"
            ROW "KBKBBBBBBBBBBKBK"
            %rep 10
            ROW "KBBBBBBBBBBBBBBK"
            %endrep
            ROW "KBKBBBBBBBBBBKBK"
            ROW "KBBBBBBBBBBBBBBK"
            ROW "KKKKKKKKKKKKKKKK"

spr_tile_pipe_tl:
            ROW "KKKKKKKKKKKKKKKK"
            %rep 14
            ROW "KGgggGGGGGGGGGGG"
            %endrep
            ROW "KKKKKKKKKKKKKKKK"

spr_tile_pipe_tr:
            ROW "KKKKKKKKKKKKKKKK"
            %rep 14
            ROW "GGGGGGGGGGGGGGGK"
            %endrep
            ROW "KKKKKKKKKKKKKKKK"

spr_tile_pipe_bl:
            %rep 16
            ROW "..KGgggGGGGGGGGG"
            %endrep

spr_tile_pipe_br:
            %rep 16
            ROW "GGGGGGGGGGGGGK.."
            %endrep

spr_tile_stair:
            ROW "KKKKKKKKKKKKKKKK"
            ROW "KWWWWWWWWWWWWWDK"
            %rep 12
            ROW "KWTTTTTTTTTTTTDK"
            %endrep
            ROW "KWDDDDDDDDDDDDDK"
            ROW "KKKKKKKKKKKKKKKK"

spr_tile_flagpole:
            %rep 16
            ROW ".......WT......."
            %endrep

spr_flag:
            BLANKROWS 1
            ROW "..............GG"
            ROW "............GGGG"
            ROW "..........GGGGGG"
            ROW "........GGGGGGGG"
            ROW "......GGGGGGGGGG"
            ROW "....GGGGGGGGGGGG"
            ROW "..GGGGGGGGGGGGGG"
            ROW "GGGGGGGGGGGGGGGG"
            BLANKROWS 7

; ---- tile code -> bitmap address (index = tile code, word entries) ----
tile_address_table:
            dw 0                         ; 0  sky (never drawn)
            dw spr_tile_ground           ; 1
            dw spr_tile_brick            ; 2
            dw spr_tile_qblock           ; 3  ? coin
            dw spr_tile_qblock           ; 4  ? mushroom
            dw spr_tile_empty            ; 5
            dw spr_tile_pipe_tl          ; 6
            dw spr_tile_pipe_tr          ; 7
            dw spr_tile_pipe_bl          ; 8
            dw spr_tile_pipe_br          ; 9
            dw spr_tile_stair            ; 10
            dw spr_coin                  ; 11
            dw spr_tile_flagpole         ; 12
