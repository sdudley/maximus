        ; Stack overflow handler for WC only.  This cleans up the
        ; keyboard handler if Max abends.

        .model medium

;       dosseg

        extrn __STACKLOW:word
        extrn brkuntrap_:word
        public __STK, __STKOVERFLOW_
        .code

MsgOverflow db  'Stack overflow!', 0dh, 0ah, '$'

__STK proc
        cmp     ax,sp
        jae     __STKOVERFLOW_
        sub     ax,sp
        neg     ax
        cmp     ax,word ptr __STACKLOW
        jbe     __STKOVERFLOW_
        ret
__STK endp

__STKOVERFLOW_ proc
        call    far ptr brkuntrap_      ; fix keyb handler

        push    cs
        pop     ds
        mov     dx,offset MsgOverflow
        mov     ah,09H
        int     21H

        mov     ax,4c10H                ; exit w/erl 16
        int     21H
__STKOVERFLOW_ endp

end

