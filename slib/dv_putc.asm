;*# name=Direct-video routines for Maximus
;/

        include dv_asm.inc

PUBLIC VidPutc

VidPutc proc uses si di es, Char:byte

IFDEF __FARDATA__
        push    ds
        mov     cx, seg _RowTable
        mov     ds, cx
ENDIF
        mov     ax,_Vid_Segment         ; Load video segment into ES
        mov     es,ax

        LoadRow 0,_RowTable,_Vid_Row,_Vid_Col

        xor     ah,ah                   ; Clear high byte of AX,
        mov     al,byte ptr Char        ; load the new byte,
        mov     si,ax                   ; and store in SI.

        or      al,al                   ; If it's zero, get outta here!
        je      ByeBye

        cmp     al,0ah                  ; If '\n', do a LF
        je      short LF

        cmp     al,0dh                  ; If it's a '\r', do CR
        je      short CR

        cmp     al,08h
        je      short BS
        jmp     AW



BS:     cmp     _Vid_Col,0
        jne     BS_Dec
        jmp     EndVP                   ; Move back one, as long as col != 0

BS_Dec: dec     _Vid_Col
        jmp     EndVP




CR:     mov     _Vid_Col,0              ; If it's a '\r', just set column to 0
ByeBye: jmp     EndVP


LF:     inc     _Vid_Row
        mov     ax,_Vid_Row             ; Check if Vid_Row is at the bottom
                                        ; of the screen.  If so, scroll.
        cmp     ax,_Vid_NumRows
        jb      short ByeBye

        mov     _Vid_Row,ax             ; If it's OK, then do the scroll.

        mov     ax,offset _Vid_Col
        push    ax

        mov     ax,offset _Vid_Row
        push    ax

        call    REGSCROLLUP
        jmp     EndVP


DoWrap: mov     _Vid_Col,0              ; Set column to one
        inc     _Vid_Row                ; Next row down

        mov     ax,_Vid_Row             ; Check if wrap would cause
                                        ; us to go over edge of
        cmp     ax,_Vid_NumRows         ; screen.

        jb      short AW_LR1            ; If not, disp the char.

        mov     _Vid_Row,ax             ; Otherwise, do the scroll.

        mov     ax,offset _Vid_Col
        push    ax

        mov     ax,offset _Vid_Row
        push    ax

        call    REGSCROLLUP

AW_LR1: mov     _Vid_Col,1

        LoadRow 1,_RowTable,_Vid_Row,_Vid_Col

        jmp     short AWNInc

AW_LR:  LoadRow 0,_RowTable,_Vid_Row,_Vid_Col
        jmp     short AWNInc

AW:     inc     _Vid_Col

AWNInc: mov     cx,_Vid_NumCols         ; Check to make sure that
        cmp     _Vid_Col,cx             ; we're within screen
        ja      short DoWrap            ; boundaries...

        mov     ax,_Vid_Bios            ; Check whether or not we need to
        or      ax,ax                   ; use the BIOS for output
        jnz     DoBios

        SnowCheck _Vid_HaveSnow 3       ; Check for display snow

        mov     ax,si
        mov     ah,_Vid_Attribute       ; Plop the attribute into AH
        stosw                           ; And put away AX into ES:DI


DoBios: BiosPutAt _Vid_Row, _Vid_Col, Char, _Vid_Attribute

EndVP:  

IFDEF __FARDATA__
        pop     ds
ENDIF

        ret

VidPutc endp

end


