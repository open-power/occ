# $Id: libfiles.mk,v 1.5 2014/06/26 12:51:16 cmolsen Exp $
# $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/libfiles.mk,v $
#  @file libofiles.mk
#
#  @brief mk for libssx.a object files
#
#  @page ChangeLogs Change Logs
#  @section ofiles.mk
#  @verbatim
#
#
# Change Log ******************************************************************
# Flag     Defect/Feature  User        Date         Description
# ------   --------------  ----------  ------------ -----------
# @pb00E                   pbavari     03/11/2012   Makefile ODE support
# @at007                   alvinwan    05/25/2012   Use complex method for linking pore and PPC objects
#
# @endverbatim
#
##########################################################################
# INCLUDES
##########################################################################

C-SOURCES = \
	assert.c \
	ctype.c \
	ctype_table.c \
	fgetc.c  \
	gpe_pba.c \
	gpsm.c \
	gpsm_dcm.c \
	gpsm_init.c \
	heartbeat.c \
	memcpy.c \
	memset.c \
	pmc_dcm.c \
	polling.c \
	printf.c \
	pstates.c \
	puts.c \
	simics_stdio.c \
	special_wakeup.c \
	sprintf.c \
	ssx_dump.c \
	ssx_io.c \
	stdlib.c \
	strcasecmp.c \
	strdup.c \
	string.c \
	string_stream.c \
	strtox.c \
	time.c \
	vrm.c \

S-SOURCES =  gpsm_dcm_fast_handler.S

LIBSSX_OBJECTS = $(C-SOURCES:.c=.o) $(S-SOURCES:.S=.o)
