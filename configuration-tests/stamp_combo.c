#include <stdio.h>
#include "compiler.h"
#include "stamp.h"

int main(int argc, char *argv[])
{
  union stamp_combo 	u_sc;
  struct _dos_st	dos_st;
  union _stampu		msg_st;
  STAMP_BITFIELD	ldate;
  int			exitCode = 0;

  printf("Testing size of stamp_combo..\n");

  if ((sizeof(u_sc) != sizeof(dos_st)) || (sizeof(u_sc) != sizeof(msg_st)) || (sizeof(u_sc) != sizeof(ldate)))
  {
    fprintf(stderr, " + Error: Stamp combo sizes differ where they shouldn't (%i %i %i %i)\n",
	    (int)sizeof(u_sc), (int)sizeof(dos_st), (int)sizeof(msg_st), (int)sizeof(ldate));
    exitCode = 1;
  }

  if (sizeof(u_sc) != 4)
  {
    fprintf(stderr, 
	" + \007Warning: Stamp combo size (%i) is incorrect; this platform\n"
	"   will not be binary compliant with other Maximus/Squish products\n", (int)sizeof(u_sc));
  }

  if (exitCode == 0)
    printf(" - Looks fine to me!\n");

  return exitCode;
}

