;*# name=Window blitz (fast update) routine
;/


        include dv_asm.inc

PUBLIC _WINBLITZ

_WINBLITZ proc uses es si di, start_col:word, num_col:word, from_ofs:dword, win_start_col:word, this_row:word

IFDEF __FARDATA__
        push    ds
        mov     cx, seg _Vid_StatusPort
        mov     ds, cx
ENDIF
        LoadRow 0,_RowTable,this_row,start_col

        mov     es, _Vid_Segment        ; Load appropriate segment

        mov     cx, num_col             ; Load the # of cols to use

        mov     si, win_start_col       ; Load start of the row
        shl     si, 1                   ; Multiply by two

        add     si, word ptr [from_ofs] ; Load the offset of DS:SI

        mov     ax, word ptr [from_ofs+2] ; Load segment of DS:SI

        cmp     _Vid_HaveSnow, 0        ; If we're snowless, do it the fast
        je      short fast              ; way.


        ;
        ; SLOW WAY
        ;


slow_top:

        mov     bx,ds                   ; Save old DS reg

        SnowCheck _Vid_HaveSnow, 1

        mov     ds,ax
        movsw                           ; Copy the byte to ES:DI
        mov     ds,bx                   ; Restore DS reg

        loopnz  slow_top                ; Continue for CX bytes.
        jmp     short getout            ; If not, get out.


        ;
        ; FAST WAY
        ;

fast:   mov     bx, ds                  ; Save DS reg
        mov     ds, ax                  ; Move CX words DS:SI -> ES:DI
        repnz   movsw                   ;
        mov     ds, bx                  ; Restore DS

getout: 

IFDEF __FARDATA__
        pop     ds
ENDIF
        ret

_WINBLITZ endp

end
