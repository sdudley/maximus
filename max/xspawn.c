#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <pty.h>
#include <utmp.h>
#include "process.h"
#include "io.h"
#include "prog.h"
#include "ntcomm.h"

#define unixfd(hc)      FileHandle_fromCommHandle(ComGetHandle(hc))

/* Record locking code */

extern HCOMM hcModem;

int xxspawnvp(int mode, const char *Cfile, char *const argv[])
{
    pid_t pid;
    pid_t tpid;
    int status;
    int fd = 0;
    int BytesRead;
    int local = FALSE;
    FILE* fp = NULL;
    char buffer[80];
    char tmp[1024];
    int i;

    struct termios term;
    struct winsize winsize;

    if(!hcModem)
	local=TRUE;

    if(!local)
	fd=unixfd(hcModem);
    else
	fd=1;

    memset(tmp, 0, 1024);

    for(i=0; argv[i]; i++)
    {
	strcat(tmp, argv[i]);
	strcat(tmp, " ");
    }

#if 0
    memset(&term,0,sizeof(term));
    cfsetspeed(&term,B19200);
    term.c_iflag = TTYDEF_IFLAG;
    term.c_oflag = TTYDEF_OFLAG;
    term.c_lflag = TTYDEF_LFLAG;
    term.c_cflag = TTYDEF_CFLAG;
    tcsetattr(fd, TCSAFLUSH, &term);

    winsize.ws_row=25;
                // #warning Currently cols are forced to 80 apparently 
    winsize.ws_col=80;

    ioctl(fd, TIOCSWINSZ, (char*) &winsize);
#endif
    
    if((pid=fork()) == 0) /* CHILD */
    {
        fcntl(fd, F_SETFL, O_NONBLOCK);
	if(!local)
	    dup2(fd, 0);	

	fp = popen(tmp, "r");
	
	while(fgets(buffer, 80, fp))
	{
	    if(!local)
	    {
		ComWrite(hcModem, buffer, strlen(buffer));
		ComWrite(hcModem, "\n", 1);
	    }
	    else
	    {
		VioWrtTTY(buffer, strlen(buffer), NULL);
		VioWrtTTY("\n", 1, NULL);
	    }
	}
	
	pclose(fp);
	
	_exit(1);
    }

    wait(&status);
    
    return 0;
}
