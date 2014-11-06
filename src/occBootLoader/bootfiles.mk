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
# @rc004    882410         rickylie    05/10/2013   Pin OCC Firmware Level to an Address in Lid
#
# @endverbatim
#
##########################################################################
# INCLUDES
##########################################################################
C-SOURCES = bootMain.c ../occ/occbuildname.c
S-SOURCES = bootInit.S  ../ssx/ppc32/savegpr.S

BOOTLOADER_OBJECTS = $(C-SOURCES:.c=.o) $(S-SOURCES:.S=.o)

			
			

