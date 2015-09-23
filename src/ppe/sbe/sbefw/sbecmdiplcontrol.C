/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/sbefw/sbecmdiplcontrol.C $                        */
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
 * @file: ppe/sbe/sbefw/sbecmdiplcontrol.C
 *
 * @brief This file contains the SBE FIFO Commands
 *
 */

#include "sbecmdiplcontrol.H"
#include "sbefifo.H"
#include "sbetrace.H"
#include "sbe_sp_intf.H"

// Forward declaration

uint32_t sbeExecuteIstep (const uint8_t i_major, const uint8_t i_minor);
bool validateIstep (const uint8_t i_major, const uint8_t i_minor);

// @TODO via RTC 129073.
// Just a dummy code for HWP to test the flow.
// Remove it once complete flow is ready
uint32_t istep1SuccessHwp(  ) { SBE_DEBUG("istep1SuccessHwp"); return 0; }
uint32_t istep1FailHwp(  ) { SBE_DEBUG("istep1FailHwp"); return 1; }


//typedefs
// @TODO via RTC 129073.
// This is currently not defined as actual HWP signature as it
// will break compilation. Once Greg FAPI codeis in master, we will
// change it
typedef uint32_t (*sbe_istep_hwp)();

// Wrapper function for HWP IPl functions
typedef uint32_t (*sbe_istep)( sbe_istep_hwp );

// Wrapper function which will call HWP with Proc target.
uint32_t istepWithProc( sbe_istep_hwp i_hwp );

//structure for mapping SBE wrapper and HWP functions
typedef struct
{
    sbe_istep istepWrapper;
    sbe_istep_hwp istepHwp;
}istepMap_t;

// Major isteps which are supported
typedef enum
{
    SBE_ISTEP2 = 2,
    SBE_ISTEP4 = 4,
    SBE_ISTEP5 = 5,
}sbe_supported_steps_t;

// constants
// @TODO via RTC 129073.
// These are random numbers now. Will fill up
// once IPL flow document is in better shape.
const uint32_t ISTEP2_MAX_SUBSTEPS = 15;
const uint32_t ISTEP4_MAX_SUBSTEPS = 2;
const uint32_t ISTEP5_MAX_SUBSTEPS = 4;

// File static data
// @TODO via RTC 129073.
// Initialise pointer tables.

static istepMap_t g_istep2PtrTbl[ ISTEP2_MAX_SUBSTEPS ] =
                    {
                        { NULL, NULL },
                        { &istepWithProc, &istep1FailHwp },
                        { &istepWithProc, &istep1SuccessHwp }

                    };
static istepMap_t g_istep4PtrTbl[ ISTEP4_MAX_SUBSTEPS ];
static istepMap_t g_istep5PtrTbl[ ISTEP5_MAX_SUBSTEPS ];

// Functions
//----------------------------------------------------------------------------
uint32_t sbeHandleIstep (uint8_t *i_pArg)
{
    #define SBE_FUNC "sbeHandleIstep "
    SBE_DEBUG(SBE_FUNC);
    uint32_t rc = SBE_SEC_OPERATION_SUCCESSFUL;
    //@TODO via RTC 129073.
    //Use proper initialisation for fapi RC
    uint32_t fapiRc = SBE_SEC_OPERATION_SUCCESSFUL;
    uint8_t len = 0;
    sbeIstepReqMsg_t req;
    sbeResponseGenericHeader_t respHdr;
    respHdr.init();
    sbeResponseFfdc_t ffdc;

    // NOTE: In this function we will have two loops
    // First loop will deque data and prepare the response
    // Second response will enque the data on DS FIFO
    //loop 1
    do
    {
        // @TODO via RTC : 130575
        // Optimize both the RC handling and
        // FIFO operation infrastructure.
        len = sizeof( req )/sizeof(uint32_t);
        rc = sbeUpFifoDeq_mult ( len, (uint32_t *)&req);
        if (rc) //FIFO access issue
        {
            SBE_ERROR(SBE_FUNC"FIFO dequeue failed, rc[0x%X]", rc);
            break;
        }
        len = 1;
        rc = sbeUpFifoDeq_mult ( len, NULL, true );

        // If we didn't receive EOT yet
        if ( rc != SBE_FIFO_RC_EOT_ACKED )
        {
            SBE_ERROR(SBE_FUNC"FIFO dequeue failed, rc[0x%X]", rc);
            break;
        }
        // override Rc as we do not want to treat SBE_FIFO_RC_EOT_ACKED as error
        rc = SBE_SEC_OPERATION_SUCCESSFUL;

        SBE_DEBUG(SBE_FUNC"Major number:0x%08x minor number:0x%08x",
                  req.major, req.minor );

        if( false == validateIstep( req.major, req.minor ) )
        {
            SBE_ERROR(SBE_FUNC" Invalid Istep. major:0x%08x"
                      " minor:0x%08x", req.major, req.minor);
            // @TODO via RTC 129073.
            // Need to change code asper better error handling.
            respHdr.setStatus( SBE_PRI_INVALID_DATA,
                               SBE_SEC_GENERIC_FAILURE_IN_EXECUTION);
            break;
        }
        fapiRc = sbeExecuteIstep( req.major, req.minor );
        if( fapiRc )
        {
            SBE_ERROR(SBE_FUNC" sbeExecuteIstep() Failed. major:0x%08x"
                                      " minor:0x%08x", req.major, req.minor);
            respHdr.setStatus( SBE_PRI_GENERIC_EXECUTION_FAILURE,
                               SBE_SEC_GENERIC_FAILURE_IN_EXECUTION);
            ffdc.setRc(fapiRc);
        }

    }while(0);

    //loop 2
    do
    {
        // FIFO error
        if ( rc )
        {
            break;
        }

        uint32_t distance = 1; //initialise by 1 for entry count itself.
        len = sizeof(respHdr)/sizeof(uint32_t);
        // sbeDownFifoEnq_mult.
        rc = sbeDownFifoEnq_mult ( len, ( uint32_t *) &respHdr);
        if (rc)
        {
            break;
        }
        distance += len;

        // If no ffdc , exit;
        if( ffdc.fapiRc )
        {
            len = sizeof(ffdc)/sizeof(uint32_t);
            rc = sbeDownFifoEnq_mult ( len, ( uint32_t *) &ffdc);
            if (rc)
            {
                break;
            }
            distance += len;
        }
        len = sizeof(distance)/sizeof(uint32_t);
        //@TODO via 129076.
        //Need to add FFDC data as well.
        rc = sbeDownFifoEnq_mult ( len, &distance);
        if (rc)
        {
            break;
        }
    }while(0);

    if( rc )
    {
        SBE_ERROR( SBE_FUNC"Failed. rc[0x%X]", rc);
    }
    return rc;
    #undef SBE_FUNC
}

//----------------------------------------------------------------------------
// @TODO via RTC 129073.
// Change return code as per design
// @note This is the responsibilty of caller to verify major/minor
//       number before calling this function

// @TODO via RTC 129077.
// This function should check for system checkstop as well.
uint32_t sbeExecuteIstep (const uint8_t i_major, const uint8_t i_minor)
{
    #define SBE_FUNC "sbeExecuteIstep "
    SBE_DEBUG(SBE_FUNC"Major number:0x%x minor number:0x%x",
                       i_major, i_minor );
    uint32_t rc = 0;

    switch( i_major )
    {
        case SBE_ISTEP2:
            rc = (g_istep2PtrTbl[i_minor-1].istepWrapper)(
                                g_istep2PtrTbl[i_minor-1].istepHwp);
            break;

        case SBE_ISTEP4:
            rc = (g_istep4PtrTbl[i_minor-1].istepWrapper)(
                              g_istep4PtrTbl[i_minor-1].istepHwp);
            break;

        case SBE_ISTEP5:
            rc = (g_istep5PtrTbl[i_minor-1].istepWrapper)(
                              g_istep5PtrTbl[i_minor-1].istepHwp);
            break;

        // We should never reach here as before calling this validation has
        // been done.
        // @TODO via RTC 129166.
        // assert if we reach in default case.
        default:
            break;
        }

    return rc;
    #undef SBE_FUNC
}

//----------------------------------------------------------------------------
bool validateIstep (const uint8_t i_major, const uint8_t i_minor)
{
    bool valid = true;
    do
    {
        if( 0 == i_minor )
        {
            valid = false;
            break;
        }

        switch( i_major )
        {
            case SBE_ISTEP2:
                // istep 2.1 loads image to PIBMEM
                // So SBE control loop can not execute istep 2.1.
                if(( i_minor > ISTEP2_MAX_SUBSTEPS ) || ( i_minor == 1) )
                {
                    valid = false;
                }
                break;

            case SBE_ISTEP4:
                if( i_minor > ISTEP4_MAX_SUBSTEPS )
                {
                    valid = false;
                }
                break;

            case SBE_ISTEP5:
                if( i_minor > ISTEP5_MAX_SUBSTEPS )
                {
                    valid = false;
                }
                break;

            default:
                valid= false;
                break;
        }
    } while(0);

    return valid;
}

//----------------------------------------------------------------------------

uint32_t istepWithProc( sbe_istep_hwp i_hwp)
{
    SBE_DEBUG("istepWithProc");
    uint32_t rc = 0;
    if( i_hwp )
    {
        rc = i_hwp();
    }
    return rc;
}

//----------------------------------------------------------------------------

uint32_t sbeWaitForSbeIplDone (uint8_t *i_pArg)
{
    uint32_t rc = 0;
    SBE_TRACE("sbeWaitForSbeIplDone");


    return rc;
}
