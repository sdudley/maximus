;# name=Direct-video assembler include file for Maximus

;; CHANGE BOTH THIS FILE AND DV_ASM.INC!


        IFDEF MSMALL
;        %out small model
                .model small, pascal
                EXTRN REGSCROLLUP:near
                .model small, c
                __NEARDATA__    equ     1
                __NEARCODE__    equ     1
        ELSEIFDEF MMEDIUM
;        %out medium model
                .model medium, pascal
                EXTRN REGSCROLLUP:far
                .model medium, c
                __NEARDATA__    equ     1
                __FARCODE__     equ     1
        ELSEIFDEF MCOMPACT
;        %out compact model
                .model compact, pascal
                EXTRN REGSCROLLUP:near
                .model compact, c
                __FARDATA__     equ     1
                __NEARCODE__    equ     1
        ELSE
;        %out large model
                .model large, pascal
                EXTRN REGSCROLLUP:far
                .model large, c
                __FARDATA__     equ     1
                __FARCODE__     equ     1
        ENDIF

_Vid_StatusPort equ     Vid_StatusPort
_Vid_Bios       equ     Vid_Bios
_Vid_Segment    equ     Vid_Segment

        include dv_asm.mac

;       DOSSEG
;.DATA

        EXTRN RowTable:WORD, Vid_Display:WORD, Vid_StatusPort:WORD
        EXTRN Vid_HaveSnow:WORD, Vid_MonoCard:WORD,
        EXTRN Vid_NumRows:WORD, Vid_NumCols:WORD, Vid_Row:WORD
        EXTRN Vid_Col:WORD, Vid_Segment:WORD, Vid_TotChars:WORD
        EXTRN Vid_Bios:WORD

        EXTRN Vid_Attribute:BYTE, Vid_Page:BYTE

.CODE


