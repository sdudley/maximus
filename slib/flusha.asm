;*# name=Truly flush a file handle to disk
;/

IFNDEF __FLAT__

        .model medium, pascal
        .code

        PUBLIC FLUSH_HANDLE2

FLUSH_HANDLE2 proc handle:WORD
        ; duplicate file handle, then close the duplicate.
        mov     ah,45h
        mov     bx,handle
        int     21h
        jc      Er
        mov     bx,ax
        mov     ah,3eh
        int     21h
Er:     ret
FLUSH_HANDLE2 endp

ELSE            ; __FLAT__

        .386p
        .model small, pascal
        .code

        PUBLIC FLUSH_HANDLE2

FLUSH_HANDLE2 proc
        ; duplicate file handle, then close the duplicate.
        mov     ah,45h
        mov     ebx, [esp+4]
        int     21h
        jc      Er

        mov     ebx,eax
        mov     ah,3eh
        int     21h
Er:     ret 4
FLUSH_HANDLE2 endp

ENDIF

END

