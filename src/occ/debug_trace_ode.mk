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
# #pb00E                    pbavari     03/11/2012    ODE Makefile support
# @endverbatim
#

#*******************************************************************************
# Defines
#*******************************************************************************
#//>@rc001a

.ifdef MAIN_DEBUG
DEFS += -DMAIN_DEBUG=1
.endif

.ifdef RTLS_DEBUG
DEFS += -DRTLS_DEBUG=1
.endif

.ifdef THRD_DEBUG
DEFS += -DTHRD_DEBUG=1
.endif

.ifdef PROC_DEBUG
DEFS += -DPROC_DEBUG=1
.endif

.ifdef AMEC_DEBUG
DEFS += -DAMEC_DEBUG=1
.endif

.ifdef APLT_DEBUG
DEFS += -DAPLT_DEBUG=1
.endif

.ifdef DCOM_DEBUG
DEFS += -DDCOM_DEBUG=1
.endif

.if defined( ERRL_DEBUG )
DEFS += -DERRL_DEBUG=1
.endif

.ifdef APSS_DEBUG
DEFS += -DAPSS_DEBUG=1
.endif

.ifdef DPSS_DEBUG
DEFS += -DDPSS_DEBUG=1
.endif

.ifdef SNSR_DEBUG
DEFS += -DSNSR_DEBUG=1
.endif

.ifdef TMER_DEBUG
DEFS += -DTMER_DEBUG=1
.endif

#//@<rc001a	

#*******************************************************************************
