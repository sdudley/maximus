;*# name=Direct-video routines for Maximus
;/

        include dv_asm.inc

PUBLIC VIDGETMODE, VIDSETMODE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                          ;;
;; Get the current video mode number - Return in AX                         ;;
;;                                                                          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

VIDGETMODE proc
        mov     ah,0fh
        int     10h
        xor     ah,ah
        ret
VIDGETMODE endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                          ;;
;; Set the current video mode                                               ;;
;;                                                                          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

VIDSETMODE proc Mode:BYTE
        mov     ah,00h
        mov     al,Mode
        int     10h
        ret
VIDSETMODE endp

end

