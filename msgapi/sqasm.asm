; $Id: sqasm.asm,v 1.1.1.1 2002/10/01 17:54:28 sdudley Exp $

IFNDEF OS_2

IFNDEF __FLAT__

        .model large, pascal
        .code

        public FARWRITE, FARREAD

; int far pascal farread(int handle,char far *buf,unsigned int len)
; int far pascal farwrite(int handle,char far *buf,unsigned int len)
; int far pascal shareloaded(void);

FARREAD proc uses ds, handle:word, buf:dword, len:word
        mov     bx,handle               ; Load file handle
        mov     cx,len                  ; Load length

        mov     ax,word ptr [buf+2]     ; Load segment into AX (and then DS)
        mov     dx,word ptr buf         ; Load offset into DX

        mov     ds,ax                   ; Move AX (the segment) into DS

        mov     ah,3fh                  ; Do it
        int     21h
        jnc     okay

        mov     ax, -1
okay:

        ret
FARREAD endp

FARWRITE proc uses ds, handle:word, buf:dword, len:word
        mov     bx,handle               ; Load file handle
        mov     cx,len                  ; Length of write

        mov     ax,word ptr [buf+2]     ; Load segment into DS

        mov     dx,word ptr buf         ; Load offset into DX

        mov     ds,ax                   ; Move AX (the segment) into DS

        mov     ah,40h                  ; Call it
        int     21h
        jnc     doneit                  ; Return # of bytes if no error

        mov     ax,-1                   ; Otherwise, return -1

doneit:
        ret
FARWRITE endp

ELSE            ; __FLAT__

        .386p
        .model small, pascal
        .code

        public FARWRITE, FARREAD

; int far pascal farread(int handle,char far *buf,unsigned int len)
; int far pascal farwrite(int handle,char far *buf,unsigned int len)
; int far pascal shareloaded(void);

FARREAD proc
        mov     ecx,[esp+4]             ; Load length

        mov     edx, [esp+8]            ; Load offset
        mov     ebx, [esp+12]           ; Load file handle

        mov     ah,3fh                  ; Do it
        int     21h
        jnc     okay

        mov     eax, -1
okay:   ret 0ch
FARREAD endp

FARWRITE proc
        mov     ebx,[esp+12]            ; Load file handle
        mov     ecx,[esp+4]             ; Length of write

        mov     edx,[esp+8]             ; Load offset into DX

        mov     ah,40h                  ; Call it
        int     21h
        jnc     doneit                  ; Return # of bytes if no error

        mov     eax,-1                  ; Otherwise, return -1

doneit: ret 0ch
FARWRITE endp


ENDIF ; FLAT

ENDIF ; !OS_2

end

