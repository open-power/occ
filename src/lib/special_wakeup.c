/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/special_wakeup.c $                                    */
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
/// \file special_wakeup.c
/// \brief Container for special wakeup related procedures

#include "special_wakeup.h"

uint32_t G_special_wakeup_count[PGP_NCORES] = {0};

/// Enter or clear special wakeup for a core
///
/// \param set 1 = set, 0 = clear, all other values will cause an error.
///
/// \param cores = mask of cores to set/clear special wakeup.
///
/// \param[out] o_timeouts.  Mask of cores that timed out before special wakeup
/// complete was observed.
///
/// \retval 0 Success
///
/// \retval -SPWU_INVALID_ARGUMENT One of the arguments was invalid in some way
///
/// \retval others This API may also return non-0 codes from
/// getscom()/putscom()
///
/// If getscom/putscom rc = 0, the state of the global special_wakeup counts
/// may no longer be valid.
///

int
occ_special_wakeup(int set,
                   ChipConfigCores cores,
                   int timeout_ms,
                   ChipConfigCores *o_timeouts)

{

    pmc_core_deconfiguration_reg_t pcdr;
    pcbs_pmgp0_reg_t pmgp0;
    pcbs_pmspcwkupocc_reg_t ppr;
    ChipConfigCores core_list;
    ChipConfigCores awake_list = 0;
    ChipConfigCores success_list = 0;
    ChipConfigCores poll_list = 0;
    ChipConfigCores timeout_list = 0;
    int rc, poll_count, core;
    int time = 0;
    int bad_clear;

    // get pmc deconfig vector
    pcdr.value = in32(PMC_CORE_DECONFIGURATION_REG);

    core_list = cores;
    bad_clear = 0;
    if (! set) {
        for (core = 0; core < PGP_NCORES; core++, core_list <<=1) {
            if (core_list & (0x1 << (PGP_NCORES - 1))) {
                if (G_special_wakeup_count[core] == 0) {
                    bad_clear = 1;
                }
            }
        }
    }

    
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF( (set < 0) ||
                      (set > 1) ||
                      (bad_clear) ||
                      (pcdr.fields.core_chiplet_deconf_vector & cores),
                      SPWU_INVALID_ARGUMENT);
    }

    do {
        ppr.value = 0;
        if (set) {
            // If count is currently zero, set the bit and increment count.
            // Otherwise, just increment count
            core_list = cores;
            for (core = 0; core < PGP_NCORES; core++, core_list <<=1) {
                if (core_list & (0x1 << (PGP_NCORES - 1))) {
                    if (! G_special_wakeup_count[core]) {
                        ppr.fields.occ_special_wakeup = 1;
                        rc = putscom(CORE_CHIPLET_ADDRESS(PCBS_PMSPCWKUPOCC_REG, core), ppr.value);
                        poll_list |= (0x1 << (PGP_NCORES - 1 - core));  
                    }
                    if (rc) break;  // break for loop
                    ++G_special_wakeup_count[core];
                    success_list |= (0x1 << (PGP_NCORES - 1 - core)); 
                }
            }
            if (rc) break;
            
            // poll special_wkup_done bit.
            poll_count = 0;
            while ((poll_list != awake_list) && (time < timeout_ms)) {
                if (! poll_count) {
                    ssx_sleep(SSX_MICROSECONDS(2));
                    ++poll_count;
                } else {
                    ssx_sleep(SSX_MILLISECONDS(5));
                    time += 5;
                }
                core_list = poll_list & (~awake_list);
                for (core = 0; core < PGP_NCORES; core++, core_list <<=1) {
                    if (core_list & (0x1 << (PGP_NCORES - 1))) {
                        rc = getscom(CORE_CHIPLET_ADDRESS(PCBS_PMGP0_REG, core), &pmgp0.value);
                        if (rc) break;
                        if (pmgp0.fields.special_wkup_done) { 
                            awake_list |= (0x1 << (PGP_NCORES - 1 - core));
                        } else {
                            if (time >= timeout_ms) {
                                timeout_list |= (0x1<<(PGP_NCORES-1-core));
                            }
                        }
                    }
                }                   
            }       
        } else { // clear special wakeup
            core_list = cores;
            for (core = 0; core < PGP_NCORES; core++, core_list <<=1) {
                if (core_list & (0x1 << (PGP_NCORES - 1))) {
                    if (G_special_wakeup_count[core] == 1) {
                        ppr.fields.occ_special_wakeup = 0;
                        rc = putscom(CORE_CHIPLET_ADDRESS(PCBS_PMSPCWKUPOCC_REG, core), ppr.value);
                    }
                    if (rc) break;
                    --G_special_wakeup_count[core];
                    success_list |= (0x1 << (PGP_NCORES - 1 - core));     
                }
            }
            if (rc) break;
        }
    } while (0);

    // bad rc recovery (recovery of counts, etc?)

    *o_timeouts = timeout_list;
    return rc;

}
                         
            
