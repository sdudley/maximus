        extrn  SBLASTINIT:dword
        extrn  SBLASTTERM:dword
        extrn  SBLASTVOCENABLE:dword
        extrn  SBLASTVOCPLAY:dword
        extrn  SBLASTFMOUTLEFT:dword
        extrn  SBLASTFMOUTRIGHT:dword
        extrn  SBLASTFMOUTALL:dword
        extrn  SBLASTFMNOTEALL:dword
        extrn  SBLASTFMNOTELEFT:dword
        extrn  SBLASTFMNOTERIGHT:dword

.286p
.model large, pascal

.data
junk    dw      0

.code
        public  SBLASTINIT32
        public  SBLASTTERM32
        public  SBLASTVOCENABLE32
        public  SBLASTVOCPLAY32
        public  SBLASTFMOUTLEFT32
        public  SBLASTFMOUTRIGHT32
        public  SBLASTFMOUTALL32
        public  SBLASTFMNOTEALL32
        public  SBLASTFMNOTELEFT32
        public  SBLASTFMNOTERIGHT32

SBLASTINIT32 proc ioaddr:word, irq:word
        push    word ptr [ioaddr]
        push    word ptr [irq]
        call    far ptr SBLASTINIT
        ret
SBLASTINIT32 endp


SBLASTTERM32 proc
        call    far ptr SBLASTTERM
        ret
SBLASTTERM32 endp


SBLASTVOCENABLE32 proc status:word
        push    word ptr [status]
        call    far ptr SBLASTVOCENABLE
        ret
SBLASTVOCENABLE32 endp

SBLASTVOCPLAY32 proc far block:dword, len:word, addr:word, delay:word, docli:word, packtype:word
        push    word ptr [block+2]
        push    word ptr [block]
        push    word ptr [len]
        push    word ptr [addr]
        push    word ptr [delay]
        push    word ptr [docli]
        push    word ptr [packtype]
        call    far ptr SBLASTVOCPLAY
        ret
SBLASTVOCPLAY32 endp

SBLASTFMOUTLEFT32 proc reg:word, dat:word
        push    word ptr [reg]
        push    word ptr [dat]
        call    far ptr SBLASTFMOUTLEFT
        ret
SBLASTFMOUTLEFT32 endp

SBLASTFMOUTRIGHT32 proc reg:word, dat:word
        push    word ptr [reg]
        push    word ptr [dat]
        call    far ptr SBLASTFMOUTRIGHT
        ret
SBLASTFMOUTRIGHT32 endp

SBLASTFMOUTALL32 proc reg:word, dat:word
        push    word ptr [reg]
        push    word ptr [dat]
        call    far ptr SBLASTFMOUTALL
        ret
SBLASTFMOUTALL32 endp

SBLASTFMNOTEALL32 proc octave:word, note:word
        push    word ptr [octave]
        push    word ptr [note]
        call    far ptr SBLASTFMNOTEALL
        ret
SBLASTFMNOTEALL32 endp

SBLASTFMNOTELEFT32 proc octave:word, note:word
        push    word ptr [octave]
        push    word ptr [note]
        call     far ptr SBLASTFMNOTELEFT
        ret
SBLASTFMNOTELEFT32 endp

SBLASTFMNOTERIGHT32 proc octave:word, note:word
        push    word ptr [octave]
        push    word ptr [note]
        call     far ptr SBLASTFMNOTERIGHT
        ret
SBLASTFMNOTERIGHT32 endp

end

