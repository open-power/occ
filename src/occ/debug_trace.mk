# $Id$

#  @file debug_trace.mk
#
#  @brief mk occ application
#

#  @page ChangeLogs Change Logs
#  @section debug_trace.mk
#  @verbatim
#
#
# Change Log ******************************************************************
# Flag     Defect/Feature  User        Date         Description
# ------   --------------  ----------  ------------ -----------
# @rc001                    rickylie  	01/09/2012    Created
#
# @endverbatim
#

#*******************************************************************************
# Defines
#*******************************************************************************
#//>@rc001a

ifdef MAIN_DEBUG
D += -DMAIN_DEBUG=1
endif

ifdef RTLS_DEBUG
D += -DRTLS_DEBUG=1
endif

ifdef THRD_DEBUG
D += -DTHRD_DEBUG=1
endif

ifdef PROC_DEBUG
D += -DPROC_DEBUG=1
endif

ifdef AMEC_DEBUG
D += -DAMEC_DEBUG=1
endif

ifdef APLT_DEBUG
D += -DAPLT_DEBUG=1
endif

ifdef DCOM_DEBUG
D += -DDCOM_DEBUG=1
endif

ifdef ERRL_DEBUG
D += -DERRL_DEBUG=1
endif

ifdef APSS_DEBUG
D += -DAPSS_DEBUG=1
endif

ifdef DPSS_DEBUG
D += -DDPSS_DEBUG=1
endif

ifdef SNSR_DEBUG
D += -DSNSR_DEBUG=1
endif

ifdef TMER_DEBUG
D += -DTMER_DEBUG=1
endif

#//@<rc001a	

#*******************************************************************************
