;*# name=Direct-video routines for Maximus
;/

        include dv_asm.inc

PUBLIC VIDGETBPAGE, VIDSETPAGE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                          ;;
;; Get the currently-active screen page                                     ;;
;;                                                                          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

VIDGETBPAGE proc
        mov     ah,0fh
        int     10h
        mov     al,bh
        xor     ah,ah
        ret
VIDGETBPAGE endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                          ;;
;; Set current video page                                                   ;;
;;                                                                          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

VIDSETPAGE proc Pg:BYTE
        cmp     _Vid_MonoCard,0         ; Can only set pages for CGA
        jne     EndSP

        mov     ah,05h
        mov     al,Pg
        int     10h

        mov     al,Pg
        mov     _Vid_Page,al

EndSP:  ret

VIDSETPAGE endp

end

