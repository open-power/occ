/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/sbefw/pool.H $                                    */
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

#ifndef SBE_VECTOR_POOL_H
#define SBE_VECTOR_POOL_H

namespace SBEVECTORPOOL
{

// Size of a block  for a vector
static const size_t G_BLOCKSIZE = 512;

//Pool size
static const size_t G_POOLSIZE = 4;

typedef struct
{
    size_t refCount;
    uint8_t data[G_BLOCKSIZE];
}vectorMemPool_t;

/**
  * @brief Returns memory pool block.
  *
  * @return  Memory block if available, NULL otherwise.
  */
vectorMemPool_t * allocMem();

/**
  * @brief Release memory pool block.
  *
  * @param[in] i_pool pool pointer.
  */
void releaseMem( vectorMemPool_t * i_pool );

} // namespace SBEVECTORPOOL
#endif //SBE_VECTOR_POOL_H
