;*# name=Fast (relative to dos) int 29h output character function
;/

;       dosseg

IFDEF   MSMALL
        .model small, pascal
ELSE
        .model medium, pascal
ENDIF
        .data

.CODE

        PUBLIC XPUTCHR

XPUTCHR proc Chr:byte
        mov     al,Chr                  ; Load the byte into AL

        cmp     al,0ah                  ; If it's a control code
        jle     X_CCode

X_DoIt: int     29h                     ; Dump char via int 29h putchar func
        jmp     short X_End

X_CCode:
        cmp     al,0ah                  ; If it's a CR/LF...
        je      X_CrLF

        cmp     al,09h                  ; If it's a tab...
        je      X_Tab
        jmp     short X_DoIt            ; Else it's a normal char.

X_CrLF: mov     al,0dh                  ; Put both a '\r' and a '\n' down
        int     29h                     ; the tube...

        mov     al,0ah
        int     29h
        jmp     short X_End

X_Tab:  mov     ah,8                    ; Print eight spaces...
        mov     al,20h

X_Tab1: int     29h

        dec     ah
        or      ah,ah
        jnz     X_Tab1

X_End:  ret

XPUTCHR endp

END

