;*# name=Get and set file date/time
;/

.MODEL SMALL

stdout equ     1

        .radix  16

t_hour  equ     [bp+4d]         ; get_fdate(int *hour,int *min,int *sec,
t_min   equ     [bp+6d]         ;           int *year,int *month,int *day,
t_sec   equ     [bp+8d]         ;           char *fname);
t_year  equ     [bp+10d]
t_month equ     [bp+12d]
t_day   equ     [bp+14d]
t_fname equ     [bp+16d]

.CODE


        PUBLIC  _get_fdate,_set_fdate

_get_fdate      PROC

        push    bp                      ; Save this for later
        mov     bp,sp                   ;  "    "    "    "
        push    ds
        push    di
        push    si

        mov     ah,3dh                  ; Open a handle for specified name
        mov     al,0                    ; Open file in "Read" mode

        mov     dx,t_fname              ; Use passed filename

        int     21h                     ; Make the call

        jc      g_error                 ; Return if it didn't work
        mov     handle,ax               ; Save handle for later


        mov     ah,57h                  ; Get/Set sub-function
        mov     al,00d                  ; Get date
        mov     bx,handle               ; Do it for the file we opened
        int     21h

        mov     ax,cx                   ; 1111
        and     ax,0fh                  ; Strip off the other bits
        shl     ax,1                    ; Multiply by two
        mov     bx,t_sec                ; Put it in t_sec
        mov     ds:[bx],ax              ; Move al into the right place


        mov     ax,cx
        push    cx                      ; Save cx
        mov     cl,5d
        shr     ax,cl
        and     ax,3fh                  ; 111111
        mov     bx,t_min                ; Put it in t_min
        mov     ds:[bx],ax              ; Move al into the right place
        pop     cx                      ; Restore CX

        mov     ax,cx
        push    cx                      ; Save cx
        mov     cl,11d
        shr     ax,cl
        and     ax,1fh                  ; 11111
        mov     bx,t_hour               ; Put it in t_hour
        mov     ds:[bx],ax              ; Move al into the right place
        pop     cx                      ; Restore CX

        mov     ax,dx
        and     ax,1fh                  ; 11111
        mov     bx,t_day                ; Put it in t_day
        mov     ds:[bx],ax              ; Move al into the right place

        mov     ax,dx
        mov     cl,5d
        shr     ax,cl
        and     ax,0fh                  ; 1111
        mov     bx,t_month              ; Put it in t_month
        mov     ds:[bx],ax              ; Move al into the right place

        mov     ax,dx
        mov     cl,9d
        shr     ax,cl
        and     ax,7fh                  ; 1111111
        add     ax,1980d
        mov     bx,t_year               ; Put it in t_year
        mov     ds:[bx],ax              ; Move al into the right place

        mov     ah,3eh                  ; Close the file
        mov     bx,handle               ;
        int     21h                     ;

        jc      g_error                 ; If something screwed up, then jump

        mov     ax,0                    ; Errorlevel 0
        jmp     g_backtoC

g_error:
        mov     ah,0                    ; Error is in AL

g_backtoC:

        pop     si
        pop     di
        pop     ds
        pop     bp                      ; return stack pointer
        ret

_get_fdate      ENDP








; set_fdate(int hour,int min,int sec,
;           int year,int month,int day,
;           char *fname);



_set_fdate      PROC

        push    bp                      ; Save this for later
        mov     bp,sp                   ;  "    "    "    "

        push    ds
        push    di
        push    si

        mov     ah,3dh                  ; Open a handle for specified name
        mov     al,0                    ; Open file in "Read" mode

        mov     dx,t_fname              ; Use passed filename

        int     21h                     ; Make the call

        jc      s_error                 ; Return if it didn't work
        mov     handle,ax               ; Save handle for later


        xor     cx,cx                   ; Zero out both CX and DX
        xor     dx,dx                   ;


; Do CX Stuff

        mov     ax,t_sec                ; Move seconds into AX
        shr     ax,1                    ; Divide by two
        or      cx,ax                   ; Put the bits in CX7


        mov     ax,t_min                ; Move minutes into AX
        push    cx
        mov     cl,5d                   ; Shift it left five
        shl     ax,cl                   ;
        pop     cx
        or      cx,ax                   ; Put the bits in CX


        mov     ax,t_hour               ; Move hours into AX
        push    cx
        mov     cl,11d
        shl     ax,cl                   ;
        pop     cx
        or      cx,ax                   ; Put the bits in CX


; Do DX stuff


        mov     ax,t_day                ; Move day into AX
        or      dx,ax                   ; Put the bits in CX7

        mov     ax,t_month              ; Move month into AX
        push    cx
        mov     cl,5d
        shl     ax,cl
        pop     cx
        or      dx,ax                   ; Put the bits in DX


        mov     ax,t_year               ; Move year into AX
        sub     ax,1980d
        push    cx
        mov     cl,9d
        shl     ax,cl
        pop     cx
        or      dx,ax                   ; Put the bits in DX




        mov     ah,57h                  ; Get/Set sub-function
        mov     al,01d                  ; Set date
        mov     bx,handle               ; Do it for the file we opened
        int     21h


        mov     ah,3eh                  ; Close the file
        mov     bx,handle               ;
        int     21h                     ;

        jc      s_error                 ; If error, then jump

        mov     ax,0                    ; Return '0'
        jmp     s_backtoC

s_error:
        mov     ah,0                    ; Errorlevel stays in AL

s_backtoC:
        pop     si
        pop     di
        pop     ds
        pop     bp                      ; return stack pointer
        ret

_set_fdate      ENDP


.DATA

handle  dw      0

        END
