/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/mem/ocmb_membuf.h $                               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2019                        */
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
#ifndef __OCMB_MEMBUF_H__
#define __OCMB_MEMBUF_H__

#include "ssx.h"
#include "memory.h"
#include "membuf_structs.h"


extern MemBufConfiguration_t G_membufConfiguration;

/**
 *  Control ocmb memory buffer thottling
 *  @param[in] The memroy control task
 *  @return TRUE if throttle values changed, otherwise FALSE.
 */
bool ocmb_control( memory_control_task_t * i_memControlTask );

/**
 * Initialize structures for throttle control on ocmb memory data buffer
 */
void ocmb_control_init( void );

#endif
