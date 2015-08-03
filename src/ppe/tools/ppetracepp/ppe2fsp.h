/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/tools/ppetracepp/ppe2fsp.h $                          */
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


#define P2F_NULL_POINTER                1
#define P2F_INVALID_INPUT_SIZE          2
#define P2F_INVALID_PPE_OFFSET          3
#define P2F_OUTPUT_BUFFER_TOO_SMALL     4
#define P2F_INPUT_BUFFER_TOO_SMALL      5
#define P2F_INVALID_VERSION             6

int ppe2fsp(void* in, unsigned long in_size, void* out, unsigned long* io_size);
