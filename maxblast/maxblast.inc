;
; For OS/2, we need the DosPortAccess function
;

IFDEF OS_2
        extrn DOSPORTACCESS:byte
ENDIF

;
; Play a note on the soundblaster - call the specified output function
; to access registers.
;

DoNote macro OutFunc
        mov     ax, 0a0h                        ; Reg for low byte
        push    ax                              ; Push it

        mov     ax, note                        ; Get nbote
        xor     ah, ah                          ; Get low byte
        push    ax                              ; Push it

        call    far ptr &OutFunc&

        mov     ax, 0b0h                        ; Register to use for
        push    ax                              ; octave and high nibble of
                                                ; note

        mov     ax, note                        ; Load note
        mov     bx, octave                      ; Load octave

        mov     al, ah                          ; Shift note right by 8 bits
        xor     ah, ah

        shl     bx, 1                           ; Shift octabe left by two
        shl     bx, 1

        or      ax, bx                          ; OR the two together
        or      ax, 020h                        ; Turn the speaker on

        push    ax                              ; Push it

        call    far ptr &OutFunc&               ; Call it
endm


OutPort macro PortNum, DoAdd
        mov     dx, PortNum                     ; Load the address port
    ifnb   <DoAdd>
        add     dx, DoAdd                       ; Addr is 222h, data is 223h
    endif
        mov     ax, reg                         ; Load the register value
        out     dx, al                          ; Send it

        rept    6
        in      al, dx                          ; in al, dx * 6
        endm

        inc     dx                              ; The data port is next
        mov     ax, dat                         ; Load data
        out     dx, al                          ; Send it

        dec     dx                              ; Read the reg port again

        rept    35
        in      al, dx                          ; in al, dx * 35
        endm
endm

;
; Macro to wait for the write latch to clear
;

WaitDX  macro LAB
&LAB&:  in      al, dx
        test    al, 80h
        jnz     short &LAB&
endm

;
; Load DS with the correct value - only for OS/2 DLL model.
;

IFDEF OS_2
        LoadDS macro
                mov     ax, seg _ioaddr
                mov     ds, ax
        endm
ELSE
        LoadDS macro
                ; Null macro for DOS
        endm
ENDIF


