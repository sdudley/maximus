#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <termios.h>

#include "../comdll/telnet.h"

static int transmit_binary = 0;

int to_read (int fd, char* buf, int count, int timeout)
{
    fd_set rfds;
    struct timeval tv;
    int i;
    
    tv.tv_sec = 0;
    tv.tv_usec = 500;
   
    do
    {
    /* Note: on some unices, tv will be decremented to report the 
     * amount slept; on others it will not. That means that some 
     * unices will wait longer than others in the case that a 
     * signal has been received. 
     */
       i = select(fd + 1, &rfds, NULL, NULL, &tv);
    } while(i < 0 && (errno == EINTR));
  
    if (i > 0)
      return read(fd, buf, count);

    if (i == 0)
      return 0;
        
    return -1;
	 
}

int telnetInterpret(unsigned char* buf, int bytesRead)
{
  /* output buffer and counter */
  int oi; 
  /* arguments */
  unsigned char arg, arg2;
  
  unsigned char obuf[bytesRead];
  /* counter */
  int i, j, found, start=0;
  
  memset(obuf, '\0', bytesRead);
  oi = 0;

  for(i=start; i < bytesRead; i++)
  {

    if(buf[i] == cmd_IAC)
    {
	/* if the argument is avaible */
        if((i+1) < bytesRead)
        {
	    i++;
	    arg = buf[i];
	}
	/* if it's not IAC xxx */
	else
	{
	    /* we read one */
	    if(to_read(0, &arg, 1, 200) != 1)
	    {
	        sleep(1);
		/* if we didn't get anything we tries agian, 
		   and hope it goes. */
	        if(to_read(0, &arg, 1, 200) != 1)
	        {
		    return oi;
		}
	    }
	}

	/* find out which argument it's */    
	switch(arg)
	{
	    /* a real IAC */
	    case cmd_IAC:
	        obuf[oi] = cmd_IAC;
	        oi++;
		break;
	    /* a erase charecter */	
	    case cmd_EC:
	        obuf[oi] = 0x08;
	        oi++;
	        break;    
	    /* IAC WILL/WONT/DO/DONT <some option> */
	    case cmd_WILL:				
	    case cmd_WONT: 
    	    case cmd_DO:
    	    case cmd_DONT:
		/* if the second argument is aviable */
		if((i+1) < bytesRead )	    
		{
		    i++;
		    arg2 = buf[i];
		}
		/* else we read one */
		else
		{
		    if(to_read(0, &arg2, 1, 200) != 1)
		    {
		        sleep(1);
		        if(to_read(0, &arg2, 1, 200) != 1)
		        {
		    	    return oi;
			}
		    }
		}
		if(arg == cmd_WILL && arg2 == mopt_TRANSMIT_BINARY)
		    transmit_binary = 1;
		else if(arg == cmd_WONT && arg2 == mopt_TRANSMIT_BINARY)
		    transmit_binary = 0;

		break;
	    /* no argument just continue the loop */
	    case cmd_SE: 
	    case cmd_NOP:
	    case cmd_DM:
	    case cmd_BRK:
	    case cmd_IP:
	    case cmd_AO:
	    case cmd_AYT:
	    case cmd_GA:
	    case cmd_EL:
	        break;
		
	    case cmd_SB:
	        found = 0;
		    
		for(j=i; j < bytesRead; j++)
		{
		    if(buf[j] == cmd_SE)
		    {
		        found = 1;
		        break;
		    }
		}	        

		if(found)
		{
		    i = j;
		}
		break;
	    default:
		obuf[oi] = arg;
		oi++;
		break;
	}
	
    }    
    else
    {
        obuf[oi] = buf[i];
        oi++;
    }
  }
  
  memcpy(buf, obuf, oi);
  
  
  return oi;
}

int writeWIAC(int fd, unsigned char* buf, int count)
{
    int i=0;
    static char iac = 255;
    char* tp;
    
    for(i=0; i < count; i++)
    {
        write(fd, &(buf[i]), 1);
	
	if(buf[i] == cmd_IAC)
	    write(fd, &iac, 1);
    }

    return 0;    
}
						    
    



void negotiateTelnetOptions(int preferBinarySession)
{
  unsigned char command[3];
  int 		ch;

  ch = read(0, &ch, 1);	/* Get the ball rolling */

  sprintf(command, "%c%c%c", cmd_IAC, cmd_DONT, opt_ENVIRON);
  write(0, command, 3);
    ch = read(0, &ch, 1);

  sprintf(command, "%c%c%c", cmd_IAC, cmd_DO, opt_SGA);
  write(0, command, 3);
    ch = read(0, &ch, 1);

  sprintf(command, "%c%c%c", cmd_IAC, cmd_WILL, opt_ECHO);
  write(0, command, 3);
    ch = read(0, &ch, 1);

  sprintf(command, "%c%c%c", cmd_IAC, cmd_WILL, opt_SGA);
  write(0, command, 3);
    ch = read(0, &ch, 1);

  sprintf(command, "%c%c%c", cmd_IAC, cmd_DONT, opt_NAWS);
  write(0, command, 3);
    ch = read(0, &ch, 1);

  if (!preferBinarySession)
    return;

  sprintf(command, "%c%c%c", cmd_IAC, cmd_DO, opt_TRANSMIT_BINARY);
  write(0, command, 3);
    ch = read(0, &ch, 1);

  sprintf(command, "%c%c%c", cmd_IAC, cmd_WILL, opt_TRANSMIT_BINARY);
  write(0, command, 3);
    ch = read(0, &ch, 1);

  return;
}

int fexist(char* filename)
{
    struct stat s;

    if(stat(filename, &s))
        return 0;
    else
        return 1;
}

int main(void)
{
    int s, t, nt, len, retval;
    struct sockaddr_un remote;
    struct timeval tv;
    fd_set rfds, wfds;
    char tmptext[128];
    char lockpath[128];
    unsigned char str[512];
    struct dirent * dirp = NULL;
    DIR* dir = NULL;

    negotiateTelnetOptions(1);

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) 
    {
        perror("socket");
        exit(1);
    }

    dir = opendir(getcwd(tmptext, 128));

    while((dirp = readdir(dir)))
    {
        if(strstr(dirp->d_name, "maxipc") 
           && !strstr(dirp->d_name, ".lck"))
        {
    	    sprintf(lockpath, "%s.lck", dirp->d_name);
	    if(fexist(lockpath))
	        continue;        
	    
	    strcpy(remote.sun_path, dirp->d_name);
    	    remote.sun_family = AF_UNIX;
	    break;
	}
    }

    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(s, (struct sockaddr *)&remote, len) == -1) 
    {
	perror("Sorry no more free nodes!");
	exit(1);
    }    

    for(;;)
    {
	tv.tv_usec = 5;
	tv.tv_sec = 0;

	FD_ZERO(&wfds);
	FD_SET(s, &wfds);

	if(select(s+1, &wfds, 0, 0, &tv) > 0)
	{
    	    if ((t=read(s, str, 512)) > 0) 
	    {
		writeWIAC(1, str, t);
    	    } 
	    else 
	    {
                if (t < 0) perror("recv");
                else fprintf(stdout, "Server closed connection\n");
	        exit(1);
    	    }
	}
	    
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);

	tv.tv_usec = 5;
	tv.tv_sec = 0;
	    
	if((retval = select(0 + 1, &rfds, 0, 0, &tv)) > 0)
	{
	    if((t = read(0, str, 512))) 
	    {
		t = telnetInterpret(str, t);
    		write(s, str, t);
	    }
	    else
	    {
	        exit(1);		
	    }
	}
    }

    close(s);

    return 0;
}
