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
# @pb00E                   pbavari     03/28/2012   Makefile ODE support
#
# @endverbatim
#
##########################################################################
# INCLUDES
##########################################################################
C-SOURCES = apssInitApplet.c sensorQueryList.c cmdhDbugCmd.c

PRDTAPLT_OBJECTS = $(C-SOURCES:.c=.o)

apssInitApplet_OFILES = apssInitApplet.o
sensorQueryList_OFILES = sensorQueryList.o
cmdhDbugCmd_OFILES = cmdhDbugCmd.o

			
			

