;*# name=Direct-video routines for Maximus
;/

        include dv_asm.inc

PUBLIC _VIDGETXYB

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                          ;;
;; Get the current cursor position                                          ;;
;;                                                                          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

IFDEF __FARDATA__
_VIDGETXYB proc uses es, Col:ptr word, Row:ptr word
ELSE
_VIDGETXYB proc Col:ptr word, Row:ptr word
ENDIF
        mov     ah,3                            ; Get current cursor pos
        mov     bh,_Vid_Page
        int     10h

        xor     ah,ah                           ; Clear upper byte of AX

        mov     al,dh                           ; Get and store the row
IFDEF __FARDATA__
        les     bx, Row
        mov     word ptr es:[bx],ax

        mov     al,dl                           ; Get and store the column
        les     bx,Col
        mov     word ptr es:[bx],ax
ELSE
        mov     bx, Row
        mov     word ptr [bx], ax

        mov     al, dl
        mov     bx, Col
        mov     word ptr [bx], ax
ENDIF

        ret                                     ; ...and return.
_VIDGETXYB endp

end

