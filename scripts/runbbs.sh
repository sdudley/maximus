#! /bin/sh
#
# $Id: runbbs.sh,v 1.3 2003/07/05 00:13:51 wesgarland Exp $
#
# $Log: runbbs.sh,v $
# Revision 1.3  2003/07/05 00:13:51  wesgarland
# Fixed bugs processing echomail/netmail, etc
#
# Revision 1.1  2003/06/12 02:09:56  wesgarland
# Initial Revision
#
#
# @file         runbbs.sh
# @author       Wes Garland
# @date         May 25 2003
# @description  A helpful script for running Maximus under 
#               UNIX. Ideal for init:respawn monitoring.
#
 
PREFIX=/var/max
 
MAXIMUS="${PREFIX}/etc/max.prm"
PATH="${PATH}:/${PREFIX}/bin"
LD_LIBRARY_PATH="${PREFIX}/lib:/usr/local/lib:${LD_LIBRARY_PATH}"

minDynNode=3

export PREFIX PATH LD_LIBRARY_PATH minDynNode
 
rm "${PREFIX}/die.now"
cd ${PREFIX}

while /bin/true
do
  [ -f "${PREFIX}/die.now" ] && break
  bin/max -w -b38400 -n0 -p2323
  sleep 1
done
