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

/*# name=Alternate int 24h critical error handler
*/

#include <dos.h>
#include "prog.h"


#if defined(__MSDOS__)

  #define INT_CRITERR 0x24


  #ifdef __FLAT__
    #include <conio.h>
    #pragma aux newint24h parm [];

    void __interrupt newint24h(dword rgs, dword rfs, dword res, dword rds,
                               dword redi, dword resi, dword rebp, dword resp,
                               dword rebx, dword redx, dword recx,
                               dword volatile reax)
    {
      NW(redi); NW(redx); NW(recx); NW(rebx); NW(rebp); NW(resi);
      NW(resp); NW(rds); NW(res); NW(rfs); NW(rgs);

      if (reax & 0x8000)
      {
        cputs("Critical error ");

        cputs((reax & 0x0100) ? "writing " : "reading ");
        cputs("drive ");
        putch('A'+(char)(reax & 0xff));
        cputs(":\r\n");
      }
      else
      {
        cputs("Critical error accessing device\r\n");
      }

      reax=3; /* FAIL error code */
      return;
    }

  #else
    void _intr newint24h(void);
  #endif

  static byte installed=FALSE;
  static void (_intr * old_24h)(void);


  void _fast install_24(void)
  {
    if (installed)
      return;

    installed=TRUE;

    old_24h=(_intcast)getvect(INT_CRITERR);
    setvect(INT_CRITERR, (_veccast)newint24h);
  }

  void _stdc uninstall_24(void)
  {
    if (!installed)
      return;

    installed=FALSE;

    setvect(INT_CRITERR, (_veccast)old_24h);
  }

#elif defined(OS_2)
  void _fast install_24(void)
  {
  }
  void _stdc uninstall_24(void)
  {
  }
#elif defined(NT) || defined(UNIX)
  void _fast install_24(void)
  {
  }
  void _stdc uninstall_24(void)
  {
  }
#else
  #error unknown operating system
#endif /* OS_2 */


#ifdef TEST_HARNESS

#include <stdlib.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>

main()
{
  int fd;

  install_24();
  printf("hoop-la!\n");

  if ((fd=open("a:asdf", O_RDONLY | O_BINARY)) != -1)
  {
    printf("\nopen successful.\n");
    close(fd);
  }
  else printf("\nopen failed.\n");

  if ((fd=open("com3", O_CREAT | O_TRUNC | O_WRONLY | O_BINARY)) != -1)
  {
    printf("\nopen successful.\n");

    if (write(fd, 0, 5) != 5)
      printf("Write unsuccessful!\n");

    close(fd);
  }
  else printf("\nopen failed.\n");

  if ((fd=open("b:gronk", O_CREAT | O_TRUNC | O_WRONLY | O_BINARY)) != -1)
  {
    printf("\nopen successful.\n");

    if (write(fd, 0, 5) != 5)
      printf("Write unsuccessful!\n");

    close(fd);
  }
  else printf("\nopen failed.\n");

  printf("done!\n");
  uninstall_24();
  return 0;
}

#endif

