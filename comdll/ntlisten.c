/*
 * Maximus Version 3.02
 * Copyright 1989, 2002 by Lanius Corporation.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#pragma off(unreferenced)
static char rcs_id[]="$Id: ntlisten.c,v 1.1.1.1 2002/10/01 17:49:35 sdudley Exp $";
#pragma on(unreferenced)

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

int main(int argc, char *argv[])
{
  HANDLE hp;
  char buf[2];
  DWORD got;
  FILE *fp;

  if ((fp=fopen("listen.log", "a"))==NULL)
  {
    printf("Can't open listen.log\n");
    return 1;
  }

  if ((hp=CreateNamedPipe(argv[1], PIPE_ACCESS_INBOUND,
                          PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                          1, 0, 0, 2000, NULL))==INVALID_HANDLE_VALUE)
  {
    printf("Error opening named pipe %s\n", argv[1]);
    exit(1);
  }

  for (;;)
  {
    printf("Waiting to connect to client.\n");

    if (!ConnectNamedPipe(hp, NULL))
    {
      printf("ConnectNamedPipe returned error %d\n", GetLastError());
      exit(1);
    }

    printf("Connected to client.\n");

    while (ReadFile(hp, buf, 1, &got, NULL))
    {
      fwrite(buf, 1, 1, fp);
      printf("%c", buf[0]);
      fflush(stdout);
    }


    printf("Disconnecting from pipe\n");

    DisconnectNamedPipe(hp);
  }

  fclose(fp);
  CloseHandle(hp);
}

