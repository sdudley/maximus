; ***************************************************************************
; *                                                                         *
; *  MAXIMUS-CBCS Source Code                                               *
; *  Copyright 1989, 1990 by Scott J. Dudley.  All rights reserved.         *
; *                                                                         *
; *  Generic FOSSIL call                                                    *
; *                                                                         *
; *  For complete details of the licensing restrictions, please refer to    *
; *  the license agreement, which is published in its entirety in MAXIMUS.C *
; *  and LICENSE.MAX.                                                       *
; *                                                                         *
; *  USE OF THIS FILE IS SUBJECT TO THE RESTRICTIONS CONTAINED IN THE       *
; *  MAXIMUS-CBCS LICENSING AGREEMENT.  IF YOU DO NOT FIND THE TEXT OF THIS *
; *  AGREEMENT IN ANY OF THE  AFOREMENTIONED FILES, OR IF YOU DO NOT HAVE   *
; *  THESE FILES, YOU SHOULD IMMEDIATELY CONTACT THE AUTHOR AT ONE OF THE   *
; *  ADDRESSES LISTED BELOW.  IN NO EVENT SHOULD YOU PROCEED TO USE THIS    *
; *  FILE WITHOUT HAVING ACCEPTED THE TERMS OF THE MAXIMUS-CBCS LICENSING   *
; *  AGREEMENT, OR SUCH OTHER AGREEMENT AS YOU ARE ABLE TO REACH WITH THE   *
; *  AUTHOR.                                                                *
; *                                                                         *
; *  You can contact the author at one of the address listed below:         *
; *                                                                         *
; *  Scott Dudley           FidoNet 1:249/106                               *
; *  777 Downing St.        IMEXnet 89:483/202                              *
; *  Kingston, Ont.         Internet f106.n249.z1.fidonet.org               *
; *  Canada - K7M 5N3                                                       *
; *                                                                         *
; *  Please feel free to contact the author at any time to share your       *
; *  comments about this software and/or licensing policies.                *
; *                                                                         *
; ***************************************************************************


;       dosseg

IFDEF __MSDOS__
        .model medium, pascal
        .data

COMM    _port:WORD      ; remove underscore prefix if .mode <size>, c

        .code

        public CALLFOSSIL
;
; unsigned short CallFossil( int FOSSIL_function, int AL );
;
; return: AX
;
CALLFOSSIL proc func:BYTE, ALreg:BYTE
        mov     dx,_port
        mov     ah,func
        mov     al,ALreg
        int     14h
        ret
CALLFOSSIL endp

ENDIF

end

