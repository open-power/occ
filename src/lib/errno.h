/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/errno.h $                                             */
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
#ifndef __ERRNO_H__
#define __ERRNO_H__
/// \file errno.h
/// \brief Replacement for <errno.h>
///
/// SSX does not support a per-thread or global 'errno'.  The standard Unix
/// errno values returned by library functions are defined here. The prefix
/// code is the 'telephone code' for "errn".

#define EINVAL 0x00377601
#define EBADF  0x00377602
#define EAGAIN 0x00377603
#define ENXIO  0x00377604
#define ENOMEM 0x00377605

#endif  /* __ERRNO_H__ */
