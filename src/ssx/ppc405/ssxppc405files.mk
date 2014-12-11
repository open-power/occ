# $Id: ssxppc405files.mk,v 1.2 2014/06/26 13:00:55 cmolsen Exp $
# $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ppc405/ssxppc405files.mk,v $
#  @file ssxppc405files.mk
#
#  @brief mk for including ppc405 object files
#
#  @page ChangeLogs Change Logs
#  @section ssxppc405files.mk
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
# Include Files
##########################################################################

			

##########################################################################
# Object Files
##########################################################################
PPC405-C-SOURCES = ppc405_core.c \
				   ppc405_lib_core.c \
				   ppc405_cache_core.c \
				   ppc405_init.c \
				   ppc405_irq_core.c \
				   ppc405_irq_init.c 
				   
PPC405-S-SOURCES =  ppc405_boot.S \
					ppc405_exceptions.S \
					ppc405_cache_init.S \
					ppc405_mmu_asm.S \
					ppc405_breakpoint.S

PPC405-TIMER-C-SOURCES = 
PPC405-TIMER-S-SOURCES = 

PPC405-THREAD-C-SOURCES += 
PPC405-THREAD-S-SOURCES += ppc405_thread_init.S

PPC405-MMU-C-SOURCES += ppc405_mmu.c
PPC405-MMU-S-SOURCES += 

PPC405_OBJECTS += $(PPC405-C-SOURCES:.c=.o) $(PPC405-S-SOURCES:.S=.o)



