; Far-model 'memmove' command

.model large, pascal

.code

public F_MEMMOVE


IFNDEF OS_2
normalize proc far
        mov     cx, dx                          ; Move offset to CX and
        shr     cx, 1                           ; shift right to find seg
        shr     cx, 1                           ; count.  
        shr     cx, 1
        shr     cx, 1

        add     ax, cx                          ; Then add to ax.

        and     dx, 0fh                         ; Leave only the remaining
        ret                                     ; 4 bits in dx.
normalize endp
ENDIF


F_MEMMOVE proc far uses es si di, to:dword, from:dword, len:word
IFNDEF OS_2
        mov     ax, word ptr [to+2]             ; Normalize the 'to' ptr
        mov     dx, word ptr [to]
        call    far ptr normalize
        mov     word ptr [to+2], ax
        mov     word ptr [to], dx

        mov     ax, word ptr [from+2]           ; Normalize the 'from' ptr
        mov     dx, word ptr [from]
        call    far ptr normalize
        mov     word ptr [from+2], ax
        mov     word ptr [from], dx
ENDIF

        mov     di, word ptr [to]               ; Load the offsets
        mov     si, word ptr [from]
        mov     cx, word ptr [len]              ; Load the length

        mov     ax, word ptr [to+2]             ; Load the segments
        mov     dx, word ptr [from+2]

        cmp     ax, dx                          ; Do CLD if to > from
        jb      do_cld

        cmp     di, si                          ; Do CLD if to > from
        jb      do_cld

        std                                     ; Otherwise do STD
        mov     dx, 1                           ; Set 'std is set' flag

        add     si, cx                          ; Set the offsets to the
        dec     si                              ; end of the block
        dec     si

        add     di, cx
        dec     di
        dec     di

        jmp     short do_copy                   ; Now perform the copy

do_cld:
        cld                                     ; Forward direction
        xor     dx, dx                          ; We're NOT doing std

do_copy:
        push    ds

        mov     ax, word ptr [to+2]
        mov     es, ax

        mov     ax, word ptr [from+2]

        shr     cx, 1                           ; Divide by 2 for word
                                                ; copy.

        mov     ds, ax                          ; Move AX into DS
        repnz   movsw                           ; Move the right # of
                                                ; words.

        adc     cx, cx                          ; Add the carry to CX,
                                                ; setting it to 1 if there
                                                ; is a byte to copy

        or      dx, dx
        jz      doing_cld

        inc     si
        inc     di

doing_cld:

        repz  movsb                             ; Move the remaining
                                                ; byte.

        pop     ds                              ; Restore DS
        cld                                     ; Restore the dir flag
        ret
F_MEMMOVE endp

end

