;*# name=Direct-video routines for Maximus
;/

        include dv_asmc.inc

        .CODE

PUBLIC VidPutch

VidPutch proc uses si di es, Row:Word, Col:word, Char:byte, Attr:byte

IFDEF __FARDATA__
        push    ds
        mov     cx, seg RowTable
        mov     ds, cx
ENDIF
        mov     ax,_Vid_Bios            ; Check whether or not we need to
        or      ax,ax                   ; use the BIOS for output
        jnz     DoBios

        mov     ax,_Vid_Segment         ; Load video segment into ES
        mov     es,ax

        LoadRow 0,RowTable,Row,Col      ; Figure out the right row/column

        mov     al,byte ptr Char        ; Load the character and
        mov     ah,byte ptr Attr        ; attribute

        SnowCheck Vid_HaveSnow 4        ; Check for display snow

        stosw                           ; And dump in memory.
        jmp     short getout

DoBios: BiosPutAt Row,Col,Char,Attr

getout:

IFDEF __FARDATA__
        pop     ds
ENDIF
        ret
VidPutch endp

end

