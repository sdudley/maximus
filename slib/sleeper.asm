.model small, c
.code

public sleeper

sleeper proc
        push    ax
        push    dx
        xor     dx,dx
        mov     ax,1
        hlt
        db      35h,0cah
        pop     dx
        pop     ax
        ret
sleeper endp

end
