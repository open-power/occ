# $Id: ssxppc32files.mk,v 1.2 2014/06/26 12:58:31 cmolsen Exp $
# $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ppc32/ssxppc32files.mk,v $
#  @file ssxppc32files.mk
#
#  @brief mk for including ppc32 object files
#
#  @page ChangeLogs Change Logs
#  @section ssxppc32files.mk
#  @verbatim
#
#
# Change Log ******************************************************************
# Flag     Defect/Feature  User        Date         Description
# ------   --------------  ----------  ------------ -----------
# @pb00E                   pbavari     03/11/2012   Makefile ODE support
#
# @endverbatim
#

##########################################################################
# Object Files
##########################################################################
PPC32-C-SOURCES = ppc32_gcc.c
PPC32-S-SOURCES = div64.S savegpr.S

PPC32_OBJECTS = $(PPC32-C-SOURCES:.c=.o) $(PPC32-S-SOURCES:.S=.o)



