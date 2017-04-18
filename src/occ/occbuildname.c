/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/occbuildname.c $                                      */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2017                        */
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

// BUILD_FIPS is a compiler environment variable that will be set by the
// build team when compiling code for FSP supported systems.
// Otherwise an open power occ version will be assigned below when code is
// pushed up to github.

#ifdef BUILD_FIPS

volatile const char G_occ_buildname[16] __attribute__((section(".buildname"))) = /*<BuildName>*/  BUILD_FIPS  /*</BuildName>*/ ;

#else

volatile const char G_occ_buildname[16] __attribute__((section(".buildname"))) = /*<BuildName>*/  "op_occ_170410a\0"  /*</BuildName>*/ ;

#endif
