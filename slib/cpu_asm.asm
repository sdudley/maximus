;*# name=CPU detection module
;   credit=Thanks go to Anders Brink for this module
;/

;       dosseg
        .model medium, pascal
        .code

        public GETCPU

;
; int getcpu(void)
;
; return: AX
;

_p8088  equ 0
_p8086  equ 1
_pV20   equ 2
_pV30   equ 3
_p80188 equ 4
_p80186 equ 5
_p286   equ 6
_p386   equ 7
_p486   equ 8

.8086

GETCPU PROC

   ;  This test differentiates between a 286 and up and an 80186 and below
   ;
   ;  This is done by checking the value of bits 12-15 in the flag register
   ;  when it is pushed onto the stack.   If they are set, then the CPU
   ;  is not a 286 and up.
   ;
   ;

   pushf                   ;; /* save flags  */
   xor  ax,ax              ;; /*  set ax=0            */
   push ax                 ;; /*  push onto stack     */
   popf                    ;; /*  pop flags           */
   pushf                   ;; /*  push flags          */
   pop  ax                 ;; /*  pop into ax         */
   and  ax,0f000h          ;; /*  mask upper 4 bits   */
   cmp  ax,0f000h          ;; /*  are bits 12-15 set? */
   je   isnot286           ;; /*  YES: Not an 286 and up */

   ;
   ;   This test differentiates between a 286 and 386/486.
   ;
   ;   This is done by checking if bits 12-14 in the flag register
   ;   remain set when pushing and popping them off the stack.
   ;
   ;

   .186                    ; ; /* enable 80186 instructions */

   mov  dl,_p286           ; ; /* load code for 286 */
   push 07000h             ; ; /* push 07000h onto stack */
   popf                    ; ; /* pop flags  */
   pushf                   ; ; /* push flags */
   pop  ax                 ; ; /* put into ax */
   and  ax,07000h          ; ; /* mask bits 12-14 */
   je   exit              ; ; /* are bits 12-14 clear? */
                                 ; /* YES: it is a 80286 */
   ;
   ;     This test differentiates between a 386 and a 486
   ;
   ;   This is done by checking if bit 18 will stick in the
   ;   extended flags register.
   ;

   .386p                   ;  enable 386 instructions

   inc  dl                 ;  code for 386
   pushfd                  ;  save extended flags
   mov   eax, 040000h      ;
   push  eax               ;  push 40000h onto stack
   popfd                   ;  pop extended flags
   pushfd                  ;  push extended flags
   pop  eax                ;  put in eax
   and  eax, 040000h       ;  is bit 18 set?
   je   end486test         ;  YES: it is a 386
   inc  dl                 ;  NO: it is a 486

end486test:

   popfd                   ;; /* restore extended flags */
   jmp  exit

   ;  This test differentiates a 80186/80188 between a V20/V30/8088/8086.
   ;
   ;  The 80186/80188 will clear the upper three bits of the shift count
   ; before executing a shift instruction using the cl register.
   ;

isnot286:

   .8086                   ;; /* restrict to 8086 instruction set */

   mov  dl,_p80188         ;; /* load code for 80188    */
   mov  al,0ffh            ;; /* set all bits in al     */
   mov  cl,021h            ;; /* try shifting 21h times */
   shr  al,cl              ;; /* try shifting           */
   jne  t8_16              ;; /* if al!=0 then it is a 80186/80188 */

   ;  This test differentiates the NEC V20/V30 between the 8088/8086
   ;
   ;  This is done by testing for a bug in the 8088/8086.  In the
   ;  Intel CPUs, if a repeated string instruction with a segment
   ;  override is interrupted by a hardware interrupt, the
   ;  instruction is not continued.
   ;

   mov  dl,_pV20           ;; /* load code for NEC V20 */
   sti                     ;; /* enable interrupts */
   push si                 ;; /* save si ( could be a register variable) */
   push cs                 ;; CS -> ES
   pop  es
   mov  si,0               ;; /* Starting with first byte in es  */
   mov  cx,0ffffh          ;; /* read a complete segment */
   rep  lods byte ptr es:[si]  ;; /* rep with a segment override */
   ; /* hardware interrupt is sure to occur during the above instruction */
   pop  si                 ;; /* restore si  */
   or   cx,cx              ;; /* has entire segment been read? */
   je   t8_16              ;; /* YES: V20 or V30 */
   mov  dl,_p8088          ;; /* NO:  must be 8088 or 8086 */

   ;
   ;  The following test differentiates the data bus size such that
   ;  the difference between a *86 and *88, and V20 and V30 can be
   ;  determined.
   ;
   ;  This is done by measuring the instruction queue.  With the 8 bit bus
   ;  CPUs the instruction queue is 4 bytes long, but with the 16 bit bus
   ;  CPU, the instruction queue is 6 bytes long.
   ;

t8_16:

   push cs                 ;; /*  cs -> es */
   pop  es                 ;
   std                     ;; /* string instruction will decrement   */
   mov  di,offset queue_end;; /* es:di points at the queue end       */
   mov  al,90h             ;; /* op-code for nop                     */
   mov  cx,3               ;; /* execute string instruction 3 times  */
   cli                     ;; /* disable interrupts */
   rep  stosb              ;
   cld                     ; ; /* increment on string instructions */
   nop                     ; ; /* fill queue with dummy instructions */
   nop                     ;
   nop                     ;

   incdx:

   inc  dx                 ; ; /* increment processor code, */
   nop                     ; ; /* overwritten in a 4 byte queue */

   queue_end:

   nop
   sti                     ; ; /* re-enable interrupts */
   mov byte ptr cs:[incdx],42h ; ; /* restore inc dx opcode in queue */

exit:

   popf                    ;; /* restore flags                   */
   xor  dh,dh              ;; /* set high byte of proc code to 0 */
   mov  ax,dx              ;; /* return proc code in ax          */
   ret
GETCPU endp


end

