;
;	--- Version 3.3 92-08-23 12:09 ---
;
;	CHECKPAT.ASM - Utility function to check a given file/path,
;			and to resolve an incomplete path.
;			(Does not use any undocumented DOS functions.)
;
;	Public Domain Software written by
;		Thomas Wagner
;		Ferrari electronic GmbH
;		Beusselstrasse 27
;		D-1000 Berlin 21
;		Germany
;
;
; Assemble with
;
; tasm  /DPASCAL checkpat,checkpap  	     - Turbo Pascal (Tasm only), near
; tasm  /DPASCAL /DFARCALL checkpat,checkpap - Turbo Pascal (Tasm only), far
; ?asm  checkpat;		  	     - C, default model (small)
; ?asm  /DMODL=large checkpat  		     - C, large model
;
;	NOTE:	For C, change the 'model' directive below according to your
;		memory model, or define MODL=xxx on the command line.
;
;		For Turbo C Huge model, you must give /DTC_HUGE on the
;		command line, or define it here.
;
;
;   This routine accepts a file name and/or path, checks and resolves the
;   path, and splits the name into its components.
;
;   A relative path, or no path at all, is resolved to a full path
;   specification. An invalid disk drive will not cause the routine 
;   to fail.
;
;   PASCAL:
;	function checkpath (name    : string,
;			    inflags : integer,
;	                    drive   : string,
;	                    dir     : string,
;	                    fname   : string,
;	                    ext     : string,
;	                    fullpath: string)
;			    : integer;
;   C:
;	int checkpath (char *name,     
;		       int  inflags,
;	               char *drive,      
;	               char *dir,        
;	               char *fname,      
;	               char *ext,        
;	               char *fullpath);  
;
;   Parameters:
;
;	name	 - Input:  file name and/or path string
;	inflags  - Input:  parameters for analysis
;			   INF_NODIR: do no interpret name as directory
;	drive	 - Output: drive letter, with trailing colon
;	dir	 - Output: directory, with leading and trailing bakslash
;	fname	 - Output: file name
;	ext	 - Output: file extension, with leading dot
;	fullpath - Output: combined path
;
;	NOTE:	The input 'name' and the output 'fullpath' parameters may
;		both point to the same string. The 'fullpath' pointer will
;		only be used after completion of input parsing.
;
;	NOTE:	For Pascal, reserve one character in addition to the
;		standard maximum length for all strings, including the
;		input parameter. All strings will be zero-terminated
;		during internal processing. DO NOT pass a constant as
;		input parameter string.
;
;   Returns:
;
;	A negative value on error:
;
;		ERR_DRIVE       -1    Invalid drive
;		ERR_PATH        -2    Invalid path
;		ERR_FNAME       -3    Malformed filename
;		ERR_DRIVECHAR   -4    Illegal drive letter
;		ERR_PATHLEN     -5    Path too long
;		ERR_CRITICAL    -6    Critical error (invalid drive, drive not ready)
;
;	On error, all output strings except the drive string will be empty.
;
;	If no error occured, a positive value consisting of the bitwise OR 
;	of the following flags:
;
;		HAS_WILD        1     Filename/ext has wildcard characters
;		HAS_EXT         2     Extension specified
;		HAS_FNAME       4     Filename specified
;		HAS_PATH        8     Path specified
;		HAS_DRIVE       0x10  Drive specified
;
;			Those flags are set only if the corresponding 
;			component was given in the input string. The 
;			'drive' and 'dir' strings will always contain 
;			the resolved drive and path.
;
;		FILE_EXISTS     0x20  File exists, upper byte has attributes
;		IS_DIR        0x1000  Directory, upper byte has attributes
;
;		The file attributes returned if FILE_EXISTS or IS_DIR 
;		is set are
;			0x0100 - Read only
;			0x0200 - Hidden
;			0x0400 - System
;			0x2000 - Archive
;			0x4000 - Device
;
;
;
	IFDEF	PASCAL
	.model	tpascal
	IFDEF	FARCALL
	%out	Pascal, far calls
	ELSE
	%out	Pascal, near calls
	ENDIF
ptrsize	=	1
	ELSE
	IFNDEF	MODL
	.model	small,c
	%out	small model
	ELSE
%	.model	MODL,c
%	%out	MODL model
	ENDIF
ptrsize	=	@DataSize
	ENDIF
;
ldds	macro	reg,var
	IF	ptrsize
	lds	reg,var
	ELSE
	mov	reg,var
	ENDIF
	endm
;
lddsf	macro	reg,var
	IF	ptrsize
	lds	reg,var
	ELSE
	mov	ds,dseg
	mov	reg,var
	ENDIF
	endm
;
ldes	macro	reg,var
	IF	ptrsize
	les	reg,var
	ELSE
	mov	reg,var
	ENDIF
	endm
;
ldesf	macro	reg,var
	IF	ptrsize
	les	reg,var
	ELSE
	mov	es,dseg
	mov	reg,var
	ENDIF
	endm
;
	public	checkpath
	public	exists
;
ERR_DRIVE	=	-1
ERR_PATH	=	-2
ERR_FNAME	=	-3
ERR_DRIVECHAR	=	-4
ERR_PATHLEN	=	-5
ERR_CRITICAL	=	-6
;
HAS_WILD	=	1
HAS_EXT		=	2
HAS_FNAME	=	4
HAS_PATH	=	8
HAS_DRIVE	=	10h
FILE_EXISTS	=	20h
IS_DIR		=	1000h
;
MAXPATH		=	66
;
INF_NODIR	=	1
;
	IFDEF	PASCAL
	.data
	ELSE
	IFDEF	TC_HUGE
	.fardata?	checkpat_data
	ELSE
	.data?
	ENDIF
	ENDIF
;
save24		label	dword
sav24_off	dw	?
sav24_seg	dw	?
fail		dw	?
dfltpath	db	68 dup(?)
;
	.code
;
@strcpy	proc	near
	lodsb
	stosb
	or	al,al
	jnz	@strcpy
	dec	di
	ret
@strcpy	endp
;
	IFDEF	PASCAL
@strlen	proc	near
	push	di
	inc	di
	xor	ax,ax
	mov	cx,-1
	repne scasb
	not	cx
	dec	cx
	pop	di
	mov	es:[di],cl
	ret
@strlen	endp
	ENDIF
;
@int24_handler	proc far
	push	ds
	IFDEF	TC_HUGE
	mov	ax,SEG checkpat_data
	ELSE
	IFDEF	PASCAL
	mov	ax,SEG fail
	ELSE
	mov	ax,SEG DGROUP
	ENDIF
	ENDIF
	mov	ds,ax
	mov	fail,1
	pop	ds
	xor	ax,ax		; ignore (for compatibility with DOS < 3.1)
	iret
@int24_handler	endp
;
;
	IFDEF	PASCAL
	IFDEF	FARCALL
checkpath PROC far uses ds, string: dword, inflags: word, drive: dword, path: dword, fname: dword, ext: dword, fullpath: dword
	ELSE
checkpath PROC near uses ds, string: dword, inflags: word, drive: dword, path: dword, fname: dword, ext: dword, fullpath: dword
	ENDIF
	ELSE
checkpath PROC uses ds si di, string: ptr byte, inflags: word, drive: ptr byte, path: ptr byte, fname: ptr byte, ext: ptr byte, fullpath: ptr byte
	ENDIF
	local	drv: word,flags:word,dseg: word
;
	IFDEF	TC_HUGE
	mov	ax,SEG my_data
	mov	ds,ax
	ENDIF
	mov	dseg,ds
	IFDEF	PASCAL
	cld
	ENDIF
;
;	save old critical error handler, install own
;
	mov	ax,3524h
	int	21h
	mov	sav24_off,bx
	mov	sav24_seg,es
	mov	fail,0
	mov	ax,cs
	mov	ds,ax
	mov	dx,offset @int24_handler
	mov	ax,2524h
	int	21h
	mov	ds,dseg
;
;	Init output strings & flags
;
	xor	ax,ax
	mov	flags,ax
	ldesf	di,path
	IFDEF	PASCAL
	stosw
	ELSE
	stosb
	ENDIF
	ldes	di,fname
	IFDEF	PASCAL
	stosw
	ELSE
	stosb
	ENDIF
	ldes	di,ext
	IFDEF	PASCAL
	stosw
	ELSE
	stosb
	ENDIF
;
;------ Check the drive. If none was given, use current drive.
;
	ldes	di,drive
	IFDEF	PASCAL
	inc	di
	ENDIF
	ldds	si,string
	IFDEF	PASCAL
; for pascal, zero-terminate input string
	lodsb
	mov	bl,al
	xor	ah,ah
	mov	[si+bx],ah
	ENDIF
;
; skip leading whitespace
;
skip_space:
	lodsb
	cmp	al,' '
	je	skip_space
	cmp	al,09h
	je	skip_space
	dec	si
;
	or	al,al
	jz	no_drive
	cmp	byte ptr 1[si],':'
	je	drive_there
;
no_drive:
	mov	ah,19h			; get default drive
	int	21h
	add	al,'A'
	stosb
	mov	bl,al
	mov	al,':'
	stosb
	jmp	short check_drive
;
badchar:
	mov	ax,ERR_DRIVECHAR
	jmp	error_exit
;
drive_there:
	inc	si
	and	al,NOT 20h		; convert to uppercase
	cmp	al,'A'
	jb	badchar
	cmp	al,'A'+1fh		; Novell allows some strange chars
	ja	badchar
	stosb
	or	flags,HAS_DRIVE
	mov	bl,al
	movsb
;
;	Now check the drive for validity by getting the current directory
;	of this drive (we need it anyway later on)
;
check_drive:
	and	bx,1fh
	mov	drv,bx
	xor	al,al
	stosb
;
	push	ds
	push	si
;
	mov	ax,dseg
	mov	ds,ax
	mov	es,ax
	mov	si,offset dfltpath+3
	mov	ah,47h		; get current directory
	mov	dx,drv		; drive number
	int	21h
	jc	cpath_bad
	cmp	fail,0
	je	cpath_good
;
;	Get dir returned an error -> the drive is invalid.
;
cpath_bad:
	add	sp,4
	mov	ax,ERR_DRIVE
	jmp	error_exit	; can't continue with invalid drive
;
;	The drive is valid, edit the current path to include
;	leading drive letter, colon, and backslash.
;	Also, copy the path into the path output string, in case no
;	path is given in the input, and append trailing backslash to it.
;
cpath_good:
	mov	di,offset dfltpath
	mov	ax,drv
	add	al,'A'-1
	stosb
	mov	al,':'
	stosb
	mov	si,di		; point to start of path
	mov	al,'\'
	stosb
	ldes	di,path
	IFDEF	PASCAL
	inc	di
	ENDIF
	call	@strcpy
	mov	al,'\'
	cmp	byte ptr es:[di-1],al	; root dir?
	je	drive_ok		; then no second backslash
	cmp	byte ptr es:[di-1],'/'	; forward slash also legal
	je	drive_ok
	stosb				; else append backslash
;
;
;------ Drive checked, is valid. Now separate path and filename.
;
drive_ok:
	pop	si
	pop	ds
	push	si
	push	di
	push	es
	mov	di,si
	mov	ax,ds
	mov	es,ax
;
;	find the end of the string
;
	xor	ax,ax
	mov	cx,-1
	repne scasb
	mov	si,di
	pop	es
	pop	di
	not	cx
	dec	cx
	jnz	has_fnp		; continue if there's more in the string
	add	sp,2
	stosb			; terminate path string
	jmp	check_fname	; exit if no path or filename
;
;	search from the end for slash/backslash
;
has_fnp:
	sub	si,2		; last char of string
	mov	bx,5c2fh	; the two slashes
	std			; backwards scan
;
spath:
	lodsb
	cmp	al,bl
	je	pfound
	cmp	al,bh
	je	pfound
	loop	spath
;
;	no slash/backslash -> no path given
;
	cld
	pop	si
	xor	cx,cx
	jmp	short cfname
;
longpath:
	mov	ax,ERR_PATHLEN
	jmp	error_exit
;
;	copy the path (note: CX has length of path including slash)
;
pfound:
	cld
	pop	si
	cmp	cx,MAXPATH
	ja	longpath
	or	flags,HAS_PATH		; we have a path
	ldes	di,path
	IFDEF	PASCAL
	inc	di
	ENDIF
	push	cx
	rep movsb
	pop	cx
;
;	check for special filenames '.' and '..', and add them to
;	the path if present.
;	The special form that adds a '.' for every level further down
;	the tree is recognized, and translated into the DOS-form '..\'
;
cfname:
	cmp	byte ptr [si],'.'
	jne	path_finished
	cmp	byte ptr [si+1],'.'
	je	is_special
	cmp	byte ptr [si+1],0
	jne	path_finished
;
is_special:
	or	flags,HAS_PATH		; we have a path
	mov	bx,MAXPATH
	sub	bx,cx
	mov	cx,2
	lodsb
;
copy_special:
	or	bx,bx
	jz	longpath
	dec	bx
	stosb
	lodsb
	or	al,al
	jz	special_finished
	cmp	al,'.'
	jne	badname
	loop	copy_special
;
add_special:
	sub	bx,3
	jle	longpath
	mov	al,'\'
	stosb
	mov	al,'.'
	stosb
	stosb
	lodsb
	or	al,al
	jz	special_finished
	cmp	al,'.'
	jne	badname
	jmp	add_special
;
badname:
	mov	ax,ERR_FNAME
	jmp	error_exit
;
special_finished:
	dec	si
	mov	al,'\'
	stosb
;
;	now copy the filename and extension (limited to 8/3 chars)
;
path_finished:
	xor	al,al			; terminate path
	stosb
;
	mov	bx,2a3fh		; the two wildcards '*' and '?'
	ldes	di,fname
	IFDEF	PASCAL
	inc	di
	ENDIF
	mov	cx,8			; max 8 for name
;
cfnloop:
	lodsb
	or	al,al			; end of string?
	jz	cfndot
	cmp	al,'.'
	je	cfndot
	jcxz	cfnloop			; skip if 8 chars copied
	stosb
	dec	cx
	or	flags,HAS_FNAME
	cmp	al,bl			; check for wildcards	
	je	fnwild
	cmp	al,bh
	jne	cfnloop
;
fnwild:
	or	flags,HAS_WILD
	jmp	cfnloop
;
cfndot:
	mov	ah,al			; save terminator (0 or '.')
	xor	al,al
	stosb				; terminate filename
	or	ah,ah
	jz	no_ext			; jump if at end of string
;
;	extension present, copy it.
;
	or	flags,HAS_EXT
	ldes	di,ext
	IFDEF	PASCAL
	inc	di
	ENDIF
	mov	cx,3
	mov	al,ah
	stosb				; store '.' as first ext char
;
cextloop:
	lodsb
	or	al,al
	jz	cextend
	stosb
	cmp	al,bl			; check for wildcards	
	je	extwild
	cmp	al,bh
	jne	cextcont
;
extwild:
	or	flags,HAS_WILD
;
cextcont:
	loop	cextloop
;
cextend:
	xor	al,al
	stosb				; terminate extension
;
;
no_ext:
	test	flags,HAS_PATH
	jz	check_fname
;
;	A path was specified, check it:
;	Change the current directory to the one specified. 
;	If valid, read back the new directory string 
;	(which has '.' and '..' resolved).
;	In any case, restore the current directory.
;
	ldes	di,fullpath	; make path string from drive and path
	IFDEF	PASCAL
	inc	di
	ENDIF
	ldds	si,drive
	IFDEF	PASCAL
	inc	si
	ENDIF
	call	@strcpy
	ldds	si,path
	IFDEF	PASCAL
	inc	si
	ENDIF
	call	@strcpy
	cmp	byte ptr es:[di-2],':'	; root dir ?
	je	no_slstrip		; then don't strip backslash
	mov	byte ptr es:[di-1],0	; else remove trailing '\'
;
no_slstrip:
	xor	cx,cx		; cx is 'path ok' flag (init to not ok)
	ldds	dx,fullpath
	IFDEF	PASCAL
	inc	dx
	ENDIF
	ldes	di,path
	IFDEF	PASCAL
	inc	di
	ENDIF
	mov	ah,3bh		; change current directory
	int	21h
	mov	ds,dseg
	jc	rest_path	; skip dir reading if invalid
	cmp	fail,0
	jne	rest_path
;
;	read back path
;
	ldds	si,path
	IFDEF	PASCAL
	inc	si
	ENDIF
	inc	si		; leave space for leading '\'
	mov	ah,47h		; get current directory
	mov	dx,drv		; drive number
	int	21h
	mov	ds,dseg
	jc	rest_path	; shouldn't happen, but...
	cmp	fail,0
	jne	rest_path
;
	mov	byte ptr es:[di],'\'	; prefix with '\'
	xor	ax,ax
	mov	cx,-1
	repne scasb			; find end of string
	not	cx
	cmp	cx,2
	je	rest_path		; don't append trailing '\' if root
	mov	byte ptr es:[di-1],'\'
	stosb				; terminate
;
rest_path:
	mov	dx,offset dfltpath
	mov	ah,3bh
	int	21h
;
;	was the path ok?
;
	or	cx,cx
	jnz	check_fname
;
;	exit if not
;
	mov	ax,ERR_PATH
	jmp	error_exit
;
;	the path was ok, now check the filename if it doesn't contain
;	wildcard chars.
;
check_fname:
	test	flags,HAS_WILD
	jz	checkfn1
	jmp	ready
;
checkfn1:
	ldesf	di,fullpath	; make full path string
	IFDEF	PASCAL
	inc	di
	ENDIF
	lddsf	si,drive
	IFDEF	PASCAL
	inc	si
	ENDIF
	call	@strcpy
	ldds	si,path
	IFDEF	PASCAL
	inc	si
	ENDIF
	call	@strcpy
	test	flags,HAS_FNAME OR HAS_EXT
	jnz	checkfn2
;
;	No filename, get the attribute of the directory
;
	or	flags,IS_DIR
	dec	di
	mov	byte ptr es:[di],0	; clear trailing '\'
	dec	di
	cmp	byte ptr es:[di],':'	; root dir?
	je	no_dirchk		; then don't get attribute
	ldds	dx,fullpath
	IFDEF	PASCAL
	inc	dx
	ENDIF
	mov	ax,4300h	; get attribute
	int	21h
	mov	ds,dseg
	jc	no_attrib	; shouldn't happen
	cmp	fail,0
	jne	no_attrib
	mov	ax,flags
	mov	ah,cl
	and	ah,7fh
	mov	flags,ax
no_dirchk:
	jmp	ready
;
no_attrib:
	mov	ax,ERR_PATH
	jmp	error_exit
;
checkfn2:
	ldds	si,fname
	IFDEF	PASCAL
	inc	si
	ENDIF
	call	@strcpy
	ldds	si,ext
	IFDEF	PASCAL
	inc	si
	ENDIF
	call	@strcpy
;
	mov	ah,2fh		; get current DTA
	int	21h		; ES:BX has current DTA
	push	bx
	push	es
;
	mov	ds,dseg
	mov	dx,offset dfltpath
	mov	ah,1ah		; set DTA
	int	21h
;
	ldds	dx,fullpath
	IFDEF	PASCAL
	inc	dx
	ENDIF
	mov	cx,10110B	; search all except label
	mov	ah,4eh		; search for first
	int	21h
	mov	ds,dseg
	jc	no_file
	cmp	fail,0
	jne	no_file
;
	mov	ax,flags
	mov	ah,dfltpath+15h	; file attributes into upper byte of flags
	test	ah,10h		; subdirectory?
	jz	no_subdir
;
;	The filename specifies a subdirectory. Append it to the path.
;
	test	inflags,INF_NODIR
	jnz	no_file
	ldesf	di,path
	IFDEF	PASCAL
	inc	di
	ENDIF
	mov	cx,-1
	xor	ax,ax
	repne scasb
	dec	di
	mov	si,offset dfltpath+1eh
	call	@strcpy
	mov	al,'\'
	stosb
	xor	al,al
	stosb
	ldes	di,fname
	IFDEF	PASCAL
	inc	di
	ENDIF
	stosb
	ldes	di,ext
	IFDEF	PASCAL
	inc	di
	ENDIF
	stosb
	mov	ax,flags
	mov	ah,dfltpath+15h	; file attributes into upper byte of flags
	and	ah,7fh		; make sure it's positive
	and	al,NOT (HAS_FNAME OR HAS_EXT)
	or	al,HAS_PATH
	mov	flags,ax
	jmp	short no_file
;
no_subdir:
	or	al,FILE_EXISTS
	and	ah,7fh		; make sure it's positive
	mov	flags,ax
;
no_file:
	pop	ds
	pop	dx
	mov	ah,1ah
	int	21h		; restore DTA
;
ready:
	ldesf	di,fullpath	; make full path string
	IFDEF	PASCAL
	inc	di
	ENDIF
	lddsf	si,drive
	IFDEF	PASCAL
	inc	si
	ENDIF
	call	@strcpy
	ldds	si,path
	IFDEF	PASCAL
	inc	si
	ENDIF
	call	@strcpy
	ldds	si,fname
	IFDEF	PASCAL
	inc	si
	ENDIF
	call	@strcpy
	ldds	si,ext
	IFDEF	PASCAL
	inc	si
	ENDIF
	call	@strcpy
	mov	ds,dseg
	mov	ax,flags
;
error_exit:
	mov	ds,dseg
	cmp	fail,0
	je	nofail
	mov	ax,ERR_CRITICAL
;
nofail:
	push	ax
	cmp	ax,0
	jge	no_error
	ldesf	di,fullpath
	mov	word ptr es:[di],0
;
no_error:
	lds	dx,save24
	mov	ax,2524h
	int	21h
	IFDEF	PASCAL
	ldesf	di,drive
	call	@strlen
	ldes	di,path
	call	@strlen
	ldes	di,fname
	call	@strlen
	ldes	di,ext
	call	@strlen
	ldes	di,fullpath
	call	@strlen
	ENDIF
	pop	ax
;
	ret
;
checkpath endp
;
;
; Returns TRUE if a file with name 'fname' exists.
;
	IFDEF	PASCAL
	IFDEF	FARCALL
exists	PROC far uses ds, fname: dword
	ELSE
exists	PROC near uses ds, fname: dword
	ENDIF
	ELSE
exists	PROC	uses ds, fname: ptr byte
	ENDIF
;
	IFDEF	TC_HUGE
	mov	ax,SEG my_data
	mov	ds,ax
	ENDIF
;
	IFDEF	PASCAL
	ldds	si,fname
; for pascal, zero-terminate input string
	lodsb
	mov	bl,al
	xor	bh,bh
	mov	[si+bx],bh
	mov	dx,si
	ELSE
	ldds	dx,fname
	ENDIF
;
	mov	ax,4300h	; get file attributes
	int	21h
	mov	ax,0
	jc	exists_end
	test	cx,10h		; directory?
	jnz	exists_end
	inc	ax
;
exists_end:
	ret
;
exists	endp
;
	end

