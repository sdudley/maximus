.286
.model large, pascal

include maxblast.inc

.DATA

        public _ioaddr, _irq

_ioaddr dw      220h
_irq    dw      5

.CODE
        public SBLASTINIT, SBLASTTERM
        public SBLASTVOCENABLE, SBLASTVOCPLAY
        public SBLASTFMOUTALL, SBLASTFMOUTRIGHT, SBLASTFMOUTLEFT
        public SBLASTFMNOTEALL, SBLASTFMNOTELEFT, SBLASTFMNOTERIGHT

; Send a byte to the left speaker

SBLASTFMOUTLEFT proc uses ds, reg:word, dat:word
        LoadDS
        OutPort _ioaddr
        ret
SBLASTFMOUTLEFT endp

; Send a byte to the right speaker

SBLASTFMOUTRIGHT proc far uses ds, reg:word, dat:word
        LoadDS
        OutPort _ioaddr, 2
        ret
SBLASTFMOUTRIGHT endp

;
; Send a byte to both the left and right speakers
;

SBLASTFMOUTALL proc far reg:word, dat:word
        OutPort 388h
        ret
SBLASTFMOUTALL endp


;
; Send a note to either or both speakers
;

SBLASTFMNOTELEFT proc octave:word, note:word
        DoNote SBLASTFMOUTLEFT
        ret
SBLASTFMNOTELEFT endp

SBLASTFMNOTERIGHT proc octave:word, note:word
        DoNote SBLASTFMOUTALL
        ret
SBLASTFMNOTERIGHT endp

SBLASTFMNOTEALL proc octave:word, note:word
        DoNote SBLASTFMOUTRIGHT
        ret
SBLASTFMNOTEALL endp


;
; Play a block of voice data on the SoundBlaster
;

SBLASTVOCPLAY proc far block:dword, len:word, addr:word, delay:word, docli:word, packtype:word
        push    bx                              ; Save registers
        push    cx
        push    dx
        push    di
        push    ds

        LoadDS                                  ; Load DS reg

        cmp     docli, 0                        ; Don't do a CLI unless
        je      aftercli                        ; the user wants us to!

        cli

aftercli:

        mov     di, word ptr [block]            ; Load offset of block
        mov     ax, word ptr [block+2]          ; Load segment of block
        mov     ds, ax                          ; Put in DS

        mov     cx, len                         ; Get # of bytes to xfer

        mov     dx, addr                        ; Get port address
        add     dx, 0ch                         ; Add to get write status port

        mov     bx, delay                       ; Get delay fudge factor
        or      bx, bx                          ; See if BX is zero
        jnz     short go                        ; If it's non-zero, ok
        mov     bx, 1                           ; Otherwise, make it 1.

go:     WaitDX  Lab1                            ; Wait for port to clear

        mov     al, 10h                         ; Send command (SENDDATA)
        out     dx, al

        WaitDX  Lab2                            ; Wait for port to clear

        mov     al, byte ptr [di]               ; Load byte
        out     dx, al

        inc     di                              ; Next byte

        mov     ax, bx                          ; Put delay in accumulator

;        sti
slow:   dec     ax                              ; Subtract one
        jnz     short slow                      ; If not zero, keep looping
;        cli

        loop    go                              ; Continue playing
                                                ; for CX bytes!

done:   cmp     docli, 0                        ; Don't do a STI unless
        je      aftersti                        ; the user wants us to!

        sti

aftersti:
        xor     ax, ax                          ; Return code 0

getout: pop     ds                              ; Restore registers
        pop     di
        pop     dx
        pop     cx
        pop     bx

        ret
SBLASTVOCPLAY endp


; Turns the SB VOC voice on or off.  'addr' is the base address of the
; card, and 'status' is a boolean flag indicating whether the voice
; should be on or off.

SBLASTVOCENABLE proc uses ds, status:word
        LoadDS

        mov     dx, _ioaddr
        add     dx, 0ch

        WaitDX  Lab1

        mov     al, 0d1h                ; Default to on
        cmp     status, 0               ; Does user want it off?
        jne     doit                    ; It's already on, so go ahead

        mov     al, 0d3h                ; OK, turn it off

doit:   out     dx, al
        ret
SBLASTVOCENABLE endp



; Initialize the soundblaster.  Returns 0 if the initialization was
; successful, or -1 if there was an error (such as no SB installed).
;
; Under OS/2, this also requests access to the sound ports.

SBLASTINIT proc uses ds, ioaddr:word, irq:word
        mov     bx, ioaddr                      ; Load addr, irq and cli flag
        mov     cx, irq

        LoadDS                                  ; Load DS reg

        mov     _ioaddr, bx                     ; Dump the stuff in our data
        mov     _irq, cx                        ; segment

        ; Now detect the blaster to see if it's there!

        mov     al, 1                   ; Send a 01h to the 226h port
        mov     dx, bx                  ; Load address port
        add     dx, 6                   ; Add 6
        out     dx, al

        in      al, dx                  ; Get four bytes
        in      al, dx
        in      al, dx
        in      al, dx

        mov     al, 0                   ; Send a zero to the status port
        out     dx, al

        add     dx, 4                   ; Now check for a response at 22ah.
        mov     cx, 100

check:  in      al, dx
        cmp     al, 0AAh                ; If we got an 0aah, the SB is here!
        je      short gotit
        loop    check

        mov     ax, -1                  ; We fell thru the loop w/o finding,
        jmp     short done              ; so the SB must not be here.

gotit:  
IFDEF OS_2
        ; int DosPortAccess(usRsvd, fRelease, uspFirst, uspLast);
        ;
        ; Request access to the SoundBlaster ports
        ;

        push    0                               ; usReserved = 0
        push    0                               ; fRelease = FALSE
        mov     ax, _ioaddr
        push    ax                              ; uspFirst
        add     ax, 17h
        push    ax                              ; uspLast
        call    far ptr DOSPORTACCESS

        or      ax,ax                           ; Test return code
        jz      okay                            ; If it's 0, we're ok!

        mov     ax, -1                          ; Otherwise, return error!
        jmp     short done                      ; Back to caller
okay:
ENDIF
        xor     ax,ax
done:   ret
SBLASTINIT endp



;
; Release access to the sound ports
;

SBLASTTERM proc far uses ds
IFDEF OS_2
        LoadDS

        ; int DosPortAccess(usRsvd, fRelease, uspFirst, uspLast);
        ;
        ; Now release access to the requested ports.
        ;

        push    0                               ; usReserved = 0
        push    1                               ; fRelease = TRUE
        mov     ax, _ioaddr
        push    ax                              ; uspFirst
        add     ax, 17h
        push    ax                              ; uspLast
        call    far ptr DOSPORTACCESS
ENDIF
        ret
SBLASTTERM endp

end


