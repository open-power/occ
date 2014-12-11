# $Id: libgpefiles.mk,v 1.3 2014/06/26 12:48:31 cmolsen Exp $
# $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/libgpefiles.mk,v $
#  @file libgpefiles.mk
#
#  @brief mk for libssx.a gpe object files
#
#  @page ChangeLogs Change Logs
#  @section libgpefiles.mk
#  @verbatim
#
#
# Change Log ******************************************************************
# Flag     Defect/Feature  User        Date         Description
# ------   --------------  ----------  ------------ -----------
# @at007                   alvinwan    05/25/2012   Use complex method for linking pore and PPC objects
#
# @endverbatim
#
##########################################################################
# INCLUDES
##########################################################################
pS-SOURCES = \
	gpe_control.pS \
	gpe_data.pS \
	gpe_scom.pS \
	gpe_pba_pgas.pS

LIB_PSOBJECTS = ${pS-SOURCES:.pS=.o}


