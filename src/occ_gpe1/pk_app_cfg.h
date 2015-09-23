#ifndef __PK_APP_CFG_H__
#define __PK_APP_CFG_H__
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pk_app_cfg.h
/// \brief Application specific overrides go here.
///

#include "global_app_cfg.h"

#ifndef SIMICS_ENVIRONMENT
#define SIMICS_ENVIRONMENT 1
#warning Building for Simics!
#endif

/// Static configuration data for external interrupts:
///
/// IRQ#, TYPE, POLARITY, ENABLE
///
#define APPCFG_EXT_IRQS_CONFIG \
    OCCHW_IRQ_CHECK_STOP_GPE1           OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_IPI1_HI_PRIORITY          OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_IPI1_LO_PRIORITY          OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \

/// The Instance ID of the occ processor that this application is intended to run on
/// 0-3 -> GPE, 4 -> 405
#define APPCFG_OCC_INSTANCE_ID 1

// This application will statically initialize it's external interrupt table
#define STATIC_IRQ_TABLE

#define STATIC_IPC_TABLES

#endif /*__PK_APP_CFG_H__*/
