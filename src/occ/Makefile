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
#
# @endverbatim
#

include cfiles.mk
include gpefiles.mk
include debug_trace.mk

APP     = occ
APP_INCLUDES += -I../ssx
APP_INCLUDES += -I../lib 
APP_INCLUDES += -I./incl 
APP_INCLUDES += -I./trac 
APP_INCLUDES += -I./async
APP_INCLUDES += -I./errl
APP_INCLUDES += -I./gpe 
APP_INCLUDES += -I./thread
APP_INCLUDES += -I./aplt
APP_INCLUDES += -I./aplt/incl
APP_INCLUDES += -I./rtls
APP_INCLUDES += -I./dcom
APP_INCLUDES += -I./sensor
APP_INCLUDES += -I./pss
APP_INCLUDES += -I./proc
APP_INCLUDES += -I./cent
APP_INCLUDES += -I./cmdh
APP_INCLUDES += -I./amec
APP_INCLUDES += -I. 
APP_INCLUDES += -I../occ 
APP_INCLUDES += -I../../occ 
APP_INCLUDES += -I./timer

export APP_INCLUDES

D += -DOCC_FIRMWARE=1 \
    -DSIMICS_MAGIC_PANIC=1 \
    -DOCC=1 \
    -DNONCACHEABLE_SUPPORT=1 \
    -DUSE_SSX_APP_CFG_H=1     
#D = -DVERIFICATION=1 \
    -DSSX_STACK_CHECK=0 \
    -DINITIALIZE_PMC=0 \
    -DINITIALIZE_SIMICS_IO=0 \
    -DINITIALIZE_RTX_IO=1 \
    -DINITIALIZE_PBA=1 \
    -DSIMICS_MAGIC_PANIC=1 \
    -DSSX_KERNEL_TRACE_ENABLE=1

# If this makefile is called as "make NO_TRAC_STRINGS=1" then trace strings 
# won't be built into the image.  This will be used for metrics regarded to the 
# realistic OCC Code Size. Note that "make clean" must be run before this define
# will be picked up by the compiler, otherwise previously compiled objects will 
# be used.  You can also see the space used by strings by running: 
# strings occ.bin | \grep "ERR\|INF\|IMP" | sed 's/^...: %s: //g' | wc -m
ifdef NO_TRAC_STRINGS
D += -DNO_TRAC_STRINGS=1   # @th002a
endif

# @pb009a - Added define to be used to enable/disable debug traces
# usage to enable debug traces: "make OCC_DBG_TRACE=1"
# @rc001d - remove OCC_DBG_TRACE 01/16/2012 - trace debug improvement


# Added define to be used to enable/disable OCC_ALONE simics model
# supported components. The only components not supported on occ_alone
# is FSP & PHYP, so this will be used to fake out all commands we need
# from FSP, and automatically go ACTIVE.
# usage: "make FSPLESS_SIMICS=1" to enable this mode
ifdef FSPLESS_SIMICS      # @th029c
D += -DFSPLESS_SIMICS=1
endif

# Added define to be used to enable GCOV based code coverage tools.
# For now, this only works in the "occ" source files, not ssx or lib.
# usage: "make GCOV_CODE_COVERAGE=1"
ifdef GCOV_CODE_COVERAGE   # @th029a
D += -DGCOV_CODE_COVERAGE=1
endif

SOURCES = ${all_cfiles} ${all_gpefiles}
MODE    = validation

PGP_ASYNC_SUPPORT = 1

include ./app.mk

pgas: 
	 $(CC) $(CFLAGS) -c -Wa,-al -Wa,--listing-cont-lines='10' ${all_gpefiles}
