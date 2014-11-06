#ifndef __VRM_H__
#define __VRM_H__

// $Id: vrm.h,v 1.2 2014/02/03 01:30:26 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/vrm.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file vrm.h
/// \brief PgP SPIVRM procedures

#include "ssx.h"

#ifndef __ASSEMBLER__

int
o2s_initialize();

int
spivid_initialize();


int
vrm_voltage_write(int vrm_select, 
                  uint8_t vdd_vid, 
                  int8_t vcs_offset, 
                  int phases);

int
vrm_read_state(int vrm_select, 
                  int  *mnp1,
                  int  *mn,
                  int  *vfan,
                  int  *vovertmp);

int
vrm_voltage_read(int vrm_select,
                 uint8_t vrail,
                 uint8_t *o_vid);

#endif  /* __ASEMBLER__ */

// Error/panic codes

#define O2S_BUSY_VRM_VOLTAGE_READ   0x00627001
#define O2S_BUSY_VRM_VOLTAGE_WRITE  0x00627002
#define O2S_BUSY_VRM_READ_STATE     0x00627003
#define O2S_READ_NOT_READY          0x00627004
#define O2S_WRITE_NOT_VALID         0x00627005
#define O2S_WRITE_ECC_ERROR         0x00627006
#define VRM_INVALID_ARGUMENT_VREAD  0x00627007
#define VRM_INVALID_ARGUMENT_VWRITE 0x00627008 
#define VRM_INVALID_ARGUMENT_SREAD  0x00627009 
#define VRM_INVALID_ARGUMENT_INIT   0x0062700a

#endif  /* __VRM_H__ */
