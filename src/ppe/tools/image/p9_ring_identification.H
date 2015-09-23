/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/tools/image/p9_ring_identification.H $                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
#ifndef _P9_RING_IDENT_H_
#define _P9_RING_IDENT_H_
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h> 


// Ring ID list structure.
typedef struct {
  const char  *ringName;
  uint8_t     ringId;
  uint8_t     chipIdMin;    // the min chipletId
  uint8_t     chipIdMax;    // the max chipletId
  const char  *ringNameImg; // Ring name in image: ringName + "_ring"
  uint8_t     vpdKeyword;
  uint8_t      bWcSpace;     // 0: fitted  1: worst-case space (3 x ring length)
} RingIdList;

extern const RingIdList  RING_ID_LIST_PG[], RING_ID_LIST_PR[];
extern const uint32_t    RING_ID_LIST_PG_SIZE, RING_ID_LIST_PR_SIZE;
extern const RingIdList  RING_ID_LIST[];
extern const uint32_t    RING_ID_LIST_SIZE;

// Enumerated VPD keyword values.
// Note! This is DIFFERENT from the MvpdKeyword list in fapiMvpdAccess.H which
//       can't be used in this file since it's not, per se, a fapi file. So
//       these values need to be translated in xip_customize when passing the
//       mvpdKeyword to getMvpdRing();
enum VpdKeyword  {
  VPD_KEYWORD_PDG,
  VPD_KEYWORD_PDR,
  NUM_OF_VPD_TYPES
};

int get_vpd_ring_list_entry(const char *i_ringName,
                            const uint8_t i_ringId,
                            RingIdList **i_ringIdList);


#endif
