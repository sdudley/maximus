call setwc32
set mode=p
dmake -k

call setwc
set mode=r
dmake -k

set mode=p
dmake -k

call setwc32
set mode=r
dmake -k

