#  @file gpefiles.mk
#
#  @brief mk occ application
#

#  @page ChangeLogs Change Logs
#  @section gpefiles.mk
#  @verbatim
#
#
# Change Log ******************************************************************
# Flag     Defect/Feature  User        Date         Description
# ------   --------------  ----------  ------------ -----------
#  @rc003                  rickylie    02/03/2012   Verify & Clean Up OCC Headers & Comments
#  @pb00D                  pbavari     02/21/2012   Changed extension from .S to .pS
#  @pb00E                  pbavari     03/11/2012   ODE Makefile support
#  @ly003  861535          lychen      11/19/2012   Remove APSS configuration/gathering of Altitude & Temperature
#
# @endverbatim
#
# @pb00Dc - Changed extension to .pS as we are now using gnu assembler for
#           compiling below files. This was done to distinguish it in the
#           makefile from other .S file which is compiled using PGAS PPC 
#           assembler.
occ_GPEFILES = gpe/apss_init.pS \
               gpe/apss_composite.pS \
               gpe/apss_meas_read_start.pS \
               gpe/apss_meas_read_cont.pS \
               gpe/pore_nop.pS \
               gpe/apss_meas_read_complete.pS

all_gpefiles = ${occ_GPEFILES:.pS=.o}

