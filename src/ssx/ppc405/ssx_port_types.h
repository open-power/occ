/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ppc405/ssx_port_types.h $                             */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2016                        */
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
#ifndef __SSX_PORT_TYPES_H__
#define __SSX_PORT_TYPES_H__

/// \file ssx_port_types.h
/// \brief Type definitions required by the SSX port.
///
/// \todo GCC provides a portable version of cntlzw called __builtin_clz().
/// We should make the SSX priority queues portable by using this facility.
///
/// \todo I think that if more of the port-dependent types were moved here, we
/// could break the circular dependencies in some of the header inclusion and
/// simplify the way the SSX/port/chip headers are included.

/// An SsxIrqId is an integer in the range of valid interrupts defined by the
/// interrupt controller.

typedef uint8_t SsxIrqId;

/// SSX requires the port to define the type SsxThreadQueue, which is a
/// priority queue (where 0 is the highest priority).  This queue must be able
/// to handle SSX_THREADS + 1 priorities (the last for the idle thread). The
/// port must also define methods for clearing, insertion, deletion and min
/// (with assumed legal priorities).  The min operation returns SSX_THREADS if
/// the queue is empty. (Or a queue could be initialized with the SSX_THREADS
/// entry always present - SSX code never tries to delete the idle thread from
/// a thread queue).
///
/// These queues are used both for the run queue and the pending queue
/// associated with every semaphore.
///
/// On PPC405 with 32 threads (implied), this is a job for a uint32_t and
/// cntlzw().

typedef uint32_t SsxThreadQueue;

#endif  /* __SSX_PORT_TYPES_H__ */
