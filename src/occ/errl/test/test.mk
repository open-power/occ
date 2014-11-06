# $Id$

#  @file test.mk
#
#  @brief mk occ application
#

#  @page ChangeLogs Change Logs
#  @section test.mk
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

errltest_CFILES = \
            ../../common.c \
            ../errl.c \
            ../../thread/threadSch.c \
            errltest.c            

all_cfiles = ${errltest_CFILES}

