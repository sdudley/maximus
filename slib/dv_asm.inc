;# name=Direct-video assembler include file for Maximus


;; CHANGE BOTH THIS FILE AND DV_ASMC.INC!

        IFDEF MSMALL
;        %out small model
                .model small, pascal
                __NEARDATA__    equ     1
                __NEARCODE__    equ     1
        ELSEIFDEF MMEDIUM
;        %out medium model
                .model medium, pascal
                __NEARDATA__    equ     1
                __FARCODE__     equ     1
        ELSEIFDEF MCOMPACT
;        %out compact model
                .model compact, pascal
                __FARDATA__     equ     1
                __NEARCODE__    equ     1
        ELSE
;        %out large model
                .model large, pascal
                __FARDATA__     equ     1
                __FARCODE__     equ     1
        ENDIF

        include dv_asm.mac

;       DOSSEG
;.DATA

        EXTRN _RowTable:WORD
        EXTRN _Vid_Display:WORD, _Vid_StatusPort:WORD
        EXTRN _Vid_HaveSnow:WORD, _Vid_MonoCard:WORD,
        EXTRN _Vid_NumRows:WORD, _Vid_NumCols:WORD, _Vid_Row:WORD
        EXTRN _Vid_Col:WORD, _Vid_Segment:WORD, _Vid_TotChars:WORD
        EXTRN _Vid_Bios:WORD

        EXTRN _Vid_Attribute:BYTE, _Vid_Page:BYTE

.CODE

        IFDEF   MSMALL
                EXTRN REGSCROLLUP:near
        ELSEIFDEF MCOMPACT
                EXTRN REGSCROLLUP:near
        ELSE
                EXTRN REGSCROLLUP:far
        ENDIF


