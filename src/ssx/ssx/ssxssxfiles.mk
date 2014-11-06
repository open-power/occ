# $Id: ssxssxfiles.mk,v 1.2 2014/06/26 13:02:00 cmolsen Exp $
# $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ssx/ssxssxfiles.mk,v $
#  @file ssxpgpfiles.mk
#
#  @brief mk for including pgp object files
#
#  @page ChangeLogs Change Logs
#  @section ssxpgpfiles.mk
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
# Include
##########################################################################


##########################################################################
# Object Files 
##########################################################################
SSX-C-SOURCES = ssx_core.c ssx_init.c ssx_stack_init.c

SSX-TIMER-C-SOURCES += ssx_timer_core.c ssx_timer_init.c

SSX-THREAD-C-SOURCES += ssx_thread_init.c ssx_thread_core.c \
	 ssx_semaphore_init.c ssx_semaphore_core.c

SSX_OBJECTS += $(SSX-C-SOURCES:.c=.o)

