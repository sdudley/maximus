;*# name=DOS compiler-independent getchar routines
;/

       include  model.inc

;        .DATA
;        public  last_scan
;
;last_scan       db      0

_BSS            SEGMENT PARA PUBLIC  'BSS'
                PUBLIC  last_scan
last_scan       db      0
_BSS            ENDS


        .CODE

        PUBLIC KGETCH, KPEEK, KHIT

KGETCH proc

IFDEF __FLAT__
        xor     eax,eax
ELSE
        xor     ah,ah                   ; Clear high byte
ENDIF
        mov     al,ss:last_scan         ; See if there was a scan code saved
        or      al,al                   ; from last time.
        jnz     gscan

        xor     ah,ah                   ; Get a character from the BIOS
        int     16h

        cmp     al,0                    ; Check to see if anything in low
        jne     short gotch             ; byte set.  If so, get out.

        mov     byte ptr ss:last_scan,ah; Store it for later
gotch:  xor     ah,ah                   ; Clear high byte
        jmp     short doneg

gscan:  mov     ss:last_scan,0
doneg:  ret
KGETCH endp

KPEEK proc

IFDEF __FLAT__
        xor     eax, eax
ELSE
        xor     ah, ah
ENDIF

        mov     al, ss:last_scan        ; See if there was a scan code saved
        or      al, al                  ; from last time.
        jne     donep

        mov     ah,1                    ; Get keyboard status from BIOS
        int     16h
        jz      nochar

        xor     ah,ah                   ; Clear high byte, if any
        jmp     short donep

IFDEF __FLAT__
nochar: mov     eax,-1
ELSE
nochar: mov     ax,-1
ENDIF

donep:  ret
KPEEK endp


IFDEF __FLAT__

KHIT proc
        call    KPEEK
        cmp     eax,-1
        je      short none

        mov     eax,1                   ; Got one
        jmp     short doneh

none:   xor     eax,eax

doneh:  ret
KHIT endp

ELSE

KHIT proc
        call    KPEEK
        cmp     ax,-1
        je      short none

        mov     ax,1                    ; Got one
        jmp     short doneh

none:   xor     ax,ax

doneh:  ret
KHIT endp

ENDIF

end

