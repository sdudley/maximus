
IFDEF OS_2
IFDEF __FLAT__

.386p
extrn   DOSSELTOFLAT:byte
extrn   DOSFLATTOSEL:byte

.model small,c
.code

; Convert a 16:16 address to a flat 32 address

thunk16to32 proc adr16:dword
        mov     eax,adr16
        or      eax,eax
        jz      done16

        call    near ptr DOSSELTOFLAT

done16:
        ret
thunk16to32 endp



; Convert a flat 32 address to a 16:16 address

thunk32to16 proc adr32:dword

        mov     eax,adr32
        or      eax,eax
        jz      done32

        call    near ptr DOSFLATTOSEL

done32:

        ret
thunk32to16 endp

ENDIF
ENDIF

end


