;*# name=Direct-video routines for Maximus
;/

        include dv_asm.inc

PUBLIC VIDSYNC, VIDSYNCCUR, VIDSYNCDV

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                          ;;
;; Update the physical cursor position, and tell DESQview/TopView to        ;;
;; fix their shadow buffers...                                              ;;
;;                                                                          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


VIDSYNCCUR proc uses es di

IFDEF __FARDATA__
        push    ds
        mov     cx, seg _RowTable
        mov     ds, cx
ENDIF
        mov     ah,02h                  ; Sync the actual cursor pos'n with
        mov     bh,_Vid_Page            ; the virtual cursor by calling the
        mov     dh,byte ptr _Vid_Row    ; BIOS SetCurPos function.
        mov     dl,byte ptr _Vid_Col
        int     10h

IFDEF __FARDATA__
        pop     ds
ENDIF
        ret
VIDSYNCCUR endp

        ; Tell DesqView/TV that the screen has been updated, so it will
        ; display correctly!

VIDSYNCDV proc uses es di

IFDEF __FARDATA__
        push    ds
        mov     cx, seg _RowTable
        mov     ds, cx
ENDIF

        mov     cx,_Vid_TotChars        ; Load total # of chars on screen,
        xor     di,di                   ; starting at offset 0...

        mov     ax,_Vid_Segment         ; Load assumed segment into AX
        mov     es,ax

        mov     ah,0ffh                 ; Make the actual call
        int     10h

IFDEF __FARDATA__
        pop     ds
ENDIF
        ret
VIDSYNCDV endp

VIDSYNC proc
        call    VIDSYNCDV
        call    VIDSYNCCUR
        ret
VIDSYNC endp

end

