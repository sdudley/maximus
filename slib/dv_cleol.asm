;*# name=Direct-video routines for Maximus
;/

        include dv_asm.inc

PUBLIC VIDCLEOL

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                          ;;
;; Clear from cursor position to end-of-line                                ;;
;;                                                                          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


VIDCLEOL proc uses si di es
        cld

        mov     ax,_Vid_Bios            ; Check whether or not we need to
        or      ax,ax                   ; use the BIOS for output
        jz      DoBios

        mov     ax,_Vid_Segment                 ; Load video segment into ES
        mov     es,ax

        LoadRow 0,_RowTable,_Vid_Row,_Vid_Col

        ; Load the number of columns to clear
        mov     ax,_Vid_NumCols                 ; CX=Vid_NumCols-Vid_Col;
        mov     dx,_Vid_Col
        sub     ax,dx
        mov     cx,ax

        cmp     _Vid_HaveSnow,0
        je      short NoSnow

Snow:   SnowCheck _Vid_HaveSnow 1

        mov     al,' '                          ; Make the char blank.
        stosb

        SnowCheck _Vid_HaveSnow 2

        mov     al,_Vid_Attribute               ; Now dump the attribute...
        stosb

        loopnz  Snow                            ; Keep going until done.

        jmp     short CLDone


        ; Here's the FAST way to do a CLEOL, if we have a snowless display:

NoSnow: mov     al,' '
        mov     ah,_Vid_Attribute               ; Plop the attribute into AH

        repnz   stosw                           ; And clear to end of line,
        jmp     short CLDone                    ; starting at ES:DI and
                                                ; continuing to ES:DI+(CX*2).

DoBios: BiosGoto _Vid_Row,_Vid_Col

        mov     ax,_Vid_NumCols                 ; CX=Vid_NumCols-Vid_Col;
        mov     dx,_Vid_Col
        sub     ax,dx
        mov     dx,ax

        BiosPutcRep ' ',_Vid_Attribute,dx

CLDone: ret

VIDCLEOL endp

end

