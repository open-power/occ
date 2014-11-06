#ifndef __HEARTBEAT_H__
#define __HEARTBEAT_H__

// $Id: heartbeat.h,v 1.3 2014/02/12 05:48:48 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/heartbeat.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file heartbeat.h
/// \brief PgP PMC/PCBS heartbeat configuration procedures

#ifndef __ASSEMBLER__

#include <stdint.h>

#define HB_INVALID_ARGUMENT_PMC  0x00482801
#define HB_INVALID_ARGUMENT_PCBS 0x00482802
#define HB_UNDERFLOW_DIVIDER_PMC 0x00482803
#define HB_OVERFLOW_DIVIDER_PMC  0x00482804
#define HB_OVERFLOW_PULSES_PMC   0x00482805
#define HB_OVERFLOW_PULSES_PCBS  0x00482806
#define HB_UNDERFLOW_PULSES_PCBS 0x00482807

int
pmc_hb_config(unsigned int enable,
              unsigned int req_time_us,
              unsigned int force,
              unsigned int *o_time_us);

int
pcbs_hb_config(unsigned int enable,
               ChipConfigCores cores,
               uint32_t hb_reg,
               unsigned int req_time_us,
               unsigned int force,
               unsigned int *o_time_us);



#endif  /* __ASEMBLER__ */


#endif  /* __HEARTBEAT_H__ */
