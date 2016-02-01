// $Id: special_wakeup.c,v 1.5 2016/01/18 11:34:20 stillgs Exp $
// $Source: /archive/shadow/ekb/.cvsroot/eclipz/chips/p8/working/procedures/lib/special_wakeup.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file special_wakeup.c
/// \brief Container for special wakeup related procedures

#include "special_wakeup.h"

uint32_t G_special_wakeup_count[PGP_NCORES] = {0};

#define TRUE  1
#define FALSE 0

/// Enter or clear special wakeup for a core
///
/// \param set 1 = set, 0 = clear, all other values will cause an error.
///
/// \param cores = mask of cores to set/clear special wakeup.
///
/// \param[out] o_timeouts.  Mask of cores that timed out before special wakeup
/// complete was observed.
///
/// \param[in] b_noclear_check.  0 = Perform the clear check; 1 = do not perform
/// the clear check
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

// SW329314 - put all the real work in an internal function to all for a new
// interface to be added.

int
__occ_special_wakeup(int set,
                   ChipConfigCores cores,
                   int timeout_ms,
                   ChipConfigCores *o_timeouts,
                   int b_noclear_check)

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

    bad_clear = 0;
    if (!b_noclear_check) { 
        core_list = cores; 
        if (! set) {
            for (core = 0; core < PGP_NCORES; core++, core_list <<=1) {
                if (core_list & (0x1 << (PGP_NCORES - 1))) {
                    if (G_special_wakeup_count[core] == 0) {
                        bad_clear = 1;
                    }
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
                         

/// Legacy interface 
/// \brief Enter or clear special wakeup for a core
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
    int rc;    
    rc = __occ_special_wakeup(set,
                              cores,
                              timeout_ms,
                              o_timeouts,
                              FALSE);
    return rc;
}
 
 
/// No Clear Chaeck interface 
/// \brief Enter or clear special wakeup for a core without checking for clearing an
/// already cleared core
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
occ_special_wakeup_noclearcheck(int set,
                   ChipConfigCores cores,
                   int timeout_ms,
                   ChipConfigCores *o_timeouts)
{
    int rc;   
    rc = __occ_special_wakeup(set,
                              cores,
                              timeout_ms,
                              o_timeouts,
                              TRUE);
    return rc;
}           
