/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: chips/p9/procedures/lib/pm/p9_config.h $                      */
/*                                                                        */
/* IBM CONFIDENTIAL                                                       */
/*                                                                        */
/* EKB Project                                                            */
/*                                                                        */
/* COPYRIGHT 2015,2016                                                    */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* The source code for this program is not published or otherwise         */
/* divested of its trade secrets, irrespective of what has been           */
/* deposited with the U.S. Copyright Office.                              */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

/// \file p9_config.h
/// \brief Chip configuration data structures for P9 OCC procedures
///
//  *HWP HWP Owner: Doug Gilbert <dgilbert@us.ibm.com>
//  *HWP FW Owner: Martha Broyles <mbroyles@us.ibm.com>
//  *HWP Team: PM
//  *HWP Level: 1
//  *HWP Consumed by: OCC

#ifndef __P9_GPE_CONFIG_H__
#define __P9_GPE_CONFIG_H__


#include <stdint.h>


/// SCOM address Ranges:
// Cores (EX chiplet): 0x20000000 - 0x37000000
// Caches: 0x10000000 - 0x15000000
//
#define CHIPLET_CORE_SCOM_BASE  0x20000000
#define CHIPLET_CACHE_SCOM_BASE 0x10000000

#define CHIPLET_CORE_ID(n) \
    (((n) << 24) + CHIPLET_CORE_SCOM_BASE)

#define CHIPLET_CACHE_ID(n) \
    (((n) << 24) + CHIPLET_CACHE_SCOM_BASE)


#endif  /* __P9_GPE_CONFIG_H__ */
