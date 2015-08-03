/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwpf/plat/include/error_scope.H $                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2012,2015                        */
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
/**
 * @file error_scope.H
 * @brief definitions which create a scope for automatic error handling
 */

#ifndef __FAPI2_ERROR_SCOPE__
#define __FAPI2_ERROR_SCOPE__

#include <stdint.h>
#include <thread>
#include <stdio.h>
#include <return_code.H>
#include <plat_includes.H>


extern "C"
{
#ifndef __noRC__
    extern fapi2::ReturnCode G_current_err;
#endif
    extern uint64_t G_pib_error_mask;
    extern uint64_t G_operational_state;
}


///
/// @brief Place holder until more of the FAPI infrastructure
/// can be compiled with this file
/// @param[in] ... varargs, passed to a platform specific output function
/// @note This is skeleton implementation for prototype purposes. This
/// does not imply "printf" is used on any or all platforms.
///
#define FAPI_INF( ... )
#define FAPI_ERR( ... ) PK_TRACE( __VA_ARGS__ )
//#define FAPI_DBG( ... ) PK_TRACE( __VA_ARGS__ )
#define FAPI_DBG( ... )

/// @cond
#define FAPI_VA_NARGS_IMPL(_1, _2, _3, _4, _5, N, ...) N
#define FAPI_VA_NARGS(...) FAPI_VA_NARGS_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1)

//#define FAPI_TRY_IMPL2(...) FAPI_TRY_TRACE (__VA_ARGS__)
#define FAPI_TRY_IMPL2(count, ...) FAPI_TRY ## count (__VA_ARGS__)
#define FAPI_TRY_IMPL(count, ...) FAPI_TRY_IMPL2(count, __VA_ARGS__)

#ifndef __noRC__
#define FAPI_TRY_NO_TRACE( __operation__ )                              \
    if ((fapi2::current_err = (__operation__)) != fapi2::FAPI2_RC_SUCCESS) \
    {                                                                   \
        goto clean_up;                                                  \
    }
#else
#define FAPI_TRY_NO_TRACE( __operation__ )                              \
    (__operation__);
#endif

// Why debug? Because this isn't a mechanism to gather FFDC
// one should be using FAPI_ASSERT. However, it is nice to
// have a conditional trace in the event of a failure in the
// operation, so that's why this is here.
//    if ((fapi2::current_err = (__operation__)) != fapi2::FAPI2_RC_SUCCESS)
#ifndef __noRC__
#define FAPI_TRY_TRACE( __operation__, ... )                            \
    if ((fapi2::current_err = (__operation__)) != fapi2::FAPI2_RC_SUCCESS) \
    {                                                                   \
        FAPI_DBG(__VA_ARGS__);                                          \
        goto clean_up;                                                  \
    }
#define FAPI_CLEANUP() \
clean_up:
#else
/// @todo:  something is wrong needed the goto_cleanup
#define FAPI_TRY_TRACE( __operation__,  ... )                           \
    FAPI_DBG(__VA_ARGS__);                                              \
    (__operation__)

#define FAPI_CLEANUP()

#endif

#define FAPI_TRY0 FAPI_TRY_NO_TRACE
#define FAPI_TRY1 FAPI_TRY_TRACE
#define FAPI_TRY2 FAPI_TRY_TRACE
#define FAPI_TRY3 FAPI_TRY_TRACE
#define FAPI_TRY4 FAPI_TRY_TRACE
#define FAPI_TRY5 FAPI_TRY_TRACE
/// @endcond

///
/// @brief Wrapper to check an operation for an error state
/// and jump to the label cleam_up if there is an error.
/// @param[in] __operation__ an operation which returns a fapi::ReturnCode
/// @param[in] Optional vararg format/agruments for trace output.
/// @note This implementation does not support PIB error masks or
/// FSP operational states.
/// @warning The trace information is only going to be seen during
/// debug, it's not an error or informational trace. This is because
/// traces might not be seen in the field. If you want information
/// you will see on a field error, use FAPI_ASSERT.
///
#ifdef DOXYGEN
#define FAPI_TRY(__operation__, ...) FAPI_TRY_IMPL
#else
#define FAPI_TRY(...) FAPI_TRY_IMPL(FAPI_VA_NARGS(__VA_ARGS__), __VA_ARGS__)
#endif

///
/// @brief Assert a conditional is true.
/// If it is not, the FFDC gathering function is called and the
/// trace is output as a FAPI error trace.
/// @param[in] __conditional__ the condition to assert
/// @param[in] __ffdc__ the FFDC gathering function
/// @param[in] ... varargs, as input to FAPI_ERR
///
#define FAPI_ASSERT( __conditional__, __ffdc__, ... )   \
    if (! (__conditional__))                            \
    {                                                   \
        /*       __ffdc__; */                           \
        FAPI_ERR(__VA_ARGS__);                          \
        goto clean_up;                                  \
    }

#endif  // __FAPI2_ERROR_SCOPE__
