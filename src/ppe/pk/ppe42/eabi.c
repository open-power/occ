/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/ppe42/eabi.c $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016                             */
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
// assuming link script instructs the c++ compiler to put
// ctor_start_address and ctor_end_address in .rodata

//extern void (*ctor_start_address)() __attribute__ ((section (".rodata")));
//extern void (*ctor_end_address)() __attribute__((section(".rodata")));
#ifdef __cplusplus
    extern "C"
#endif
__attribute__((weak)) void __eabi()
{
    // This is the default eabi and can be overridden.
    // eabi environment is already set up by the PK kernel
    // Call static C++ constructors if you use C++ global/static objects

    //void(**ctors)() = &ctor_start_address;
    //while(ctors != &ctor_end_address)
    //{
    //    (*ctors)();
    //    ctors++;
    //}
}


