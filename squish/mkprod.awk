# $Id: mkprod.awk,v 1.1 2003/09/10 22:15:48 paltas Exp $
#

BEGIN	{
		print "#include <stdio.h>"
		print "#include <stdlib.h>"
		print "#include \"prog.h\""
		print "#include \"max.h\""
		print "#include \"squish.h\""		
		print "#include \"s_toss.h\""				
		print ""
		print "struct ftscprod_ products[] = {"
	}
/^[^;]/	{
		if ($2 != "DROPPED")
			print "	{0x" $1 ",(char *)\"" $2 "\"},"
	}
END	{
		print "	{0xff,(char*)0L}"
		print "};"
	}
