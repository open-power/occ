# $Id: ssxpgpfiles.mk,v 1.4 2014/06/26 12:56:28 cmolsen Exp $
# $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/ssxpgpfiles.mk,v $
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
# Object Files
##########################################################################
PGP-C-SOURCES = pgp_init.c pgp_irq_init.c pgp_pmc.c pgp_ocb.c pgp_pba.c \
	pgp_id.c pgp_centaur.c
PGP-S-SOURCES = pgp_cache.S

PGP-TIMER-C-SOURCES = 
PGP-TIMER-S-SOURCES = 

PGP-THREAD-C-SOURCES = 
PGP-THREAD-S-SOURCES = 

PGP-ASYNC-C-SOURCES = pgp_async.c pgp_async_pore.c pgp_async_ocb.c \
	pgp_async_pba.c
PGP-ASYNC-S-SOURCES =

PGP_OBJECTS += $(PGP-C-SOURCES:.c=.o) $(PGP-S-SOURCES:.S=.o)

