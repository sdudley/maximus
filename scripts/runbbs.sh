#! /bin/sh
#
# $Id: runbbs.sh,v 1.1 2003/06/12 02:09:56 wesgarland Exp $
#
# $Log: runbbs.sh,v $
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
minDynNode=3

export PREFIX PATH minDynNode
 
rm "${PREFIX}/die.now"
cd ${PREFIX}

while /bin/true
do
  [ -f "${PREFIX}/die.now" ] && break
  bin/max -w -b38400 -n0 -p2000
  sleep 1
done
