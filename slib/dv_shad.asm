;*# name=Direct-video routines for Maximus
;/

        include dv_asm.inc

PUBLIC START_SHADOW, END_SHADOW, VIDGETBUFFER

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                          ;;
;; Start DESQview's shadow buffering                                        ;;
;;                                                                          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


START_SHADOW proc                               ; Start DesqView shadow
        mov     ax,2b02h                        ; buffering...
        mov     cx,4445h
        mov     dx,5351h
        int     21h
        ret
START_SHADOW endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                          ;;
;; End DESQview's shadow buffering                                          ;;
;;                                                                          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

END_SHADOW proc
        mov     ax,2b02h
        mov     cx,4445h
        mov     dx,5351h
        int     21h
        ret
END_SHADOW endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                          ;;
;; Get address of virtual shadow buffer.  Segment is returned in AX.        ;;
;;                                                                          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

VIDGETBUFFER proc uses es di, Mono:WORD
        mov     cx,0b000h               ; Assumed buffer for mono
        cmp     Mono,0
        jne     DoCall                  ; If we are mono, do the call.

        mov     cx,0b800h               ; Otherwise, assume C/E/VGA.

DoCall: 

IFNDEF __FLAT__
        mov     ah,0feh                 ; Get virtual video buffer, func 0feh
        mov     es,cx
        xor     di,di
        int     10h

        mov     ax,es                   ; Return segment in AX
ELSE
        mov     ax, cx
ENDIF
        ret
VIDGETBUFFER endp


end

