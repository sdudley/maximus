;--------------------------------------------------------------------
;
;	SYSID.ASM
;
;	Version 4.7
;
;	Two subprograms used by SYSID.PAS:
;
;		CPUID		- identifies host CPU and NDP (if
;					any)
;		DISKREAD	- reads absolute sectors from disk
;
;	Steve Grant
;	Long Beach, CA
;	July 31, 1989
;
;--------------------------------------------------------------------

;       dosseg
        .model medium, c
        .code

false   equ     0
true    equ     1

pFUNK   equ     0
p8088   equ     1
pr8086  equ     2
pV20    equ     3
pV30    equ     4
p80188  equ     5
p80186  equ     6
p80286  equ     7
p80386  equ     8


Get_CPU_Type proc

; interrupt of multi-prefix string instruction

	sti
	mov	cx, 0FFFFH
	rep	lods	byte ptr es : [si]
	jcxz	cpu_03

	call	piq
	cmp	dx, 4
	jne	cpu_01

        mov     ax, p8088
	ret

cpu_01:
	cmp	dx, 6
	je	cpu_02

	jmp	cpu_12

cpu_02:
        mov     ax, pr8086
	ret

cpu_03:

; number of bits in displacement register used by shift

	mov	al, 0FFH
	mov	cl, 20H
	shl	al, cl
	or	al, al
	jnz	cpu_05

	call	piq
	cmp	dx, 2;		Was 4, cbf
	jne	cpu_04

        mov     ax, pV20
	ret

cpu_04:
	cmp	dx, 4;		Was 6, cbf
	jne	cpu_12

        mov     ax, pV30
	ret

cpu_05:

; order of write/decrement by PUSH SP

	push	sp
	pop	ax
	cmp	ax, sp
	je	cpu_07

	call	piq
	cmp	dx, 4
	jne	cpu_06

        mov     ax, p80188
	ret

cpu_06:
	cmp	dx, 6
	jne	cpu_12

        mov     ax, p80186
	ret

cpu_07:

; try to alter flag register bits 15-12

	pushf
	pop	ax
	mov	cx, ax
	xor	cx, 0F000H
	push	cx
	popf
	pushf
	pop	cx
	cmp	ax, cx
	jne	cpu_08

        mov     ax, p80286
	ret

cpu_08:
        mov     ax, p80386
	ret

; BIX ibm.at/hardware #4663

cpu_12:
        mov     ax, pFUNK
	ret

Get_CPU_Type endp

;--------------------------------------------------------------------

piq	proc near

; 	On exit:
;
;		DX	= length of prefetch instruction queue
;
;	This subroutine uses self-modifying code but can nevertheless
;	be run repeatedly in the course of the calling program.

count	=	7
opincdx	equ	42H			; inc dx opcode
opnop	equ	90H			; nop opcode

	mov	al, opincdx
	mov	cx, count
	push	cx
	push	cs
	pop	es
	mov	di, offset piq_01 - 1
	push	di
	std
	rep stosb
	mov	al, opnop
	pop	di
	pop	cx
	xor	dx, dx
	cli
	rep stosb
	rept	count
	inc	dx
	endm

piq_01:
	sti
	ret

piq	endp

end

