##############################################################################
#
# Global project makefile
#
# $Id: makefile.mk,v 1.1.1.1 2002/10/01 17:49:16 sdudley Exp $
#
# This makefile defines the subprojects that make up the main Maximus
# distribution.  The definitions below are used to identify which of the
# subprojects are to be compiled for each memory model and/or operating
# system.
#
# The subprojects are interdependent, so they must be built in the
# order shown.  The conditionals define which subprojects are to be
# built, while the main TARGS variable defines the order (which
# should not be changed) in which the subprojects are built.
#
##############################################################################

##############################################################################
# All compilers (all models)
##############################################################################

DO_SLIB := slib
DO_MSGAPI := msgapi
#DO_SQUISH := squish
DO_BTREE := btree
DO_UI := ui

.IF $(MODE)==r
.IF $(FLAT)==YES
##############################################################################
# DOS, 32-bit
##############################################################################

.ELSE

##############################################################################
# DOS, 16-bit
##############################################################################
DO_SWAP := swap swap2
DO_UTIL := util
DO_MEX := mex
DO_MAX := max
DO_INSTALL := install

.END
.ELIF $(MODE)==p

##############################################################################
# OS/2 (all models)
##############################################################################
DO_MCP := mcp

.IF $(FLAT)==YES

##############################################################################
# OS/2, 32-bit
##############################################################################
DO_UTIL := util
DO_REXX := rexx
DO_SM := sm
DO_WIMP := wimp
DO_MAX := max
DO_MEX := mex
DO_INSTALL := install

.ELSE

##############################################################################
# OS/2, 16-bit
##############################################################################
DO_MAXBLAST := maxblast
DO_INSTALL := install

.END
.ELSE

##############################################################################
# NT, 32-bit
##############################################################################

DO_UTIL := util
DO_MEX := mex
DO_MAX := max
DO_COMDLL := comdll

.END


TARGS := $(DO_SLIB) $(DO_COMDLL) $(DO_MSGAPI) $(DO_SWAP) $(DO_SQUISH)   \
         $(DO_BTREE) $(DO_UI) $(DO_UTIL) $(DO_MEX) $(DO_MCP) $(DO_REXX) \
         $(DO_SM) $(DO_WIMP) $(DO_MAXBLAST) $(DO_MAX) $(DO_INSTALL)

all .PHONY: $(TARGS)

slib .PHONY:
        +cd slib && $(MAKE)

comdll .PHONY:
        +cd comdll && $(MAKE)

msgapi .PHONY:
        +cd msgapi && $(MAKE)

swap .PHONY:
        +cd swap && $(MAKE)

swap2 .PHONY:
        +cd swap2 && $(MAKE)

squish .PHONY:
        +cd squish && $(MAKE)

btree .PHONY:
        +cd btree && $(MAKE)

ui .PHONY:
        +cd ui && $(MAKE)

util .PHONY:
        +cd util && $(MAKE)

mex .PHONY:
        +cd mex && $(MAKE)

mcp .PHONY:
        +cd mcp && $(MAKE)

rexx .PHONY:
        +cd rexx && $(MAKE)

sm .PHONY:
        +cd sm && $(MAKE)

wimp .PHONY:
        +cd wimp && $(MAKE)

maxblast .PHONY:
        +cd maxblast && $(MAKE)

max .PHONY:
        +cd max && $(MAKE)

install .PHONY:
        +cd install && $(MAKE)

