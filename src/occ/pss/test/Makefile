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
#  @rc003                  rickylie    02/03/2012   Verify & Clean Up OCC Headers & Comments
#  @ly003  861535          lychen      11/19/2012   Remove APSS configuration/gathering of Altitude & Temperature
#
# @endverbatim
#


apsstest_CFILES = \
	    ../../common.c \
	    ../../errl/errl.c \
	    ../../thread/threadSch.c \
	    ../apss.c \
	    apsstest.c
all_cfiles = ${apsstest_CFILES} 


occ_GPEFILES = ../../gpe/apss_init.S \
	       ../../gpe/apss_composite.S \
	       ../../gpe/apss_meas_read_start.S \
	       ../../gpe/apss_meas_read_cont.S \
	       ../../gpe/apss_meas_read_complete.S \
	       ../../gpe/pore_test.S
all_gpefiles = ${occ_GPEFILES}


APP     = apsstest
APP_INCLUDES += -I../../../ssx
APP_INCLUDES += -I../../../lib 
APP_INCLUDES += -I../../incl 
APP_INCLUDES += -I../../trac 
APP_INCLUDES += -I../../async
APP_INCLUDES += -I../../errl
APP_INCLUDES += -I../../gpe
APP_INCLUDES += -I../../thread
#APP_INCLUDES += -I../../aplt
#APP_INCLUDES += -I../../rtls
#APP_INCLUDES += -I../../sensor
APP_INCLUDES += -I../../pss
APP_INCLUDES += -I. 

D = -DSIMICS_MAGIC_PANIC=1

#D = -DVERIFICATION=1 \
    -DSSX_STACK_CHECK=0 \
    -DINITIALIZE_PMC=0 \
    -DINITIALIZE_SIMICS_IO=0 \
    -DINITIALIZE_RTX_IO=1 \
    -DINITIALIZE_PBA=1 \
    -DSIMICS_MAGIC_PANIC=1 \
    -DSSX_KERNEL_TRACE_ENABLE=1


SOURCES = ${all_cfiles} ${all_gpefiles}
MODE    = validation

PGP_ASYNC_SUPPORT = 1

include ./app.mk

pgas: 
	 $(CC) $(CFLAGS) -c -Wa,-al -Wa,--listing-cont-lines='10' ${all_gpefiles}
