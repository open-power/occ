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
# @gm043   928988          milesg      06/19/2014   Added pstApplet
#
# @endverbatim
#
##########################################################################
# INCLUDES
##########################################################################
# New test applet source file must to listed as part of the SOURCES variable
# to create test applet image. This will only create the image. It will not
# add these images as part of the mainstore single image. To make it part of
# of the mainstore single image, please see comments for IMAGE_TO_COMBINE 
# variable
C-SOURCES = sensorTest.c apsstest.c errlTest.c traceTest.c pstApplet.c

TESTAPLT_OBJECTS = $(C-SOURCES:.c=.o)

sensorTest_OFILES = sensorTest.o
apsstest_OFILES = apsstest.o
errlTest_OFILES = errlTest.o
traceTest_OFILES = traceTest.o
pstApplet_OFILES = pstApplet.o
			
			

