/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/sbefw/sbecmdscomaccess.H $                        */
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
/*
 * @file: ppe/sbe/sbefw/sbecmdscomaccess.H
 *
 * @brief This file contains the Interfaces for the SCOM Access chip-ops
 *
 */

#ifndef __SBEFW_SBECMDSCOMACCESS_H
#define __SBEFW_SBECMDSCOMACCESS_H

#include <stdint.h>

/**
  * @brief sbeDownFifoGetStatus : Write data into Downstream FIFO
  *
  * @param[in] i_pArg Buffer to be passed to the function (not used as of now)
  *
  * @return    Rc from the FIFO access utility
  */
uint32_t sbeGetScom (uint8_t *i_pArg);


/**
  * @brief sbeDownFifoGetStatus : Write data into Downstream FIFO
  *
  * @param[in] i_pArg Buffer to be passed to the function (not used as of now)
  *
  * @return    Rc from the FIFO access utility
  */
uint32_t sbePutScom (uint8_t *i_pArg);




#endif /* __SBEFW_SBECMDSCOMACCESS_H */
