#! /bin/sh
#
# @file		make.sh		Quick test for execution and verion of GNU Make
# @author	Wes Garland
# @date		June 6th, 2003
#
# We require GNU Make that supports overriding variables on a 
# target-by-target basis -- otherwise thing get ugly, having
# to specify rules over and over again with minor variations..
#
# 3.76 does NOT support this feature. 3.79 does.
#
[ ! "${MAKE}" ] && echo "Warning: configure did not set \${MAKE}!" >&2
[ ! "${MAKE}" ] && MAKE=make

nonlecho()
{
  if [ -f "/usr/bin/printf" ] || [ -f "/bin/printf" ]; then
    if [ "$2" ]; then
      printf '%s' "$*"
    else
      printf "$1"
    fi
  else
    if [ -f "/usr/ucb/echo" ]; then
      /usr/ucb/echo -n "$*"
    else
      echo -n "$*"
    fi
  fi

  return 0 # true
}

beep()
{
  nonlecho "\007"
  return 0 # true
}

echo "Testing your version of GNU make: ${MAKE}"

set -- `${MAKE} --version | grep GNU | sed -e "s/GNU Make//" -e "s/.*version[ ]*//" -e "s/,.*//" -e "s/\./ /g"`

# failure: 
#	yes - no good
#	no - just fine
#	else - probably just fine

[ ! "$1" ] && failure=probably && beep && echo ">>>>> This does not appear to be GNU make." >&2
[ "$1" -lt "3" ] && failure=yes
[ "$1" -gt "3" ] && failure="probably"
[ "$1" -eq "3" ] && [ "$2" -le "76" ] && failure=yes
[ "$1" -eq "3" ] && [ "$2" -ge "77" ] && failure="probably not"
[ "$1" -eq "3" ] && [ "$2" -ge "79" ] && failure=no
[ "$1" -eq "3" ] && [ "$2" -gt "80" ] && failure="almost certainly"

case "${failure}" in
	yes)
		beep
		echo
		echo " + You are using a version of GNU make (version `echo $*|sed 's/ /./g'`) which is"
		echo "   known to be incompatible with the Maximus Makefiles. While it"
		echo "   is possible to modify these files to work with your version"
		echo "   of make, it would be far simpler for you to simply upgrade"
		echo "   to GNU Make 3.79 or newer."
		retval=1
		;;
	no)
		echo " - Looks fine to me! (version `echo $*|sed 's/ /./g'`)"
		retval=0
		;;
	*)
		beep
		echo
		echo " + You are using a unrecognized version (`echo $*|sed 's/ /./g'`) of GNU make."
		echo "   The development platforms run GNU make 3.79 and 3.80. Our"
		echo "   guess is that your version will ${failure} work."
		retval=0
		echo
		nonlecho "Press Enter to Continue: "
		;;
esac

exit ${retval}
