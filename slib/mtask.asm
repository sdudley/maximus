;*# name=DOS multitasker detection and sleep routines
;/

IFNDEF OS_2

IFNDEF __FLAT__

        .model medium, pascal
        .code

PUBLIC DVLOADED, TVLOADED, DVSLEEP, DDOSLOADED, DDOSSLEEP
PUBLIC PCMOSLOADED, PCMOSSLEEP, WINLOADED, WINSLEEP
PUBLIC MLINKLOADED, MLINKSLEEP, SPOOLSLEEP

; Check to see whether or not DV is loaded

DVLOADED proc
        mov     ax,2b01h                ; Installation check
        mov     cx,4445h
        mov     dx,5351h
        int     21h

        cmp     al,0ffh                 ; ffh == dv not installed
        je      nodv

        mov     ax,1                    ; OK, we got it!
        jmp     short donedv

nodv:   xor     ax,ax
donedv: ret
DVLOADED endp


TVLOADED proc
        mov     ax,1022h                ; TopView installation check
        xor     bx,bx
        int     15h

        cmp     bx,0                    ; If non-zero, TV is loaded
        je      notv
        
        mov     ax,1
        jmp     short endtv

notv:   xor     ax,ax
endtv:  ret
TVLOADED endp


; Give away a timeslice to a DESQview or TopView program

DVSLEEP proc
        mov     ax, 101Ah       ; switch to task stack
        int     15h
        mov     ax, 1000h       ; give up CPU time
        int     15h
        mov     ax, 1025h       ; switch back to user stack
        int     15h
        ret
DVSLEEP endp


; DDos's main 'installation check' interrupt is used by a few other
; programs for other purposes, so we call a few other instructions to
; ensure that it actually is here...

DDOSLOADED proc uses es
; DoubleDOS cannot be reliably detected with commented out code below.

        mov     ax,1
        ret

;        mov     ah,0e4h                 ; Installation check
;        int     21h
;        cmp     al,00                   ; If it's zero or above two,
;        je      nodd                    ; DDOS isn't there.
;
;        cmp     al,2
;        ja      nodd
;
;        mov     ah,0e5h                 ; Other partition status... If
;        int     21h                     ; AH > 2, then it's an invalid
;                                        ; return code too.
;        cmp     ah,2
;        ja      nodd
;
;        int     0fch                    ; Get addr of virt display buffer
;        mov     ax,es
;        cmp     ax,0                    ; Make sure the scrnbuf is non-zero
;        je      nodd
;
;        mov     ax,1                    ; It's non-zero, so DD is here
;        jmp     short donedd            ; after all
;
;nodd:   xor     ax,ax
;donedd: ret
        
DDOSLOADED endp


; Give away a timeslice under the DDos environment

DDOSSLEEP proc
        int     0f4h
        ret
DDOSSLEEP endp


; Check to see if PC-MOS is loaded

PCMOSLOADED proc
        mov     ah,30h                  ; Get DOS version
        int     21h

        push    ax                      ; Push vernum on stack
        mov     ax,3000h
        mov     bx,3000h
        mov     cx,3000h
        mov     dx,3000h
        int     21h

        pop     cx                      ; Retrieve into cx reg
        cmp     ax,cx                   ; See if they match
        je      nopcm

        mov     ax,1
        jmp     short donepm

nopcm:  xor     ax,ax
donepm: ret
PCMOSLOADED endp


; Give away a timeslice under PC-MOS

PCMOSSLEEP proc
        mov     ax,0703h
        mov     bx,1
        xor     cx,cx
        xor     dx,dx
        int     38h
        ret
PCMOSSLEEP endp


; Check to see if Windows 3.x is loaded

WINLOADED proc
        mov     ax,1600h                ; Installation check
        int     2fh

        cmp     al,00                   ; Non-zero means win is installed
        je      nowin

        mov     ax,1680h                ; Release timeslice call
        int     2fh
 
        cmp     al,00                   ; If this returns zero, then
        jne     nowin                   ; it ensures that Win is around.

        mov     ax,1
        jmp     short donwin

nowin:  xor     ax,ax
donwin: ret
WINLOADED endp


; Give away a Windows timeslice

WINSLEEP proc
        mov     ax,1680h                ; Give up timeslice for MS-Windows
        int     2fh
        ret
WINSLEEP endp

MLINKLOADED proc uses ds

; Multilink cannot be reliably detected with commented out code below.

        mov     ax,1
        ret

;        xor     ax,ax
;        push    ax
;        pop     ds
;        mov     ax,ds:[01feh]
;        cmp     ax,0
;        je      noml
;
;        mov     ax,1
;        jmp     short doneml
;
;noml:   xor     ax,ax
;doneml: ret
MLINKLOADED endp


MLINKSLEEP proc
        mov     ax,0200h
        int     7fh
        ret
MLINKSLEEP endp


OS2LOADED proc
        mov     ah, 30h         ; Get version number
        int     21h

        cmp     al, 20          ; If version number >= 20, OS/2 2.x is loaded
        jge     os2done
        xor     ax,ax           ; Otherwise, it's not there.

os2done:
        ret
OS2LOADED endp

OS2SLEEP proc
        push    ax              ; Save registers
        push    dx

        xor     dx,dx           ; DosSleep(dx*65536+ax) = DosSleep(2)
        mov     ax,2
        hlt                     ; Call OS/2 function dispatcher using
        db      35h,0cah        ; function code 0x35ca
        pop     dx
        pop     ax
OS2SLEEP endp

; Give away a timeslice to the MS-DOS "keyboard busy" loop, as per INTER290.

SPOOLSLEEP proc
        int     28h
        ret
SPOOLSLEEP endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

ELSE ; __FLAT__

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        .386p
        .model small, pascal
        .code

PUBLIC DVLOADED, TVLOADED, DVSLEEP, DDOSLOADED, DDOSSLEEP
PUBLIC PCMOSLOADED, PCMOSSLEEP, WINLOADED, WINSLEEP
PUBLIC MLINKLOADED, MLINKSLEEP, SPOOLSLEEP


DVLOADED proc
        xor     eax, eax
        mov     ax,2b01h                ; Installation check
        mov     cx,4445h
        mov     dx,5351h
        int     21h

        cmp     al,0ffh                 ; ffh == dv not installed
        je      nodv

        mov     eax,1                   ; OK, we got it!
        jmp     short donedv

nodv:   xor     eax, eax
donedv: ret
DVLOADED endp


TVLOADED proc
        xor     eax, eax
        mov     ax,1022h                ; TopView installation check
        xor     bx,bx
        int     15h

        cmp     bx,0                    ; If non-zero, TV is loaded
        je      notv
        
        mov     eax,1
        jmp     short endtv

notv:   xor     eax,eax
endtv:  ret
TVLOADED endp


; Give away a timeslice to a DESQview or TopView program

DVSLEEP proc
        mov     ax, 101Ah       ; switch to task stack
        int     15h
        mov     ax, 1000h       ; give up CPU time
        int     15h
        mov     ax, 1025h       ; switch back to user stack
        int     15h
        ret
DVSLEEP endp


; DDos's main 'installation check' interrupt is used by a few other
; programs for other purposes, so we call a few other instructions to
; ensure that it actually is here...

DDOSLOADED proc uses es
        xor     eax, eax
        mov     ah,0e4h                 ; Installation check
        int     21h
        cmp     al,00                   ; If it's zero or above two,
        je      nodd                    ; DDOS isn't there.

        cmp     al,2
        ja      nodd

        mov     ah,0e5h                 ; Other partition status... If
        int     21h                     ; AH > 2, then it's an invalid
                                        ; return code too.
        cmp     ah,2
        ja      nodd

        int     0fch                    ; Get addr of virt display buffer
        mov     ax,es
        cmp     ax,0                    ; Make sure the scrnbuf is non-zero
        je      nodd

        mov     eax,1                   ; It's non-zero, so DD is here
        jmp     short donedd            ; after all

nodd:   xor     eax,eax
donedd: ret
        
DDOSLOADED endp


; Give away a timeslice under the DDos environment

DDOSSLEEP proc
        int     0f4h
        ret
DDOSSLEEP endp


; Check to see if PC-MOS is loaded

PCMOSLOADED proc
        xor     eax, eax
        mov     ah,30h                  ; Get DOS version
        int     21h

        push    ax                      ; Push vernum on stack
        mov     ax,3000h
        mov     bx,3000h
        mov     cx,3000h
        mov     dx,3000h
        int     21h

        pop     cx                      ; Retrieve into cx reg
        cmp     ax,cx                   ; See if they match
        je      nopcm

        mov     eax,1
        jmp     short donepm

nopcm:  xor     ax,ax
donepm: ret
PCMOSLOADED endp


; Give away a timeslice under PC-MOS

PCMOSSLEEP proc
        mov     ax,0703h
        mov     bx,1
        xor     cx,cx
        xor     dx,dx
        int     38h
        ret
PCMOSSLEEP endp


; Check to see if Windows 3.x is loaded

WINLOADED proc
        xor     eax, eax
        mov     ax,1600h                ; Installation check
        int     2fh

        cmp     al,00                   ; Non-zero means win is installed
        je      nowin

        mov     ax,1680h                ; Release timeslice call
        int     2fh
 
        cmp     al,00                   ; If this returns zero, then
        jne     nowin                   ; it ensures that Win is around.

        mov     eax,1
        jmp     short donwin

nowin:  xor     eax,eax
donwin: ret
WINLOADED endp


; Give away a Windows timeslice

WINSLEEP proc
        mov     ax,1680h                ; Give up timeslice for MS-Windows
        int     2fh
        ret
WINSLEEP endp

MLINKLOADED proc uses ds
        xor     eax,eax
        ret
MLINKLOADED endp


MLINKSLEEP proc
        ret
MLINKSLEEP endp


; Give away a timeslice to the MS-DOS "keyboard busy" loop, as per INTER290.

SPOOLSLEEP proc
        int     28h
        ret
SPOOLSLEEP endp

ENDIF   ; __FLAT__

ENDIF   ; !OS_2
end

