;*# name=Alternate int 24h critical error handler
;/

;       dosseg

IFNDEF __FLAT__
        .model large
        .code

        ASSUME cs:CRIT_ASM_TEXT, ds:CRIT_ASM_TEXT, ss:CRIT_ASM_TEXT

        PUBLIC _newint24h

; Int 24h critical error handler.
;
; On entry:
;
;       AH = error code
;       AL = drive code
;       DI = driver error code (bits 0-7)
;    BP:SI = address of device driver's header

diskerr db 13, 10, 'Critical error XXXXing drive X:', 13, 10, 0
deverr  db 13, 10, 'Critical error accessing device         ', 13, 10, 0


_newint24h proc
        sti                             ; Turn on interrupts

        push    ax                      ; Save registers
        push    bx
        push    cx
        push    dx
        push    ds
        push    es
        push    si
        push    di

        push    cs                      ; All the data we need is in
        pop     ds                      ; our code segment.

        mov     es, bp                  ; Segment of dev driver header

        test    ah, 80h                 ; Check for a disk error
        jnz     ErrOther                ; If not, go somewhere else.



ErrDisk:

        mov     bp, offset diskerr      ; Load the disk error msg

        add     al, 'A'                 ; Display the drive on which the
        mov     byte ptr ds:[bp+31], al ; error occurred.

        test    ah, 1                   ; If the low bit of AH is set to
        jnz     ErrWrite                ; 1, then it's a write error.

        mov     byte ptr ds:[bp+17], 'r'
        mov     byte ptr ds:[bp+18], 'e'
        mov     byte ptr ds:[bp+19], 'a'
        mov     byte ptr ds:[bp+20], 'd'
        jmp     short ErrShow

ErrWrite:
        mov     byte ptr ds:[bp+17], 'w'
        mov     byte ptr ds:[bp+18], 'r'
        mov     byte ptr ds:[bp+19], 'i'
        mov     byte ptr ds:[bp+20], 't'
        jmp     short ErrShow

        ; Handle a non-disk error message


ErrOther:

        mov     bp, offset deverr               ; Load error message offset

        test    word ptr es:[si+4], 8000h       ; Is it a char device?
        jz      short ErrShow                   ; If not, no more info, so
                                                ; display the error msg.

        ; Transfer name of char device to the err msg

        push    ds                              ; Save DS

        push    es                              ; Load seg of device header.
        pop     ds

        add     si, 0ah                         ; SI already points to
                                                ; header start.  Get the
                                                ; name, which is 10 chars
                                                ; past beginning of header.

        push    cs                              ; Segment of msg
        pop     es

        mov     di, bp                          ; Offset of msg
        add     di, 34                          ; Device name is 32 chars
                                                ; into msg.

        mov     cx, 4                           ; Name is 4 words long
        repnz   movsw                           ; Transfer DS:SI -> ES:DI

        pop     ds                              ; Restore DS

ErrShow:
        ; By now, err msg is in ds:bp.  Display this on-screen without
        ; using any DOS services.


        mov     al, byte ptr ds:[bp]            ; Fetch a character

        or      al, al                          ; If it's zero, get out
        jz      Done

        inc     bp                              ; Increment the string ptr

        mov     ah, 0eh                         ; Display char (TTY)
        xor     bx, bx                          ; Page zero
        int     10h                             ; Display the message
        jmp     short ErrShow


Done:
        pop     di                              ; Restore registers
        pop     si                              ; and exit.
        pop     es
        pop     ds
        pop     dx
        pop     cx
        pop     bx
        pop     ax

        mov     al, 3                           ; Fail operation
        iret
_newint24h endp
ENDIF

end

