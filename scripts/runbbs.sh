#! /bin/sh
#
# $Id: runbbs.sh,v 1.2 2003/06/29 20:56:02 wesgarland Exp $
#
# $Log: runbbs.sh,v $
# Revision 1.2  2003/06/29 20:56:02  wesgarland
# Added the "AFTER_MAX" capability -- this allows forking Maximuses (e.g.
# those running ipcomm.so or telnetcomm.so comm plug-ins) to call runbbs.sh
# with appropriate parameters to simulate an errorlevel for a maximus instance
# running in "traditional" mode (i.e. serialcomm.so).
#
# runbbs.sh has been updated to be roughly functionally equivalent to runbbs.bat
# which ships with Maximus 3.02 for DOS platforms. It has also been written
# so that someone who knows DOS batch (but not much Bourne shell) can set up
# their custom errorlevels without much of a learning curve.
#
# Revision 1.1  2003/06/12 02:09:56  wesgarland
# Initial Revision
#
#
# @file         runbbs.sh
# @author       Wes Garland
# @date         May 25 2003
# @description  A helpful script for running Maximus under 
#               UNIX. Ideal for init:respawn monitoring, and
#		processing error codes (à la runbbs.bat)
#
# @note		This script should run under any bourne-derived shell.
#

###
# Where is Maximus?
###
PREFIX=/var/max
MAXIMUS="${PREFIX}/etc/max.prm"
export PREFIX MAXIMUS

###
# Uncomment these variables to run squish after echomail
# or netmail is created by Maximus
###
SQUISH="${PREFIX}/etc/squish.cfg"
SQUISH_CMD="${PREFIX}/bin/squish -c${PREFIX}/etc/squish.cfg"
export SQUISH

###
# Uncomment these variables to run a binkp poll right after squish
###
BINKP=/var/binkp/sbin/binkp
BINKP_CFG=/var/binkp/etc/binkp.cfg
UPLINK=1:249/303

PATH="${PATH}:/${PREFIX}/bin"
LD_LIBRARY_PATH="${PREFIX}/lib:${LD_LIBRARY_PATH}:/usr/local/lib"
export PATH LD_LIBRARY_PATH

###
# Where/What is the log file?
###
logfile=${PREFIX}/log/runbbs.log
# logfile=syslog
# logtag=runbbs
# logfac=local0

###
# Set the minimum dynamic node (task) number. MUST be bigger than the maximum 
# task number number of any fixed Maximus process (e.g.  serial instances)
###
minDynNode=3
export minDynNode

###
# Set which TCPIP_PORT we're binding to
# for listen, when launching maximus for
# a TCP/IP device. Can be override on
# the command line.
###
TCPIP_PORT=2000 

log()
{
  level="$1"
  shift

  [ "${logfile}" = "syslog" ] && logger -t ${logtag} -p ${logfac}.${level} "$*" && return 0

  case "$1" in
	debug|info)
 	  date "+%d %b %T RUNBBS  $*" >> $logfile
	  ;;
	*)
	  echo "$*" >&2
	  date "+! %d %b %T RUNBBS  $*" >> $logfile
	;;
  esac

  return 0
}

###
# Wait for a call; ipcomm.c comm driver.
###
WaitForCall_ipcomm()
{
  log info "Entered WaitForCall_ipcomm()"

  basename="`basename $0`"
  dirname="`dirname $0`"

  [ "${dirname}"  = "." ] && dirname="`pwd`"
  [ "${dirname}"  = "" ]  && dirname="${PREFIX}/bin"
  [ "${basename}" = "" ]  && basename="runbbs.sh"

  AFTER_MAX="${dirname}/${basename}"
  export AFTER_MAX

  log debug "Set AFTER_MAX=${AFTER_MAX}"

  # When maximus accepts a "call" with the ipcomm driver, it exits
  # with status=0, while a child handles the user's session.

  OLD_PWD="`pwd`"
  cd "${PREFIX}"
  bin/max -w -b38400 -n0 -p${TCPIP_PORT}
  errlvl=$?
  cd "${OLD_PWD}"
  
  if [ "$errlvl" != "0" ]; then
    log warn "Maximus/ipcomm returned errorlevel $errlvl"
    sleep 10
  fi

  return $errlvl
}

###
# Wait for a call; serial.c comm driver.
###
WaitForCall_serial()
{
  log info "Entered WaitForCall_serial()"

  OLD_PWD="`pwd`"
  cd "${PREFIX}"
  bin/max -w -s38400 -b38400 -n${TASK_ID} -pserial:${SERIAL_PORT}
  errlvl=$?
  cd "${OLD_PWD}"

  return $errlvl
}

##
# Wait for a call; ipserial.c comm driver.
##
WaitForCall_ipserial()
{
  log info "Entered WaitForCall_ipcomm()"

  OLD_PWD="`pwd`"
  cd "${PREFIX}"
  bin/max -w -s38400 -b38400 -n${TASK_ID} -pipserial:${TCPIP_HOST}:${TCPIP_PORT}
  errlvl=$?
  cd "${OLD_PWD}"

  return $errlvl
}

goto()
{
  # syntactic sugar

  $*
}

##### Code Below is based on runbbs.bat, so that the docs will still make sense #####

EchoMail()
{
  # Invoke scanner and packer here. Next,
  # go to the "Aftercall" label to process
  # any after-caller actions.
  #
  # For the Squish mail processor, use the
  # following command:

  [ ${SQUISH_CMD} ] && 		${SQUISH_CMD} OUT SQUASH -f ${PREFIX}/log/echotoss.log 
  [ ${BINKP} ]  && 		${BINKP} -P ${UPLINK} ${BINKP_CFG}

  goto Aftercall
}

NetMail()
{
  # Invoke packer here, then go to
  # the "Aftercall" label.
  #
  # For the Squish mail processor, use the
  # following command:

  [ ${SQUISH_CMD} ] && 		${SQUISH_CMD} SQUASH
  [ ${BINKP} ]  && 		${BINKP} -P ${UPLINK} ${BINKP_CFG}

  goto Aftercall
}

Aftercall()
{
  # Invoke after-each-caller utilities here.

  ${PREFIX}/bin/scanbld all
  goto End
}

Error()
{
  log error "An error occurred!"
  goto Down
}

End()
{
  # This label should re-load your phone
  # answering program. If you are using
  # the Maximus WFC, you want to jump back
  # to the top of the loop:

  # return should get us back to Loop(),
  # unless we're running in AFTER_MAX
  # mode (e.g. ipcomm)

  return 0
}

Down()
{
  # The system arrives here if there was a
  # problem.

  log emerg "Error! Maximus had a fatal error and could not continue!"
  goto Done
}

Done()
{
  log info "Done (pid $$)"
  fix_screen
  exit ${ERRORLEVEL}
}

processErrorLevel()
{
  while read if_statement
  do
    set -- ${if_statement} ""
    [ ! "$3" ] && continue

    if [ "${ERRORLEVEL}" -ge "$3" ]; then
      shift 
      shift
      shift
      $*
      break
    fi
  done <<BATCH
if errorlevel 255 goto Error
if errorlevel  16 goto Error
if errorlevel  12 goto EchoMail
if errorlevel  11 goto NetMail
if errorlevel   5 goto Aftercall
if errorlevel   4 goto Error
if errorlevel   3 goto Error
if errorlevel   2 goto Loop
if errorlevel   1 goto Done
BATCH
}

Loop()
{
  [ ! "${driver}" ] && goto Done

  while [ ! -f "${PREFIX}/die.now" ] && [ ! -f "${PREFIX}/${driver}.die.now" ]
  do
    WaitForCall_${driver}
    ERRORLEVEL="$?"
    if [ "${driver}" != "ipcomm" ]; then
      processErrorLevel
      # ipcomm driver only returns errorlevel=0,
      # but it spawns a copy of the AFTER_MAX
      # program just before it exits, setting
      # ERRORLEVEL to the right value
    fi      
  done
}

fix_screen()
{
  OLDPATH="$PATH"
  [ -d /usr/ucb ] && PATH="/usr/ucb:$PATH"
  stty sane
  reset || tset
  PATH="$OLDPATH"  
}

##### main: script starts executing here #####
[ "X${TERM}" = "X" ] || TERM=vt100

if [ "$1" = "AFTER_MAX" ]; then
  # must be coming in from AFTER_MAX handler designed
  # for ipcomm.c, see max/atexit.c
  
  log debug "After Max pid $$, errorlevel $2"
  [ "$2" ] && ERRORLEVEL="$2"
  processErrorLevel
  exit ${ERRORLEVEL}
fi

[ -f "${PREFIX}/${driver}.die.now" ] && rm "${PREFIX}/${driver}.die.now"
[ -f "${PREFIX}/die.now" ] && echo "Waiting to die.." && sleep 60 && exit 0

case "$1" in
  ipcomm|"")
	  driver=ipcomm
	  [ "$2" ] && TCPIP_PORT="$2"
	  ;;
  serial)
	  driver=serial
	  [ "$2" ] || (log crit "No Task number specified" && exit 1)
	  SERIAL_PORT="$2"
	  TASK_ID="$3"
	  ;;
  ipserial)
	  driver=ipserial
	  HOST="$2"
	  [ "$3" ] && TCPIP_PORT="$3"
	  ;;
  *)
     	  echo
     	  echo "Invalid arguments: $*"
     	  echo
     	  echo "Usage Examples:"
     	  echo "  $0 ipcomm 2000"
     	  echo "  $0 serial /dev/cua/b 4"
     	  echo
     	  exit 1
	  ;;
esac

# Loop forever, until we're told to die via file semaphore or get SIGINT/SIGTERM
trap "fix_screen && exit 1" 2 15
Loop
goto Done
