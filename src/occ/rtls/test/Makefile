# $Id$

#  @file Makefile
#
#  @brief Makefile occ application
#

#  @page ChangeLogs Change Logs
#  @section Makefile
#  @verbatim
#
#
# Change Log ******************************************************************
# Flag     Defect/Feature  User        Date         Description
# ------   --------------  ----------  ------------ -----------
#                          np, dw      08/10/2011   created by nguyenp & dwoodham
#                                                   borrowed from occ/thread/test/app.mk
#  @dw000                  dwoodham    12/13/2011   compile with applet header & ID
#  @rc003                  rickylie    02/03/2012   Verify & Clean Up OCC Headers & Comments
#
# @endverbatim
#
# Note: This compiles the _local_ test copy of rtls_tables.c, _NOT_ the production copy.

rtlstest_CFILES = \
        ../../common.c \
        ../../errl/errl.c \
        ../rtls.c \
        rtls_tables.c \
        main.c

all_cfiles = ${rtlstest_CFILES}

APP           =  rtlstest
APP_INCLUDES += -I../../../ssx
APP_INCLUDES += -I../../../lib 
APP_INCLUDES += -I../../incl 
APP_INCLUDES += -I../../trac 
APP_INCLUDES += -I../../errl
APP_INCLUDES += -I../../thread
APP_INCLUDES += -I../../rtls
APP_INCLUDES += -I../../aplt/incl
APP_INCLUDES += -I. 

D = -DOCC_FIRMWARE=1

#D = -DVERIFICATION=1 \
    -DSSX_STACK_CHECK=0 \
    -DINITIALIZE_PMC=0 \
    -DINITIALIZE_SIMICS_IO=0 \
    -DINITIALIZE_RTX_IO=1 \
    -DINITIALIZE_PBA=1 \
    -DSIMICS_MAGIC_PANIC=1 \
    -DSSX_KERNEL_TRACE_ENABLE=1


SOURCES = ${all_cfiles} 
MODE    = validation

PGP_ASYNC_SUPPORT = 1

include ./app.mk

pgas: 
	 $(CC) $(CFLAGS) -c -Wa,-al -Wa,--listing-cont-lines='10'
