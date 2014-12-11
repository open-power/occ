/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occApplet/testApplet/pstApplet.c $                        */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* COPYRIGHT International Business Machines Corp. 2011,2014              */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
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

//*************************************************************************
// Includes
//*************************************************************************
#include <common_types.h>   // imageHdr_t declaration and image header macro
#include <errl.h>           // For error handle
#include "ssx_io.h"         // For printfs
#include <trac.h>           // For traces
#include <appletId.h>       // For applet ID
#include "ssx.h"
#include "pgp_common.h"
#include "pstates.h"
#include "gpsm.h"

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define MYENTRYPOINT_ID  "pstApplet\0"

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

//*************************************************************************
// Entry point function
//*************************************************************************
errlHndl_t myEntryPoint(void * i_arg)
{

    TRAC_INFO("Enter");
    //initialize variables
    errlHndl_t l_err = NULL;
    int rc=0;
    GlobalPstateTable * l_gpst_ptr = NULL;
    l_gpst_ptr = gpsm_gpst();
    unsigned int deconfigured_cores = in32(PMC_CORE_DECONFIGURATION_REG);
    pcbs_power_management_control_reg_t pmcr;
    pmcr.value=0;
    unsigned char trace_count[PGP_NCORES] = {0};


    //get minimum and maximum p states
    Pstate min_ps = gpst_pmin(l_gpst_ptr);
    Pstate max_ps = gpst_pmax(l_gpst_ptr);
    int iterations;
    for(iterations=0;iterations<1000;iterations++){
        //loop through the functioning cores
        int core;
        for (core = 0; core < PGP_NCORES; core++) {
            if (deconfigured_cores & (0x80000000 >> core)) continue;
            //go through all the valid power states
            int ps;
            for (ps=min_ps;ps<=max_ps;ps++){

                //Issue scoms l
                pmcr.fields.local_pstate_req=ps;
                pmcr.fields.global_pstate_req=ps;
                rc = _putscom(CORE_CHIPLET_ADDRESS(PCBS_POWER_MANAGEMENT_CONTROL_REG,core),pmcr.value, SCOM_TIMEOUT * 20);
                if (rc) {
                    if(trace_count[core] < 5)
                    {
                        trace_count[core]++;
                        TRAC_ERR("pstApplet: putscom failed on core %d with rc = 0x%08x, deconfig_cores = 0x%08x, iteration = %d",
                                 core, rc, deconfigured_cores, iterations);
                    }
                    if(rc == 7)
                    {
                        continue;
                    }
                    break;
                }
                //Wait 500us before changing the pstate
                ssx_sleep(SSX_MICROSECONDS(500));

            }
        }
    }
    TRAC_INFO("Exit");
    return l_err;
}

//*************************************************************************
// Image Header
//*************************************************************************
IMAGE_HEADER (G_myEntryPoint,myEntryPoint,MYENTRYPOINT_ID,OCC_APLT_TEST);
