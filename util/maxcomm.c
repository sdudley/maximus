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
    struct termios termio;
    int s, t, len, tmp;
    int i = 1;
    char c;
    struct sockaddr_un remote;
    struct timeval tv;
    fd_set rfds, wfds;
    char tmptext[128];
    char lockpath[128];
    char str[100];
    struct dirent * dirp = NULL;
    DIR* dir = NULL;

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) 
    {
        perror("socket");
        exit(1);
    }

    dir = opendir(getcwd(tmptext, 128));

    while(dirp = readdir(dir))
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

    while(1) 
    {
	tv.tv_usec = 1;
	tv.tv_sec = 0;

	FD_ZERO(&wfds);
	FD_SET(s, &wfds);

	if(select(s+1, &wfds, 0, 0, &tv) > 0)
	{
    	    if ((t=read(s, str, 100)) > 0) 
	    {
                str[t] = '\0';
		write(1, str, t);
    	    } 
	    else 
	    {
                if (t < 0) perror("recv");
                else printf("Server closed connection\n");
	        exit(1);
    	    }
	}
	    
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);

	tv.tv_usec = 1;
	tv.tv_sec = 0;
	    
	if(select(0 + 1, &rfds, 0, 0, &tv) > 0)
	{
	    read(0, &c, 1);
	    if(c != 10)
	    {
    	        write(s, &c, 1);
	    }
	    else
	    {
	        write(s, "\r\n", 2);
	    }
	}
    }

    close(s);

    return 0;
}
