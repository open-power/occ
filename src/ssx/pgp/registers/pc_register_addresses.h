#ifndef __PC_REGISTER_ADDRESSES_H__
#define __PC_REGISTER_ADDRESSES_H__

// $Id: pc_register_addresses.h,v 1.1.1.1 2013/12/11 21:03:25 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/registers/pc_register_addresses.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pc_register_addresses.h
/// \brief Symbolic addresses for the PC unit

// *** WARNING *** - This file is generated automatically, do not edit.


#define PC_PCB_BASE 0x10010000
#define PC_PFTH_MODEREG 0x100132a7
#define PC_OCC_SPRC 0x100132ab
#define PC_OCC_SPRD 0x100132ac
#define PC_PFTH_THROT_REG 0x100132ad
#define PC_DIRECT_CONTROLN(n) (PC_DIRECT_CONTROL0 + ((PC_DIRECT_CONTROL1 - PC_DIRECT_CONTROL0) * (n)))
#define PC_DIRECT_CONTROL0 0x10013000
#define PC_DIRECT_CONTROL1 0x10013010
#define PC_DIRECT_CONTROL2 0x10013020
#define PC_DIRECT_CONTROL3 0x10013030
#define PC_DIRECT_CONTROL4 0x10013040
#define PC_DIRECT_CONTROL5 0x10013050
#define PC_DIRECT_CONTROL6 0x10013060
#define PC_DIRECT_CONTROL7 0x10013070
#define PC_RAS_MODEREGN(n) (PC_RAS_MODEREG0 + ((PC_RAS_MODEREG1 - PC_RAS_MODEREG0) * (n)))
#define PC_RAS_MODEREG0 0x10013001
#define PC_RAS_MODEREG1 0x10013011
#define PC_RAS_MODEREG2 0x10013021
#define PC_RAS_MODEREG3 0x10013031
#define PC_RAS_MODEREG4 0x10013041
#define PC_RAS_MODEREG5 0x10013051
#define PC_RAS_MODEREG6 0x10013061
#define PC_RAS_MODEREG7 0x10013071
#define PC_RAS_STATUSN(n) (PC_RAS_STATUS0 + ((PC_RAS_STATUS1 - PC_RAS_STATUS0) * (n)))
#define PC_RAS_STATUS0 0x10013002
#define PC_RAS_STATUS1 0x10013012
#define PC_RAS_STATUS2 0x10013022
#define PC_RAS_STATUS3 0x10013032
#define PC_RAS_STATUS4 0x10013042
#define PC_RAS_STATUS5 0x10013052
#define PC_RAS_STATUS6 0x10013062
#define PC_RAS_STATUS7 0x10013072
#define PC_POW_STATUSN(n) (PC_POW_STATUS0 + ((PC_POW_STATUS1 - PC_POW_STATUS0) * (n)))
#define PC_POW_STATUS0 0x10013004
#define PC_POW_STATUS1 0x10013014
#define PC_POW_STATUS2 0x10013024
#define PC_POW_STATUS3 0x10013034
#define PC_POW_STATUS4 0x10013044
#define PC_POW_STATUS5 0x10013054
#define PC_POW_STATUS6 0x10013064
#define PC_POW_STATUS7 0x10013074

#endif // __PC_REGISTER_ADDRESSES_H__

