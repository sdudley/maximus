#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

int spawnlp(char* cmd, ...)
{
    va_list var_args;
    char * tmp;

    char * syscmd = NULL;

    va_start(var_args, cmd);
    syscmd = (char*) malloc(1024);
    
    strcpy(syscmd, cmd);
    
    while((tmp = va_arg(var_args, char*)) != NULL)
    {
	strcat(syscmd, " ");	
	strcat(syscmd, tmp);
    }
    return system(syscmd);
}

