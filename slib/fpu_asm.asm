;*# name=FPU detection module
;   credit=Thanks go to Anders Brink for this module
;/

;       dosseg
        .model medium, pascal
        .code

        PUBLIC GETFPU

.8087

GETFPU PROC

        push    bp
        int     11h
        pop     bp
        and     ax,0002                         ; equipment check
        mov     ax,0
        jz      done                            ; no FPU installed
        jmp     next

ndpdd1  dw      7 dup (?)
ndp_env label   word
        dw      7 dup (0)
ndp_cw  label   word
        dw      0
ndpdd2  dw      7 dup (?)
next:
        cli
        fnstenv ndp_env
        sti
        fninit
        fnstcw  ndp_cw
        cmp     byte ptr cs:ndp_cw+1,03h
        mov     ax,1                            ; 8087
        jne     done

        and     byte ptr cs:ndp_cw,7FH
        fldcw   ndp_cw
        fdisi
        fstcw   ndp_cw
        fldenv  ndp_env
        test    byte ptr cs:ndp_cw,80H
        mov     ax,2                            ; 80287
        jnz     done

        mov     ax,3                            ; 80387
done:
        ret

GETFPU ENDP

end


