;*# name=Direct-video routines for Maximus
;/

        include dv_asm.inc

PUBLIC _VIDGETNUMROWS

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                          ;;
;; Get the number of rows on-screen (EGA/VGA BIOS call - if BIOS not        ;;
;; present, defaults to 25)                                                 ;;
;;                                                                          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_VIDGETNUMROWS proc
        push    es
        push    bp

        mov     dl,24                           ; Default 24 rows on screen

        mov     ax,1130h                        ; EGA BIOS: Get bottom row
        int     10h

        inc     dl                              ; Increment by one

        xor     ah,ah                           ; Zero out high byte
        mov     al,dl                           ; And return in AX.

        pop     bp
        pop     es
        ret
_VIDGETNUMROWS endp

end

