page 60, 132

;   SWAP.ASM        Version 3.00    October 4, 1990
;
;   Contains code and data needed to swap most of the current program out
;   to extended memory, expanded memory, or disk; execute another program;
;   and re-load the original program back into memory.
;
;   Copyright (C) 1990 by Marty Del Vecchio
;   Released to the public domain for free use by all
;   Product is supplied as is and author disclaims all warranties,
;   explicit or implied, about the functionality of the source code
;   or object modules supplied, and shall not be held responsible
;   for any damages caused by use of product.
;
;   Code to parse default FCB's written and generously donated
;   by David E. Jenkins (jenkins@wang.com or dave.jenkins@office.wang.com).
;
;   Contributions not solicited.  Just appreciate the fact that somebody
;   took the time to write and comment this code.  If you have any
;   questions, please contact me at:
;
;   Marty Del Vecchio                   Channel 1 BBS
;   99 Marlboro Road                    Boston, MA
;   Southborough, MA  01772             (617) 354-8873
;   (508) 485-9718
;
;   internet:  marty@bsn.mceo.dg.com
;
;   For information about the contents of this file, see the accompanying
;   file SWAP.DOC.
;


; 'DOSSEG' gives us support for Microsoft C and Turbo C segment naming
; and ordering schemes
DOSSEG

; Figure out which memory model we're assembling for.  Specified on
;  MASM command line with /D followed by either _Small, _Compact, _Medium,
;  or _Large.  If none specified, _Small is assumed.

; Once the model is defined, MASM provides two definitions, @codesize
;  and @datasize, to determine the size of code and data pointers.  If
;  @codesize is 0 (Small and Compact), there is one code segment, and
;  code addresses are 16 bits (offset only).  If @codesize is 1 (Medium
;  and Large), there are multiple code segments, and code addresses are
;  32 bits (segment and offset).  Similarly, @datasize of 0 means one
;  data segment (16-bit pointers), and @datasize of 1 means multiple
;  data segments (32-bit pointers).

IFDEF _large
   .MODEL Large, C
   IF1
      %out Assembling for C, Large memory model
   ENDIF
ELSE
   IFDEF _compact
      .MODEL Compact, C
      IF1
         %out Assembling for C, Compact memory model
      ENDIF
   ELSE
      IFDEF _medium
         .MODEL Medium, C
         IF1
            %out Assembling for C, Medium memory model
         ENDIF
      ELSE
         .MODEL Small, C
         IF1
            %out Assembling for C, Small memory model
         ENDIF
      ENDIF
   ENDIF
ENDIF

; Report whether multiple DOS memory blocks will be swapped
IF1
   IFDEF NOFRAG
      %out Multiple DOS memory blocks will NOT be swapped
   ELSE
      %out Multiple DOS memory blocks will be swapped
   ENDIF
ENDIF

; Figure out which save method we are using--EMS, XMS, disk, or a
;  combination.

; Specified on MASM command line with /D followed by either "xms", "ems",
;  "disk", or "all".  For example, to create a swap() that will try using
;  XMS and EMS, you would use "masm swap.asm /Dems /Dxms".

; If none specified, it will use all.  To change the order in which swap()
;  attempts to save the program to different places, see the function
;  save_program below.

; First, see if they want all of them...
IFDEF all
   USE_DISK  EQU 1
   USE_XMS   EQU 1
   USE_EMS   EQU 1
ELSE
   ; /Dall not specified--try each individually...
   IFDEF disk
      USE_DISK  EQU 1
   ENDIF

   IFDEF xms
      USE_XMS   EQU 1
    ENDIF

   IFDEF ems
      USE_EMS   EQU 1
   ENDIF

ENDIF

; Now see if they declared anything--if not, it will use them all
IFNDEF USE_DISK
   IFNDEF USE_EMS
      IFNDEF USE_XMS
         USE_DISK  EQU 1
         USE_XMS   EQU 1
         USE_EMS   EQU 1
      ENDIF
   ENDIF
ENDIF

; Constant definitions for easier reading
STDERR          equ     2           ; Standard DOS file handle for error output
GET_VECTOR      equ     35h         ; DOS function to get interrupt vector
EMM_INT         equ     67h         ; EMS interrupt vector
EMM_NAME_LEN    equ     8           ; Length of EMS device driver name
MAX_DOS_CMD     equ     127         ; Maximum DOS command-line length

; If we will swap out all DOS memory blocks a program owns, we need a
;   place to store information about them
MAX_EXTRA       equ     16          ; Maximum number of extra DOS allocation blocks to swap

dos_block       struc               ; Structure for extra DOS memory blocks
block_seg       dw      0           ; User's segment address of block
block_size      dw      0           ; Size in paragraphs of block
dos_block       ends


bptr            equ     byte ptr    ; Means we're loading/storing 8 bits
wptr            equ     word ptr    ; Means we're loading/storing 16 bits
dptr            equ     dword ptr   ; Means we're loading/storing 32 bits


; All code and data must be in the code segment, which is the first segment
;  in all Turbo C, Turbo C++, and Microsoft C memory models.

; If we are in the Medium or Large models, there are multiple code segments.
;  If this is the case, our default code segment name will be "SWAP_TEXT".
;  This is acceptable in most cases, except when using the Turbo C integrated
;  development environment.  See SWAP.DOC for details.

; If you are using Turbo C's Integrated Development Environment, the line
;  right after "IF @codesize" MUST say ".CODE  _TEXT"!!!!!!!!!!!!!!!!!!!!
IF @codesize
  IFDEF __WATCOMC__
    %out Compiling for WC
    .CODE   SWAP_TEXT
  ELSE
    %out Compiling for TC
    .CODE   _TEXT
  ENDIF
ELSE
  .CODE
ENDIF

; *****************************************************************************
; Our resident data declarations--this data will be needed after the swap
;  has occurred, and thus must be above the resident line
; *****************************************************************************

; *****************************************************************************
; First, all variables that will be used by all versions assembled from
; this source file, regardless of what save options are selected
; *****************************************************************************
ret_code    dw      0           ; Return code (to C caller) of this swap routine
                                ;   0 = success
                                ;   1 = unable to shrink DOS memory allocation
                                ;   2 = unable to save program to EMS
                                ;   3 = unable to execute requested program
                                ; These values must be the same as those listed
                                ;  in SWAP.H!!!!!!!!!

; *****************************************************************************
; Variables that deal with DOS' memory allocation blocks
old_size    dw      0           ; The old size (in paragraphs) of this program
new_size    dw      0           ; The new "resident" size, doesn't include code/data swapped
prog_size   dw      0           ; Size in paragraphs of saved part of program block (old_size - new_size)
total_paras dw      0           ; Size (in paragraphs) of all blocks combined
my_psp      dw      0           ; This program's Program Segment Prefix (PSP)
mcb_psp     dw      0           ; The PSP address in this program's memory block
start_seg   dw      0           ; Segment address of released memory

; If we are swapping all DOS memory blocks a program owns, we store
;  them in this array of structures
IFNDEF NOFRAG
extra_count dw      0           ; # of extra blocks to save (not including program block)
dos_blocks  dos_block MAX_EXTRA dup (<>)    ; Array for extra blocks
ENDIF
; *****************************************************************************

; *****************************************************************************
; Variable used during the save/restore process
handle      dw      0           ; EMS/XMS/disk file handle
; *****************************************************************************

; *****************************************************************************
; A temporary stack in our code segment, and associated variables
old_sp      dw      0               ; Place to save this program's stack
old_ss      dw      0               ;  information while executing new program

; XMS driver needs a large stack (at least 256 bytes free when called)
IFDEF USE_XMS
new_stack   db      320 dup ('?')   ; Temporary stack we can address after swap
ELSE
new_stack   db      128 dup ('?')   ; Temporary stack we can address after swap
ENDIF
new_sp      label   word            ; Point SP to "top" of stack
; *****************************************************************************

; *****************************************************************************
; Variables that deal with the execution of the new program
prog_name   db      128 dup (0)     ; Storage for name of program to execute
cmd_pad     db      0               ; Maintain word-alignment for variables
cmd_len     db      0               ; Storage for length of command line parameters
cmd_line    db      128 dup (0)     ; Storage for command line parameters

param_blk   label   byte            ; Program Parameter Block--pass to DOS on exec call
env_seg     dw      0               ; Environment segment address, 0 means a COPY of ours
cmd_ofs     dw      offset @code:cmd_len    ; Offset address of command line
cmd_seg     dw      seg cmd_line    ; Segment address of command line
fcb_5C_ofs  dw      offset fcb5C    ; Far pointers to default FCB's.  Some
fcb_5C_seg  dw      seg fcb5C       ;  programs (such as DOS' CHKDSK.COM)
fcb_6C_ofs  dw      offset fcb6C    ;  depend on these being parsed from
fcb_6C_seg  dw      seg fcb6C       ;  the command line before the EXEC call
; *****************************************************************************

; *****************************************************************************
; Variables needed to parse the command line into the default FCB's
c_l_length  dw      0               ; Command line length
si_5C       dw      0               ; Save area for pointer to cmd line arg 1
si_6C       dw      0               ; Save area for pointer to cmd line arg 2

; Default FCB to be passed to PSP offset 5C (hex)
fcb5C       label   byte
fcb5C_drive db      0               ; drive
fcb5C_fname db      8 dup (?)       ; file name
fcb5C_ext   db      3 dup (?)       ; extension
fcb5C_pad   db      4 dup (?)       ; unused

; Default FCB to be passed to PSP offset 6C (hex)
fcb6C       label   byte
fcb6C_drive db      0               ; drive
fcb6C_fname db      8 dup (?)       ; file name
fcb6C_ext   db      3 dup (?)       ; extension
fcb6C_pad   db      4 dup (?)       ; unused
; *****************************************************************************

exec_ret    db      0               ; Return code from executed program
exec_pad    db      0               ; Maintain word-alignment for variables
restore_proc dw     0               ; Address of appropriate restore routine

; *****************************************************************************
; Message to display to screen when we can't reload program
abort_msg   db      0dh, 0ah, 'SWAP: Unable to reload program.', 0dh, 0ah
abort_len   dw      $ - offset @code:abort_msg
; *****************************************************************************

; *****************************************************************************
; Next, the variables needed only for certain versions of the routine,
;  depending on which save/restore options are chosen
; *****************************************************************************

; *****************************************************************************
; Variables needed only when swapping to XMS
IFDEF USE_XMS
XMS_proc    dd      0               ; Address of XMS entry point

XMS_struc       label   byte        ; Structure needed to move memory with XMS
XMS_size        dd      0           ; # of bytes to move (must be even)
XMS_from        dw      0           ; Handle of source, 0=conventional memory
XMS_from_addr   dd      0           ; Address of source memory
XMS_to          dw      0           ; Handle of destionation, 0=conventional memory
XMS_to_addr     dd      0           ; Address of destination memory
ENDIF
; *****************************************************************************

; *****************************************************************************
; Variables needed only when swapping to EMS
IFDEF USE_EMS
pages_used  db      0           ; # of pages of EMS used
emm_name    db      'EMMXXXX0'  ; Name of EMS device driver

EMS_struc   label   byte        ; Structure needed to move memory with EMS 4.0+
EMS_size    dd      0           ; # of bytes to move
EMS_from    db      0           ; Type of source memory (0 = conventional, 1 = expanded)
EMS_from_h  dw      0           ; Source memory handle (0 = conventional)
EMS_from_o  dw      0           ; Offset of source memory (expanded = 0-16K, conventional = 0-64K)
EMS_from_s  dw      0           ; Segment/page of source (expanded = logical page, conventional = segment)
EMS_to      db      0           ; Type of desination memory (0 = conventional, 1 = expanded)
EMS_to_h    dw      0           ; Destination memory handle (0 = conventional)
EMS_to_o    dw      0           ; Offset of destination memory (expanded = 0-16K, conventional = 0-64K)
EMS_to_s    dw      0           ; Segment/page of destination (expanded = logical page, conventional = segment)

ems_offset  dd      0           ; Destination pointer--absolute byte offset into handle
ENDIF
; *****************************************************************************

; *****************************************************************************
; Variables needed only when swapping to disk
IFDEF USE_DISK
fname       db      80 dup (0)  ; Name of the file data is saved to/read from
paras_left  dw      0           ; temporary counter
ENDIF
; *****************************************************************************



; *****************************************************************************
; Version-dependent code--only assemble the routine to restore the program
; from each media (XMS, EMS, disk) if it was specified on the command line
; *****************************************************************************


; *****************************************************************************
; restore_xms   Attempts to restore program from XMS extended memory
;
; Entry:        DS points to our variables
;               Program was saved to XMS extended memory (block referred to by handle)
;
; Return:       Carry set on error, carry clear on success
; *****************************************************************************
IFDEF USE_XMS
restore_xms     proc    near
                push    es

                assume  ds:@code                    ; Tell MASM that DS points to our variables

; First, attempt to restore the portion of the program block that was saved
xms_prog_rest:  mov     ax, wptr start_seg          ; Released segment address
                mov     es, ax
                mov     ax, wptr prog_size          ; Size (in paragraphs)

                xor     bx, bx
                mov     wptr XMS_from_addr, bx      ; Initialize XMS source
                mov     wptr XMS_from_addr + 2, bx  ;  address (offset into extended memory block)

                call    rest_xms_seg                ; Attempt to restore it

IFNDEF NOFRAG
                jc      xms_dealloc                 ; Carry set = error, exit

; Next, restore the extra DOS segments
xms_extra_rest: mov     cx, wptr extra_count    ; Number of extra blocks to save
                jcxz    xms_dealloc             ; If CX = 0, we exit routine

                mov     di, offset dos_blocks   ; DI -> array of segment/size pairs

xms_extra_rest_loop:
                mov     ax, wptr [di].block_seg
                mov     es, ax                  ; ES = segment to restore
                mov     ax, wptr [di].block_size; AX = size in paragraphs
                push    cx
                push    di
                call    rest_xms_seg            ; Attempt to restore this block
                pop     di
                pop     cx
                jc      xms_dealloc             ; Carry flag set == error, exit
                add     di, size dos_block
                loop    xms_extra_rest_loop     ; Keep going through all blocks

ENDIF

xms_dealloc:    rcl     bl, 1                   ; Save carry flag in low bit of bl

                mov     dx, wptr handle         ; First, free XMS handle
                mov     ah, 0Ah
                push    bx
                call    dptr XMS_proc
                pop     bx

                rcr     bl, 1                   ; Restore carry flag from bl low bit

restore_xms_ret:pop     es
                ret
restore_xms     endp


; *****************************************************************************
; rest_xms_seg  Attempts to restore a chunk of RAM from XMS memory
;
; Entry:        ES points to the segment to restore
;               AX contains its length (in paragraphs)
;               handle holds the XMS handle to read from
;               XMS_from_addr contains offset into extended memory for read
;
; Return:       Carry set on error, carry clear on success
;               Updates XMS_from_addr for next read
; *****************************************************************************
rest_xms_seg    proc    near
                push    ds
                push    es

; Call the XMS copy memory function to do this; fill in request block
xms_read_size:  mov     bx, 10h                     ; AX = # of paragraphs, convert to bytes
                mul     bx                          ; DX:AX = AX * 10h, # of bytes to read
                mov     wptr XMS_size, ax           ; Store # of bytes to read
                mov     wptr XMS_size + 2, dx

xms_read_from:  mov     ax, wptr handle             ; Source XMS handle
                mov     wptr XMS_from, ax           ;  XMS_from_addr already filled in

xms_read_to:    xor     bx, bx
                mov     wptr XMS_to, bx             ; Read into conventional memory
                mov     wptr XMS_to_addr, bx        ; Offset of dest address
                mov     ax, es                      ; Segment of destination address
                mov     wptr XMS_to_addr + 2, ax

do_xms_read:    mov     si, offset @code:XMS_struc  ; DS:SI -> XMS structure
                mov     ah, 0Bh
                call    dptr XMS_proc               ; Do the move
                cmp     ax, 1
                jnz     rest_xms_seg_er

rest_xms_seg_ok:mov     ax, wptr XMS_size           ; Retrieve length
                mov     dx, wptr XMS_size + 2       ;  (32 bits)
                add     wptr XMS_from_addr, ax      ; Add two 32-bit values
                adc     wptr XMS_from_addr + 2, dx  ; Update XMS read pointer
                clc                                 ; Signal success
                jmp     short rest_xms_seg_ret

rest_xms_seg_er:stc

rest_xms_seg_ret:
                pop     es
                pop     ds
                ret
rest_xms_seg    endp

ENDIF
; *****************************************************************************


; *****************************************************************************
; restore_ems   Attempts to restore program from EMS expanded memory
;
; Entry:        DS points to our variables
;               Program was saved to EMS expanded memory (block referred to by handle)
;
; Return:       Carry set on error, carry clear on success
; *****************************************************************************
IFDEF USE_EMS
restore_ems     proc    near
                push    es

                assume  ds:@code                    ; Tell MASM that DS points to our variables

; First, attempt to restore the portion of the program block that was saved
ems_prog_rest:  mov     ax, wptr start_seg          ; Released segment address
                mov     es, ax
                mov     ax, wptr prog_size          ; Size (in paragraphs)

                xor     bx, bx
                mov     wptr ems_offset, bx         ; Maintain absolute by offset
                mov     wptr ems_offset + 2, bx     ;  pointer relative to handle

                call    rest_ems_seg                ; Attempt to restore it

IFNDEF NOFRAG
                jc      ems_dealloc                 ; Carry set = error, exit

; Next, restore the extra DOS segments
ems_extra_rest: mov     cx, wptr extra_count    ; Number of extra blocks to save
                jcxz    ems_dealloc             ; If CX = 0, we exit routine

                mov     di, offset dos_blocks   ; DI -> array of segment/size pairs

ems_extra_rest_loop:
                mov     ax, wptr [di].block_seg
                mov     es, ax                  ; ES = segment to restore
                mov     ax, wptr [di].block_size; AX = size in paragraphs
                push    cx
                push    di
                call    rest_ems_seg            ; Attempt to restore this block
                pop     di
                pop     cx
                jc      ems_dealloc             ; Carry flag set == error, exit
                add     di, size dos_block
                loop    ems_extra_rest_loop     ; Keep going through all blocks

ENDIF

ems_dealloc:    rcl     bl, 1                   ; Save carry flag in low bit of bl

                mov     ah, 45h                 ; Deallocate EMS memory
                mov     dx, wptr handle         ; Specify which handle
                push    bx
                int     67h
                pop     bx

                rcr     bl, 1                   ; Restore carry flag from bl low bit

restore_ems_ret:pop     es
                ret
restore_ems     endp

; *****************************************************************************
; rest_ems_seg  Attempts to restore a chunk of RAM from EMS memory
;
; Entry:        ES points to the segment to restore
;               AX contains its length (in paragraphs)
;               handle holds the EMS handle to write to
;               ems_offset holds the 32-bit absolute offset in expanded
;                memory to read this block from
;
; Return:       Carry set on error, carry clear on success
;               Updates ems_offset with proper offset for next read
; *****************************************************************************
rest_ems_seg    proc    near
                push    ds
                push    es

                assume  ds:@code                ; Tell MASM DS points to our variables

; Call the EMS copy memory function to do this; fill in the EMS request block
ems_read_size:  mov     bx, 10h                     ; AX = # of paragraphs
                mul     bx                          ; DX:AX = AX * 10h, convert paragraphs to bytes
                mov     wptr EMS_size, ax           ; Store # of bytes to write
                mov     wptr EMS_size + 2, dx

ems_read_to:    xor     bx, bx
                mov     bptr EMS_to, bl             ; Copying to conventional memory (0)
                mov     wptr EMS_to_h, bx           ; Destination handle is 0 (conventional memory)
                mov     wptr EMS_to_o, bx           ; Destination offset is 0
                mov     ax, es                      ; Segment of destination address is ES
                mov     wptr EMS_to_s, ax

ems_read_from:  mov     bptr EMS_from, 1            ; Copying to expanded memory
                mov     ax, wptr handle
                mov     wptr EMS_from_h, ax         ; Specify EMS handle

                ; 32-bit absolute offset for copy is in ems_offset
                ;  convert to EMS page:offset (16K pages) values
                mov     ax, wptr ems_offset         ; Load 32-byte offset
                mov     dx, wptr ems_offset + 2
                mov     bx, ax                      ; Save a copy of ax (low 16 bits)
                and     ax, 0011111111111111b       ; Get (ax & (16K - 1)), this is the offset (14 bits)
                mov     wptr EMS_from_o, ax         ; Save page offset
                mov     cl, 14
                shr     bx, cl                      ; Move low 2 bits of page into low 2 bits of bx
                mov     cl, 2
                shl     dx, cl                      ; Move hi ? bits of page into dx shl 2
                or      dx, bx                      ; DX = page number (combine two values)
                mov     wptr EMS_from_s, dx         ; Save

                mov     ax, wptr EMS_size           ; Retrieve size of copy
                mov     dx, wptr EMS_size + 2
                add     wptr ems_offset, ax         ; Update EMS copy pointer
                adc     wptr ems_offset + 2, dx     ;  for next EMS write

do_ems_read:    mov     si, offset @code:EMS_struc  ; DS:SI -> EMS request structure
                mov     ax, 5700h                   ; Function 57 (copy/exchange memory), sub 0, copy memory
                int     67h                         ; Call EMS manager
                or      ah, ah                      ; AH = 0 means success
                jnz     rest_ems_seg_er             ; Not 0 means error

rest_ems_seg_ok:clc                                 ; Signal success
                jmp     short rest_ems_seg_ret

rest_ems_seg_er:stc

rest_ems_seg_ret:
                pop     es
                pop     ds
                ret
rest_ems_seg    endp

ENDIF
; *****************************************************************************


; *****************************************************************************
; restore_disk  Attempts to restore program from DOS disk file
;
; Entry:        DS points to our code segment
;               Program was saved to DOS disk file (full path stored in fname)
;
; Return:       Carry set on error, carry clear on success
; *****************************************************************************
IFDEF USE_DISK
restore_disk    proc    near

                push    ds

                assume  ds:@code                ; Tell MASM that DS points to our variables

open_file:      mov     dx, offset @code:fname  ; DS:DX -> file name
                mov     ax, 3D42h               ; DOS function 3Dh, open file
                                                ;  al = open for read only, deny none
                int     21h                     ; Call DOS
                jnc     open_ok                 ; Carry clear = all OK
                jmp     short restore_disk_ret  ; Carry set, just exit with error

open_ok:        mov     wptr handle, ax         ; File handle returned from DOS

; First, restore the program block contents saved to disk
disk_prog_rest: mov     ax, wptr start_seg      ; Get segment of program block saved
                mov     es, ax
                mov     ax, wptr prog_size      ; Get size of program block saved
                call    rest_disk_seg           ; Try to restore it
                jc      restore_disk_er         ; Carry set == error

IFNDEF NOFRAG
; Next, restore the contents of the extra blocks saved to disk
disk_extra_rest:
                mov     cx, wptr extra_count    ; Number of extra blocks to restore
                jcxz    close_read              ; IF CX = 0, we're done restoring

                mov     di, offset dos_blocks   ; DI -> array of segment/size pairs

disk_extra_rest_loop:
                mov     ax, wptr [di].block_seg
                mov     es, ax                  ; ES = segment to restore to
                mov     ax, wptr [di].block_size; AX = size in paragraphs
                push    cx
                push    di
                call    rest_disk_seg           ; Attempt to restore this block
                pop     di
                pop     cx
                jc      restore_disk_er         ; Error--exit routine
                add     di, size dos_block
                loop    disk_extra_rest_loop    ; Look for next DOS block

ENDIF

close_read:     mov     ah, 3Eh                 ; Close file
                int     21h                     ; Call DOS

restore_disk_ok:clc                             ; Signal success
                jmp     short restore_disk_ret  ;  and Exit

restore_disk_er:
                mov     ah, 3Eh                 ; Error, close file first
                int     21h                     ; Call DOS
                stc                             ; Signal failure

restore_disk_ret:
                pop     ds                      ; Restore our DS! (error in revs 2.11 and before)

                rcl     bl, 1                   ; Save carry flag in low bit of bl

                mov     dx, offset @code:fname  ; DS:DX -> file name
                mov     ah, 41h                 ; DOS function 41h, delete file
                push    bx
                int     21h                     ; Call DOS
                pop     bx

                rcr     bl, 1                   ; Restore carry flag from low bit of bl

                ret
restore_disk    endp

; *****************************************************************************
; rest_disk_seg Attempts to restore a chunk of RAM from the DOS disk file
;
; Entry:        ES points to the segment to restore
;               AX contains its length (in paragraphs)
;               handle contains the file handle to read from
;               Program was saved to DOS disk file (fname)
;
; Return:       Carry set on error, carry clear on success
; *****************************************************************************
rest_disk_seg   proc    near
                push    es
                push    ds

                mov     bx, es
                mov     ds, bx                  ; DS -> segment to restore to

                assume  ds:nothing

                mov     wptr cs:paras_left, ax  ; Keep count in this variable

disk_read_32k:  cmp     ax, 0800h                   ; Less than 32K left?
                jb      last_disk_read              ; Yes, do last read
                sub     wptr cs:paras_left, 0800h   ; 32K left to read
                mov     ah, 3Fh                 ; DOS function 3Fh, read file
                mov     bx, wptr cs:handle      ; BX = handle to read from
                mov     cx, 8000h               ; Read 32K bytes
                xor     dx, dx                  ; DS:DX -> buffer to read to
                int     21h                     ; Call DOS
                jc      rest_disk_seg_er        ; Carry set = error

disk_read_ok:   mov     ax, ds                  ; Address next read location
                add     ax, 0800h               ; It's 800h paragraphs ahead
                mov     ds, ax                  ; DS -> new restore location
                mov     ax, wptr cs:paras_left  ; Expecting this above
                jmp     short disk_read_32k     ; Read next 32K

last_disk_read: mov     cx, 4                   ; Convert paragraphs to bytes
                shl     ax, cl
                mov     cx, ax                  ; # of bytes left in cx
                mov     ah, 3Fh                 ; Read last bytes
                mov     bx, wptr cs:handle      ; BX = handle to read from
                xor     dx, dx                  ; DS:DX -> buffer to restore to
                int     21h                     ; Call DOS
                jc      rest_disk_seg_er        ; Error reading!  Close file first

rest_disk_seg_ok:
                clc
                jmp     short rest_disk_seg_ret

rest_disk_seg_er:
                stc

rest_disk_seg_ret:
                pop     ds
                pop     es
                ret
rest_disk_seg   endp

ENDIF
; *****************************************************************************


                
; *****************************************************************************
; execute_program   Execute the program specified
;
; Entry:            param_blk has been initialized
;                   DS points to our data
; Return:           puts return code in cs:exec_ret
; *****************************************************************************
execute_program proc    near                    ; Called only from inside our segment

                push    ds                      ; These are destroyed by the
                push    es                      ;  DOS EXEC call

                assume  ds:@code                ; Tell MASM that DS points to our variables

exec_program:   mov     ax, ds                  ; Our path name is in CS (point DS to our segment)
                mov     es, ax                  ; Our parameter block is in CS (point ES to our segment)
                mov     ax, 4B00h               ; Load and execute program
                mov     bx, offset @code:param_blk
                mov     dx, offset @code:prog_name
                int     21h                     ; Sets carry flag if error
                                                ; All registers destroyed
                                                ;  except CS:IP!

                assume  ds:nothing              ; Tell MASM that DS doesn't point to our variables

                mov     bptr cs:exec_ret, al    ; Store EXEC code
                jc      exec_err                ; Ooops

get_return:     mov     ah, 4Dh                 ; DOS function to get ret code
                int     21h                     ; All registers destroyed
                mov     bptr cs:exec_ret, al    ; Store EXEC code
                jmp     short exec_exit

exec_err:       mov     wptr cs:ret_code, 3     ; Signal error on executing

exec_exit:      pop     es
                pop     ds

                ret

execute_program endp


; *****************************************************************************
; err_exit          Prints error message and terminates program
;
; Entry:            Nothing.
; Returns:          Doesn't return--calls DOS terminate function.
;                   Naturally, we can't use the C runtime routines,
;                   since they are swapped out.
; *****************************************************************************
err_exit        proc    near                    ; Called only from inside our segment

                mov     ax, cs
                mov     ds, ax                  ; Point DS to our data

                assume  ds:@code                ; Tell MASM that DS points to our data

                mov     ah, 40h                 ; DOS function to write to file
                mov     bx, STDERR              ; Write to standard error handle
                mov     cx, wptr abort_len      ; CX = length of message
                mov     dx, offset @code:abort_msg  ; DS:DX = message
                int     21h

                mov     ax, 4CFFh           ; Exit, return code 255 decimal (FF hex)
                int     21h                 ; Exit to DOS, no return

err_exit        endp


; *****************************************************************************
; do_exec           Calls the execute routine, then restores program
;
; Entry:            Nothing
; Returns:          Since it is called from the non-resident area, it
;                   can only return if the program is restored completely.
; *****************************************************************************
do_exec         proc
                call    near ptr execute_program    ; Execute the specified program
                jnc     re_size                     ; No carry, OK

exec_er:        mov     wptr ret_code, 3        ; Signal error

re_size:        mov     es, wptr my_psp         ; Get our PSP address
                mov     bx, wptr old_size       ; Increase back to old size
                mov     ah, 4Ah                 ; DOS function 4Ah = resize
                int     21h
                jc      resize_err              ; Carry clear = all OK

IFNDEF NOFRAG
; If necessary, allocate all extra DOS memory blocks our program owned

                mov     cx, wptr extra_count    ; CX = number of extra DOS blocks
                jcxz    restore_prog            ; If zero, don't bother
                mov     di, offset dos_blocks   ; DI -> array of addresses/sizes

                push    es

alloc_extra_loop:
                mov     bx, wptr [di].block_size; BX = old size
                mov     ah, 48h                 ; DOS function to allocate memory block
                push    cx
                push    di
                int     21h
                pop     di
                pop     cx
                jc      resize_err              ; Unlikely error

check_alloc:    cmp     ax, wptr [di].block_seg ; Is it the same as the origignal segment address?
                jnz     resize_err              ; Nope.  We could do some fancy tricks here,
                                                ;  but for the most part it's not necessary.

                add     di, size dos_block      ; Point to next entry
                loop    alloc_extra_loop        ; Keep going through extra blocks

                pop     es
ENDIF
                jmp     short restore_prog

resize_err:     call    near ptr err_exit       ; Can't return, exit to DOS

restore_prog:   call    wptr restore_proc       ; Restore program from disk
                jc      resize_err              ; Carry set if error
                                                ; If no error, it returns
                                                ;  down to restored code
                ret
do_exec         endp

; *****************************************************************************
; *****************************************************************************
ALIGN 10h       ; Aligns next code item on paragraph boundary
                ; para_align is a proc instead of just a data
                ;  item because the ALIGN directive in MASM only
                ;  applies to code items, not data items!
para_align      proc    near
new_mcb         db      16 dup (0)          ; DOS will put MCB of released memory here
para_align      endp
; *****************************************************************************
; *****************************************************************************

; *****************************************************************************
; Everything after here is only needed BEFORE we change our allocation size.
;  Everything below this line will be (temporarily) swapped out of memory,
;  and thus cannot be used once we shrink our memory allocation.
; *****************************************************************************

; *****************************************************************************
;   swap        The routine that does it all
;
;   Callable by a C program, takes these parameters (regardless
;     of which swap options chosen at assembly time, because
;     C calling conventions let us ignore parameters to the
;     right if we want to):
;
;   swap_both:
;       prog        Full path name of program to execute
;       cmdline     Command-line parameters for program to execute
;       return      Pointer to byte for return code of exec'd program
;       save_file   Full path name of file in which to save program image (if disk is to be used)
;
;   Depending on the memory model used, the pointers to the
;   parameters each occupy 2 bytes or 4 bytes on the stack.
;   If there is only one data segment (Small and Medium), each
;   value is a 2-byte near pointer, with DS assumed as the segment
;   register.  If there are multiple data segments (Compact and
;   Large), each value is a 4-byte far pointer, with segment and
;   offset values each pushed on the stack.
;
;   The function is declared with 4 parameters, regardless of whether
;   disk swapping is being included.  This is because the file name
;   parameter is the last on the parameter list, which C lets us
;   ignore if we want.
;
;   The swap() routine does not check the program name or command
;   line to verify that a legal command has been requested--that's
;   the caller's responsibility!
;
; *****************************************************************************

                public  swap

swap            proc    prog:PTR, cmdline:PTR, return:PTR, save_file:PTR

                push    si                      ; Save registers needed
                push    di                      ;  by the caller
                push    es
                push    ds

point_segs:     mov     ax, cs                  ; Point ES to our segment
                mov     es, ax                  ;  for copying of parameters

; *****************************************************************************
get_name:       ; Copy program name to our variable, all versions

; If multiple data segments, load DS:SI from stack.  Else, just load SI
IF @datasize
                push    ds                      ; Save segment register
                lds     si, dptr prog           ; Load 32-bit far pointer
ELSE
                mov     si, wptr prog           ; Load 16-bit near pointer
ENDIF                                           ; DS:SI -> program name from caller

                mov     di, offset @code:prog_name  ; ES:DI -> our storage area

name_loop:      lodsb                           ; Fetch next byte
                stosb                           ; Save next byte
                or      al, al                  ; Was it 0 (end of string)?
                jnz     name_loop               ; No, get next one

IF @datasize
                pop     ds                      ; Pop DS if it was pushed above
ENDIF
; *****************************************************************************

; *****************************************************************************
get_cmd:        ; Copy command line to our variable, all versions

; If multiple data segments, load DS:SI from stack.  Else, just load SI
IF @datasize
                push    ds                      ; Save segment register
                lds     si, dptr cmdline        ; Load 32-bit far pointer
ELSE
                mov     si, wptr cmdline        ; Load 16-bit near pointer
ENDIF                                           ; DS:SI -> command line from caller
                
                mov     di, offset @code:cmd_line   ; ES:DI -> our storage area
                xor     cl, cl                  ; Keep track of length in cl

cmd_loop:       lodsb                           ; Fetch next byte from DS:SI
                or      al, al                  ; Was it 0 (end of string)?
                jz      cmd_end                 ; Yes, we're done
                stosb                           ; No, store byte
                inc     cl                      ; Increment length
                cmp     cl, MAX_DOS_CMD         ; Are we at maximum cmd length?
                jnz     cmd_loop                ; Nope, keep going

cmd_end:        mov     bptr es:[di], 0dh       ; Put CR at end of cmd line
                mov     bptr cs:cmd_len, cl     ; Store command-line length

IF @datasize
                pop     ds                      ; Pop DS if it was pushed above
ENDIF
; *****************************************************************************
; Set up the default FCBs at 5Ch and 6Ch in the PSP
;  Code provided by David E. Jenkins
                push    ds                      ; Save caller's DS

                mov     ax, cs                  ; Point DS to our
                mov     ds, ax                  ;  variables

                assume  ds:@code                ; Tell MASM that DS points to our variables
;
;   Locate the first two command line arguments
;
                push    ds                      ; Copy ds into es
                pop     es                      ;  "   "   "   "
                mov     di, offset @code:cmd_line   ; Point to command line in CS
                mov     al, bptr cmd_len            ; load the command line length
                xor     ah, ah
                inc     ax                      ; Include the CR in the length
                mov     wptr c_l_length, ax     ; Save the command line length
                add     ax, di                  ; Point to end of command line
                mov     wptr si_5c, ax          ; default to just after command line
                mov     wptr si_6c, ax          ;    "    "   "     "      "     "
                cmp     bptr cmd_len, 0         ; Is there anything to parse?
                jz      args_located            ; if not then args have been located

                mov     cx, wptr c_l_length     ; Load the command line length
                mov     al, ' '                 ; We must find the first non-blank
                repe    scasb                   ; Go until we find it or run out
                or      cx, cx                  ; Did we run out (CX = 0)?
                jz      args_located            ; Yes--then args have been located

                dec     di                      ; Move back to the right one
                inc     cx                      ;  "    "   "   "    "    "
                mov     wptr si_5c, di          ; Save the location of arg 1
                repne   scasb                   ; Find the next space (between arg1,2)
                or      cx, cx                  ; Did we run out
                jz      args_located            ; If so then args have been located

                dec     di                      ; Move back to the left one
                inc     cx                      ;  "    "   "   "    "   "
                repe    scasb                   ; Now find next non-blank (arg 2)
                or      cx, cx                  ; Did we run out
                jz      args_located            ; If so then args have been located

                dec     di                      ; Move back to the right one
                inc     cx                      ;  "    "   "   "    "    "
                mov     wptr si_6c,di           ; Save location of arg 2

args_located:
; parse the first argument into the first FCB

                mov     si, wptr si_5c                  ; Point to the first argument
                mov     di, offset @code:fcb5C_drive    ; Point to the unopened FCB
                mov     ah, 29h                 ; Parse file name function
                mov     al, 00h                 ; Do it like COMMAND.COM does it
                int     21h                     ; go for it

; parse the second argument into the second FCB
                mov     si, wptr si_6c                  ; Point to the second argument
                mov     di, offset @code:fcb6c_drive    ; point to the unopened FCB
                mov     ah, 29h                 ; Parse file name function
                mov     al, 00h                 ; Do it like COMMAND.COM does it
                int     21h                     ; go for it

                pop     ds                      ; Restore caller's DS

; *****************************************************************************
; Get the file name from the command line, if this version needs it
IFDEF USE_DISK
get_file:

; If multiple data segments, load DS:SI, else just load SI
IF @datasize
                push    ds                      ; Save segment register
                lds     si, dptr save_file      ; Load 32-bit pointer
ELSE
                mov     si, save_file           ; Load 16-bit pointer
ENDIF                                           ; DS:SI -> swap file name from caller

                mov     di, offset @code:fname  ; ES:DI -> our storage area

resolve:        mov     ah, 60h                 ; DOS INTERNAL function to resolve file name to full path name
                int     21h                     ; Stores complete path at ES:DI--we need it after EXEC in case
                                                ;  current drive or directory have changed
                                                ; Ignore file name error here--it
                                                ;  will be caught in save_disk if need be

IF @datasize
                pop     ds                      ; Pop DS if it was pushed above
ENDIF

ENDIF           ; IFDEF disk
; *****************************************************************************
; We have the parameters--let's go
; *****************************************************************************

                mov     wptr cs:ret_code, 0     ; Initialize swap's return code
                mov     cs:exec_ret, 0          ; Initialize exec's return code

save_stack:     mov     ax, ss
                mov     wptr cs:old_ss, ax      ; Save current SS
                mov     ax, sp
                mov     wptr cs:old_sp, ax      ; Save current SP

our_stack:      mov     ax, cs                  ; Our stack is in our CS
                cli                             ; Disable interrupts
                mov     ss, ax
                mov     sp, offset @code:new_sp ; Set new stack
                sti                             ; Re-enable interrupts

save_regs:      push    es                      ; Save needed registers
                push    ds                      ; This is the caller's DS!
                push    bp

                mov     ax, cs
                mov     ds, ax                  ; Point DS to our data

                assume  ds:@code                ; Tell MASM that DS points to our variables

save_info:      mov     ah, 51h                 ; DOS function 51h, get PSP
                int     21h                     ; Call DOS
                mov     ax, bx                  ; ax = PSP
                mov     wptr my_psp, ax         ; Save in cs: addressable location
                dec     ax                      ; PSP-1 = MCB for this mem block
                mov     es, ax
                mov     ax, es:[0001h]          ; Get PSP address--should be same!
                cmp     ax, wptr my_psp         ; All kosher?
                jz      psp_ok                  ; Yes

psp_error:      mov     wptr ret_code, 1        ; No, pass return code
                jmp     short exit_swap         ; Exit

psp_ok:         call    near ptr calc_size      ; Calc size to keep, save

try_save:       call    near ptr save_program   ; Write program to disk
                jnc     shrink_mem              ; Carry flag set on error

no_save:        mov     wptr ret_code, 2        ; Error--set return code
                jmp     short exit_swap         ; Exit routine on error

shrink_mem:     mov     ah, 4Ah                 ; DOS 4Ah--modify memory allocation
                mov     es, wptr my_psp         ; Point to PSP again
                mov     bx, wptr new_size       ; new_size was figured in calc_size
                int     21h                     ; Call DOS to shrink size
                jc      no_shrink               ; Carry set = error

IFNDEF NOFRAG
; If necessary, free all extra DOS memory blocks our program owns

                mov     cx, wptr extra_count    ; CX = number of extra DOS blocks
                jcxz    exec_prog               ; If zero, don't bother
                mov     di, offset dos_blocks   ; DI -> array of addresses/sizes

                push    es

free_extra_loop:
                mov     ax, wptr [di].block_seg
                mov     es, ax                  ; ES = DOS memory segment to free
                mov     ah, 49h                 ; DOS function to free memory block
                push    cx
                push    di
                int     21h
                pop     di
                pop     cx
                jc      no_shrink               ; Unlikely error
                add     di, size dos_block      ; Point to next entry
                loop    free_extra_loop         ; Keep going through extra blocks

                pop     es
ENDIF

                jmp     short exec_prog

; *****************************************************************************
; Any routine called or data referred to after this point MUST be located
;  in this source file BEFORE the variable new_mcb below!
; *****************************************************************************

no_shrink:      mov     wptr ret_code, 1        ; Carry = couldn't shrink block
                jmp     short exit_swap         ; Should delete file here!

exec_prog:      call    do_exec                 ; This code is resident, and can
                                                ;  be found above the resident line

; do_exec execute the routine AND restores the program!

exit_swap:      pop     bp                      ; Restore saved registers
                pop     ds                      ; This is the caller's DS!
                pop     es

                assume  ds:nothing              ; Tell MASM DS doesn't point to our variables

prev_stack:     mov     ax, wptr cs:old_ss      ; Restore original stack
                cli
                mov     ss, ax
                mov     sp, wptr cs:old_sp
                sti

; Giving user exec's return code.  It could be a 16- or 32-bit pointer
IF @datasize
                push    ds
                lds     si, dptr return         ; Load 32-bit pointer
ELSE
                mov     si, wptr return         ; Load 16-bit pointer
ENDIF                                           ; DS:SI -> return code variable
                
                mov     al, bptr cs:exec_ret    ; Store exec's return code
                mov     bptr [si], al           ;  at address specified by caller

IF @datasize
                pop     ds                      ; Pop DS if pushed above
ENDIF

                pop     ds
                pop     es
                pop     di
                pop     si
                mov     ax, wptr cs:ret_code    ; Give return code
                ret
swap            endp

; *****************************************************************************
; *****************************************************************************
; calc_size     Calculates the total size (in paragraphs) of all DOS blocks
;               owned by this program plus the amount of the initial program
;               allocation block we can swap out.
;
; Entry:        DS points to our variables
;               ES points to DOS Memory Control Block for our program
;
; Return:       old_size, start_seg, new_size, total_paras, extra_count initialized
; *****************************************************************************
calc_size       proc    near                    ; Called only from inside our segment

                push    es

                assume  ds:@code                ; Tell MASM that DS points to our variables

                mov     ax, es:[0003h]          ; Get # paragraphs allocated
                                                ;  in this memory block
                mov     wptr old_size, ax       ; Save old size of program
                mov     bx, cs                  ; BX = segment of our code
                mov     ax, offset @code:new_mcb; Last address to keep
                mov     cl, 4                   ; new_mcb is para aligned
                shr     ax, cl                  ; AX = ofs new_mcb / 16
                inc     ax
                add     bx, ax
                mov     wptr start_seg, bx      ; Segment of released memory
                sub     bx, wptr my_psp         ; BX = size to keep in paragraphs
                mov     wptr new_size, bx       ; Save new, smaller size
                mov     ax, wptr old_size
                sub     ax, bx
                mov     wptr prog_size, ax      ; ax = size of program block to swap out
                mov     wptr total_paras, ax    ; ax = total paragraphs

IFNDEF NOFRAG
; Now loop through all subsequent MCBs looking for blocks that we own (if
;  the MCB's "owner" (PSP) matches us (our PSP).  Right now ES points to
;  our MCB.  The MCB has three fields of interest:
;
;   Offset  Size    Description
;   -------------------------------------------------------------------------
;   0000h   Byte    Chain flag: 'M' (4Dh) if not last, 'Z' (5Ah) if last block in chain
;   0001h   Word    PSP segment of owner, 0000h if free memory
;   0003h   Word    Size of memory block in paragraphs, NOT including this MCB!

find_extras:    mov     wptr extra_count, 0     ; Initialize count
                mov     bx, wptr my_psp         ; Use bx to hold PSP for easy comparisons
                mov     di, offset dos_blocks   ; di = pointer to storage area

check_next_mcb: cmp     bptr es:[0000h], 'Z'    ; Is this the last block?
                jz      calc_size_ret           ; Yup

next_mcb2:      mov     ax, es                  ; ax = this MCB
                mov     cx, wptr es:[0003h]     ; cx = size of this mcb
                add     ax, cx
                inc     ax                      ; ax = addres of next MCB
                mov     es, ax                  ; ES -> next MCB

my_block:       cmp     wptr es:[0001h], bx     ; Does it match my PSP?
                jnz     check_next_mcb          ; Nope, move along

is_my_block:    inc     wptr extra_count        ; One more extra block
                cmp     wptr extra_count, MAX_EXTRA
                ja      calc_size_ret           ; Too many blocks--just exit

is_my_block2:   inc     ax                      ; Was MCB, now is address of segment
                mov     wptr [di].block_seg, ax ; Store segment address
                mov     cx, wptr es:[0003h]     ; Get size in paragraphs
                mov     wptr [di].block_size, cx; Store size
                add     wptr total_paras, cx    ; Increment total
                add     di, size dos_block      ; Next index (move pointer)
                jmp     short check_next_mcb
ENDIF

calc_size_ret:  pop     es
                ret

calc_size       endp
; *****************************************************************************

; *****************************************************************************
; xms_installed     Checks to see if XMS driver (himem.sys) is loaded
;
; Entry:            No assumptions--can be called by user
; Return:           1 if XMS driver is load, 0 if not
; *****************************************************************************
IFDEF USE_XMS
                public  xms_installed
xms_installed   proc                            ; Called by user also!

                push    ds                  ; Save all "important" registers
                push    si
                push    es
                push    di

                mov     ax, 4300h           ; Multiplex code for XMS driver, load check function
                int     2Fh                 ; Call multiplex interrupt
                cmp     al, 80h             ; al = 80h means XMS driver IS loaded
                jnz     no_xms              ; Nope, not there

yes_xms:        mov     ax, 4310h               ; Get address of entry point
                int     2Fh                     ; Returns address in ES:BX
                mov     wptr cs:XMS_proc, bx
                mov     wptr cs:XMS_proc + 2, es
                mov     ax, 1                   ; Return 1, XMS installed
                jmp     short xms_ret

no_xms:         xor     ax, ax              ; Return 0, XMS not installed

xms_ret:        pop     di
                pop     es
                pop     si
                pop     ds
                ret

xms_installed   endp
ENDIF
; *****************************************************************************

; *****************************************************************************
; ems4_installed    Checks to see if EMS 4.0 or above driver is loaded
;
; Entry:            No assumptions--can be called by user
; Return:           1 if EMS 4.0 driver is load, 0 if not
; *****************************************************************************
IFDEF USE_EMS
                public  ems4_installed
ems4_installed  proc                            ; Called by user also!

                push    ds                      ; Save "important" registers
                push    si
                push    es
                push    di


get_emm_vector: mov     ah, GET_VECTOR          ; Get EMM interrupt vector
                mov     al, 67h                 ; EMM accessed through Int 67h
                int     21h                     ; Call DOS to get vector
                mov     di, 0ah                 ; vector + di = name
                mov     ax, cs
                mov     ds, ax                  ; DS:SI -> EMM device driver name
                mov     si, offset @code:emm_name   ; Compare with EMM device name
                mov     cx, EMM_NAME_LEN
                cld
                repe    cmpsb                   ; Compare bytes
                jnz     ems_no                  ; Same?  If not, EMS installed

ems_yes:        mov     ah, 46h                 ; Get EMM version number
                int     67h                     ; Returns BCD in al
                cmp     al, 40h                 ; Look only at high 4 bits
                jb      ems_no                  ; Version not high enough--return 0

ems4_yes:       mov     ax, 1                   ; EMS installed, return 1
                jmp     short ems_ret

ems_no:         xor     ax, ax                  ; EMS not installed, return 0

ems_ret:        pop     di
                pop     es
                pop     si
                pop     ds
                ret

ems4_installed  endp
ENDIF
; *****************************************************************************


; *****************************************************************************
; save_program      Try to save in XMS/EMS/disk.
;
; Entry:            DS points to our variables
;
; Returns:          Success:  carry flag clear
;                   Failure:  carry flag set
; *****************************************************************************
save_program    proc    near            ; Called only from inside our segment

                push    si              ; Save registers
                push    di
                push    ds
                push    es

; Now figure out which routines to call, based on command-line definitions
; To change the order in which swap() attempts to swap, change the order
;  of these three conditional blocks.
IF1
   %out swap() will attempt to save the program in the following order:
ENDIF
   

; *****************************************************************************
IFDEF USE_XMS
IF1
   %out -- XMS extended memory
ENDIF
                call    save_xms        ; Try saving to XMS extended memory
                jnc     save_ok         ; Carry clear == success, all done
ENDIF
; *****************************************************************************


; *****************************************************************************
IFDEF USE_EMS
IF1
   %out -- EMS expanded memory
ENDIF
                call    save_ems        ; Try saving to EMS expanded memory
                jnc     save_ok       ; Carry clear == success, all done
ENDIF
; *****************************************************************************


; *****************************************************************************
IFDEF USE_DISK
IF1
   %out -- DOS disk file
ENDIF
                call    save_disk       ; Try saving to DOS disk file
                jnc     save_ok         ; Carry clear == success, all done
ENDIF
; *****************************************************************************

save_er:        stc                     ; Couldn't save anywhere, return error
                jmp     short save_ret

save_ok:        clc                     ; Saved successfully, return OK

save_ret:       pop     es              ; Restore registers
                pop     ds
                pop     di
                pop     si

                ret
save_program    endp
; *****************************************************************************


; *****************************************************************************
; Version-dependent code--only assemble the routine to save the program
; to each place if it was requested on the command line
; *****************************************************************************


; *****************************************************************************
; save_xms      Attempts to save program to XMS extended memory
;
; Entry:        DS points to our variables
;
; Return:       Carry set on error, carry clear on success
;               If successful, updates restore_proc with the address of
;               the XMS restore routine
; *****************************************************************************
IFDEF USE_XMS
save_xms        proc    near

                assume  ds:@code                ; Tell MASM DS points to our variables

                call    xms_installed           ; Check if XMS installed
                or      ax, ax                  ; Returns 0 if not installed
                jnz     xms_inst                ; AX != 0, XMS installed
                jmp     short save_xms_er       ; AX == 0, XMS not installed

xms_inst:       mov     dx, wptr total_paras    ; dx = total # of paragraphs to write
                mov     cl, 6                   ; Convert Paragraphs to kilobytes
                shr     dx, cl                  ; dx = dx / 64
                inc     dx                      ; dx = kilobytes needed (plus 1 for safety)

xms_alloc:      mov     ah, 09h                 ; XMS function 09, allocate extended memory block
                call    dptr XMS_proc           ; Call XMS entry point directly
                cmp     ax, 1                   ; AX = 1 on success
                jnz     save_xms_er             ; Allocation unsuccessful, error

xms_alloc_ok:   mov     wptr handle, dx         ; Save returned handle in DX

; First, attempt to save the portion of the program block
xms_prog_save:  mov     ax, wptr start_seg      ; Released segment address
                mov     es, ax
                mov     ax, wptr prog_size      ; Size (in paragraphs) of program block to save
                xor     bx, bx
                mov     wptr XMS_to_addr, bx    ; Initialize XMS destination
                mov     wptr XMS_to_addr + 2, bx;  address (offset into extended memory block)

                call    save_xms_seg            ; Attempt to save the program block
                jc      write_error             ; Carry set = failure, return

IFNDEF NOFRAG
; Next, save the extra DOS segments
xms_extra_save: mov     cx, wptr extra_count    ; Number of extra blocks to save
                jcxz    save_xms_ok             ; If CX = 0, we exit routine

                mov     di, offset dos_blocks   ; DI -> array of segment/size pairs

xms_extra_save_loop:
                mov     ax, wptr [di].block_seg
                mov     es, ax                  ; ES = segment to save
                mov     ax, wptr [di].block_size; AX = size in paragraphs
                push    cx
                push    di
                call    save_xms_seg            ; Attempt to save this block
                pop     di
                pop     cx
                jc      write_error             ; Carry flag set == error
                add     di, size dos_block
                loop    xms_extra_save_loop     ; Keep going through all blocks

ENDIF
                jmp     short save_xms_ok

write_error:    mov     dx, wptr handle             ; Free allocated handle
                mov     ah, 0Ah
                call    dptr XMS_proc               ; Falls through to failure code

save_xms_er:    stc
                jmp     short save_xms_ret

save_xms_ok:    mov     wptr restore_proc, offset @code:restore_xms     ; Initialize pointer
                clc                                                     ;  to restore routine

save_xms_ret:   ret
save_xms        endp


; *****************************************************************************
; save_xms_seg  Attempts to save a chunk of RAM to XMS memory
;
; Entry:        ES points to the segment to save
;               AX contains its length (in paragraphs)
;               handle holds the XMS handle to write to
;               XMS_to_addr contains offset into extended memory for write
;
; Return:       Carry set on error, carry clear on success
;               Updates XMS_to_addr for next write
; *****************************************************************************
save_xms_seg    proc    near
                push    ds
                push    es

; Call the XMS copy memory function to do this; fill in the XMS request block
xms_write_size: mov     bx, 10h                     ; AX = # of paragraphs
                mul     bx                          ; DX:AX = AX * 10h, convert paragraphs to bytes
                mov     wptr XMS_size, ax           ; Store # of bytes to write
                mov     wptr XMS_size + 2, dx

xms_write_from: xor     bx, bx
                mov     wptr XMS_from, bx           ; 0 means from conventional memory
                mov     wptr XMS_from_addr, bx      ; Offset of source address is 0
                mov     ax, es                      ; Segment of source address is ES
                mov     wptr XMS_from_addr + 2, ax

xms_write_to:   mov     ax, wptr handle             ; Destination XMS handle
                mov     wptr XMS_to, ax             ;  XMS_to_addr already filled in

do_xms_write:   mov     si, offset @code:XMS_struc  ; DS:SI -> XMS request structure
                mov     ah, 0Bh                     ; Function B, copy memory
                call    dptr XMS_proc               ; Do the memory copy move
                cmp     ax, 1                       ; AX = 1 means success
                jnz     save_xms_seg_er             ; Success, all done!

save_xms_seg_ok:mov     ax, wptr XMS_size           ; Retrieve length
                mov     dx, wptr XMS_size + 2       ;  (32 bits)
                add     wptr XMS_to_addr, ax        ; Add two 32-bit values
                adc     wptr XMS_to_addr + 2, dx    ; Update XMS write pointer
                clc                                 ; Signal success
                jmp     short save_xms_seg_ret

save_xms_seg_er:stc

save_xms_seg_ret:
                pop     es
                pop     ds
                ret
save_xms_seg    endp

ENDIF
; *****************************************************************************


; *****************************************************************************
; save_ems      Attempts to save program to EMS 4.0 expanded memory
;
; Entry:        DS points to our variables
;
; Return:       Carry set on error, carry clear on success
;               If successful, updates restore_proc with the address of
;               the EMS restore routine
; *****************************************************************************
IFDEF USE_EMS
save_ems        proc    near

                assume  ds:@code                ; Tell MASM DS points to our variables

                call    ems4_installed          ; Check if EMS 4.0 installed
                or      ax, ax                  ; AX = 0 if not installed
                jnz     ems_inst                ; AX != 0, ems installed
                jmp     short save_ems_er       ; AX = 0, no EMS, error!

ems_inst:       mov     bx, wptr total_paras    ; Total # of paragraphs we need
                mov     cl, 10                  ; Convert Paragraphs to 16K pages
                shr     bx, cl
                inc     bx                      ; BX = pages needed
                mov     bptr pages_used, bl     ; Save for later use

                mov     ah, 43h                 ; EMM function 43h, allocate
                int     67h
                or      ah, ah                  ; OK return code?
                jz      ems_alloc_ok            ; Yes, skip ahead
                jmp     short save_ems_er       ; No, not enough EMS

ems_alloc_ok:   mov     wptr handle, dx         ; Returned handle in DX

; First, attempt to save the portion of the program block
ems_prog_save:  mov     ax, wptr start_seg      ; Released segment address
                mov     es, ax
                mov     ax, wptr prog_size      ; Size (in paragraphs) of program block to save

                xor     bx, bx
                mov     wptr ems_offset, bx     ; Maintain absolute byte offset
                mov     wptr ems_offset + 2, bx ;  pointer into handle

                call    save_ems_seg            ; Attempt to save the program block

                jc      save_ems_fail           ; Carry set = failure, return

IFNDEF NOFRAG
; Next, save the extra DOS segments
ems_extra_save: mov     cx, wptr extra_count    ; Number of extra blocks to save
                jcxz    save_ems_ok             ; If CX = 0, we exit routine

                mov     di, offset dos_blocks   ; DI -> array of segment/size pairs

ems_extra_save_loop:
                mov     ax, wptr [di].block_seg
                mov     es, ax                  ; ES = segment to save
                mov     ax, wptr [di].block_size; AX = size in paragraphs
                push    cx
                push    di
                call    save_ems_seg            ; Attempt to save this block
                pop     di
                pop     cx
                jc      save_ems_fail           ; Carry flag set == error
                add     di, size dos_block
                loop    ems_extra_save_loop     ; Keep going through all blocks
ENDIF
                jmp     short save_ems_ok

save_ems_fail:  mov     dx, wptr handle         ; Failure--free handle
                mov     ah, 45h
                int     67h                     ; Falls through to failure code

save_ems_ok:    mov     wptr restore_proc, offset @code:restore_ems     ; Initialize pointer
                clc                                                     ;  to restore routine
                jmp     short save_ems_ret

save_ems_er:    stc

save_ems_ret:   ret
save_ems        endp

; *****************************************************************************
; save_ems_seg  Attempts to save a chunk of RAM to EMS memory
;
; Entry:        ES points to the segment to save
;               AX contains its length (in paragraphs)
;               handle holds the EMS handle to write to
;               ems_offset holds the 32-bit absolute offset in expanded
;                memory to write this block to
;
; Return:       Carry set on error, carry clear on success
;               Updates ems_offset with proper offset for next write
; *****************************************************************************
save_ems_seg    proc    near
                push    ds
                push    es

                assume  ds:@code                ; Tell MASM DS points to our variables

; Call the EMS copy memory function to do this; fill in the eMS request block
ems_write_size: mov     bx, 10h                     ; AX = # of paragraphs
                mul     bx                          ; DX:AX = AX * 10h, convert paragraphs to bytes
                mov     wptr EMS_size, ax           ; Store # of bytes to write
                mov     wptr EMS_size + 2, dx

ems_write_from: xor     bx, bx
                mov     bptr EMS_from, bl           ; Copying from conventional memory (0)
                mov     wptr EMS_from_h, bx         ; Source handle is 0 (conventional memory)
                mov     wptr EMS_from_o, bx         ; Source offset is 0
                mov     ax, es                      ; Segment of source address is ES
                mov     wptr EMS_from_s, ax

ems_write_to:   mov     bptr EMS_to, 1              ; Copying to expanded memory
                mov     ax, wptr handle
                mov     wptr EMS_to_h, ax           ; Specify EMS handle

                ; 32-bit absolute offset for copy is in ems_offset
                ;  convert to EMS page:offset (16K pages) values
                mov     ax, wptr ems_offset         ; Load 32-byte offset
                mov     dx, wptr ems_offset + 2
                mov     bx, ax                      ; Save a copy of ax (low 16 bits)
                and     ax, 0011111111111111b       ; Get (ax & (16K - 1)), this is the offset (14 bits)
                mov     wptr EMS_to_o, ax           ; Save page offset
                mov     cl, 14
                shr     bx, cl                      ; Move low 2 bits of page into low 2 bits of bx
                mov     cl, 2
                shl     dx, cl                      ; Move hi ? bits of page into dx shl 2
                or      dx, bx                      ; DX = page number (combine two values)
                mov     wptr EMS_to_s, dx           ; Save

                mov     ax, wptr EMS_size           ; Retrieve size of copy
                mov     dx, wptr EMS_size + 2
                add     wptr ems_offset, ax         ; Update EMS copy pointer
                adc     wptr ems_offset + 2, dx     ;  for next EMS write

do_ems_write:   mov     si, offset @code:EMS_struc  ; DS:SI -> EMS request structure
                mov     ax, 5700h                   ; Function 57 (copy/exchange memory), sub 0, copy memory
                int     67h                         ; Call EMS manager
                or      ah, ah                      ; AH = 0 means success
                jnz     save_ems_seg_er             ; Not 0 means error

save_ems_seg_ok:clc                                 ; Signal success
                jmp     short save_ems_seg_ret

save_ems_seg_er:stc

save_ems_seg_ret:
                pop     es
                pop     ds
                ret
save_ems_seg    endp
ENDIF
; *****************************************************************************


; *****************************************************************************
; save_disk     Attempts to save program to DOS disk file
;
; Entry:        DS points to our variables
;
; Return:       Carry set on error, carry clear on success
;               If successful, updates restore_proc with the address of
;               the disk restore routine
; *****************************************************************************
IFDEF USE_DISK
save_disk       proc    near
                push    es

                assume  ds:@code                ; Tell MASM DS points to our variables

creat_file:     mov     dx, offset @code:fname  ; DS:DX -> file name
                mov     ah, 3Ch                 ; Create/truncate file
                mov     cx, 02h                 ; Create a hidden file
                int     21h                     ; Call DOS
                jc      save_disk_er            ; Carry set, couldn't create file

creat_ok:       mov     wptr handle, ax         ; Save handle returned by DOS

; First, attempt to save the portion of the program block
disk_prog_save: mov     ax, wptr start_seg      ; Released segment address
                mov     es, ax
                mov     ax, wptr prog_size      ; Size (in paragraphs) of program block
                call    save_disk_seg           ; Attempt to save the program block
                jc      disk_write_er           ; Carry flag set == error

IFNDEF NOFRAG
; Next, save the extra DOS segments
disk_extra_save:
                mov     cx, wptr extra_count    ; Number of extra blocks to save
                jcxz    save_disk_ok            ; If CX = 0, we exit routine

                mov     di, offset dos_blocks   ; DI -> array of segment/size pairs

disk_extra_save_loop:
                mov     ax, wptr [di].block_seg
                mov     es, ax                  ; ES = segment to save
                mov     ax, wptr [di].block_size; AX = size in paragraphs
                push    cx
                push    di
                call    save_disk_seg           ; Attempt to save this block
                pop     di
                pop     cx
                jc      disk_write_er           ; Carry flag set == error
                add     di, size dos_block
                loop    disk_extra_save_loop    ; Keep going through all blocks

ENDIF
                jmp     short save_disk_ok


disk_write_er:  mov     ah, 3Eh                 ; Close file first
                mov     bx, wptr handle
                int     21h
                stc
                jmp     short save_disk_ret


save_disk_ok:   mov     ah, 3Eh                 ; 3eh = close file
                mov     bx, wptr handle
                int     21h
                mov     wptr restore_proc, offset @code:restore_disk    ; Initialize pointer
                clc                                                     ;  to restore routine
                jmp     short save_disk_ret

save_disk_er:   stc

save_disk_ret:  pop     es
                ret
save_disk       endp


; *****************************************************************************
; save_disk_seg Attempts to save a chunk of RAM to DOS disk file
;
; Entry:        ES points to the segment to save
;               AX contains its length (in paragraphs)
;               handle holds the file handle to write to
;
;
; Return:       Carry set on error, carry clear on success
; *****************************************************************************
save_disk_seg   proc    near
                push    ds
                push    es
                push    di

                assume  ds:@code

                mov     wptr paras_left, ax     ; Used to count paras written
                mov     bx, es
                mov     ds, bx                  ; DS -> segment to write

                assume  ds:nothing

disk_write_32k: cmp     ax, 0800h               ; paras_left less than 32K?
                jb      finish_disk_write       ; Yes, exit
                sub     wptr cs:paras_left, 800h; We will write 32K bytes now

                mov     ah, 40h                 ; DOS function to write to file
                mov     bx, wptr cs:handle      ; BX = file handle to write to
                mov     cx, 8000h               ; Write 32K bytes
                xor     dx, dx                  ; DS:DX is buffer to write
                int     21h                     ; Write data to file
                jc      save_disk_seg_er        ; This write failed--escape

                ; Check for disk full err.
                cmp     ax, 8000h               ; /*SJD Mon  09-09-1991  22:05:05 */
                jne     save_disk_seg_er        ; /*SJD Mon  09-09-1991  22:05:07 */

disk_write_ok:  mov     ax, ds                  ; Move write pointer in memory
                add     ax, 800h                ; We just wrote 1K paragraphs
                mov     ds, ax
                mov     ax, wptr cs:paras_left  ; AX checked above
                jmp     short disk_write_32k    ; Loop on next 32K

finish_disk_write:
                mov     cl, 4                   ; AX = # paragraphs left to write
                shl     ax, cl                  ; Paragraphs to bytes
                mov     cx, ax
                mov     ah, 40h                 ; 40h = write to file
                mov     bx, wptr cs:handle      ; BX = file handle to write to
                xor     dx, dx                  ; DS:DX = buffer
                int     21h                     ; Call DOS
                jc      save_disk_seg_er        ; Carry set, error (close file first)

save_disk_seg_ok:

                clc
                jmp     short save_disk_seg_ret

save_disk_seg_er:
                stc

save_disk_seg_ret:
                pop     di
                pop     es
                pop     ds

                ret
save_disk_seg   endp



ENDIF
; *****************************************************************************

END



