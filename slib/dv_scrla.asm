;*# name=Direct-video routines for Maximus
;/

        include dv_asm.inc

PUBLIC VIDSCROLL

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                          ;;
;; Scroll the screen in a certain direction                                 ;;
;;                                                                          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

VIDSCROLL proc Direction:BYTE, NumOfLines:BYTE, Attribute:BYTE, LeftCol:BYTE, TopRow:BYTE, RightCol:BYTE, BotRow:BYTE
        mov     ah,Direction
        mov     al,NumOfLines
        mov     bh,Attribute
        mov     ch,TopRow
        mov     cl,LeftCol
        mov     dl,RightCol
        mov     dh,BotRow
        int     10h
        ret
VIDSCROLL endp

end

