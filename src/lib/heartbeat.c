/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/heartbeat.c $                                         */
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
/// \file heartbeat.c
/// \brief PgP PMC/PCBS heartbeat configuration procedures

#include "ssx.h"
#include "heartbeat.h"

/// Configure/Enable/Disable the pmc heartbeat register.
///
/// \param enable 1 = enable, 0 = disable, all other values will cause error.
///
/// \param req_time_us heartbeat interval time request (in microseconds).
/// If the pmc does not detect a heartbeat within this time the pmc will
/// set the corresponding fir bit and enter safe mode.  This interval
/// is the requested value.  The return value will be the actual setting.
/// The procedure well attempt to get as close to the requested time as possible
/// without choosing a setting lower then requested.
/// Legal values: 1-4194240 (us).  Ignored if force = 1 or enable = 0
///
/// \param force 1 = force safe mode (debug), 0 = do not force, all other values
/// will cause an error.  enable = 0 and force = 1 will return an error
///
/// \param[out] o_time_us Actual configured time rounded down to the nearest us.
/// This will be as close as the procedure could get to the requested time given
/// the frequency and pulse time settings.  Returns 0 if hearbeat was disabled or
/// if safe mode was forced.
///
/// \retval 0 Success
///
/// \retval -HB_INVALID_ARGUMENT_PMC One of the arguments was invalid in 
/// some way

int
pmc_hb_config(unsigned int enable,
              unsigned int req_time_us,
              unsigned int force,
              unsigned int *o_time_us
              )

{
    pmc_parameter_reg0_t ppr0;
    pmc_occ_heartbeat_reg_t pohr;
    tpc_hpr2_t l_hpr2;
    uint64_t divider, pulses, total_pulses, hp_freq;
    int rc = 0;

    // @dyd SW238882 fix
    // remove req_time_us overflow check since the upper boundary of 
    // the req_time_us doesnt depand on certain static value but based on 
    // the value set in hang_pulse_2_reg at runtime.
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((enable > 1) || 
                     (force > 1) ||
                     ((req_time_us < 1) && enable && (! force)) ||
                     ((force == 1 && enable == 0)),
                     HB_INVALID_ARGUMENT_PMC);
    }

    do {

        // in case firmware does not call ocb_timer_setup 
        // before calling this procedure to setup g_ocb_timer_divider
        rc = getscom(TPC_HPR2, &l_hpr2.value);
        if(rc) break;
        g_ocb_timer_divider = 1 << l_hpr2.fields.hang_pulse_reg;

        // calculation based on pmc_occ_heartbeat_reg defination
        hp_freq = (__ssx_timebase_frequency_mhz/g_ocb_timer_divider);
        if(hp_freq < 1)
           hp_freq = 1;
        total_pulses = (req_time_us * hp_freq);
        
        // this may be an overkill for safety but no one should notice
        if ((req_time_us*__ssx_timebase_frequency_mhz) % g_ocb_timer_divider) {
            total_pulses++;
        }     

        divider = 0;
        // determine values for predivider and number of pulses.
        if (force || (! enable)) {  // predivider a don't care in this case
            pulses = 0; 
            *o_time_us = 0;
        } else {
            // can count up to 2^16 pulses with no pre-divide, first determine
            // minimum pre-divider needed
            do {
                divider++;
            } while ((((divider << 16) - 1) / total_pulses) < 1);

            // @dyd SW238882 fix
            // underflow case
            // since pmc heartbeat counter counts with nest_nclk/4 
            // instead of hang pulse when hangpulse_predivider==0,
            // this procedure doesnt allow predivider to be set to
            // zero as it is a special case which doesnt work with
            // occ heartbeat time value calculated by this procedure.
            // Given hangpulse_predivider = divider - 1, 
            // set divider to 2 if it is 1, zero not possible.  
            if (divider < 2) { 
                divider = 2;
                //rc = HB_UNDERFLOW_DIVIDER_PMC;
                //break;
            }
            // overflow case
            // since hangpulse_predivider field is only 6 bit long, 
            // check the overflow first, set to maximum if larger.
            if (divider > 64) {
                divider = 64;            
                //rc = HB_OVERFLOW_DIVIDER_PMC;
                //break;
            }
            
            // divider is determined, now setup number of pulses
            pulses = total_pulses / divider;
            if (total_pulses % divider) {
                pulses++;
            }

            // @dyd SW238882 fix
            // there is no underflow case for pulses, because pulses=0 as 
            // intended immediate timeout is allowed, plus no mathematical 
            // substraction from pulses is done; however there is an overflow 
            // case: the value of pulses doesnt fit into 16 bits HW field.
            // Here we set pulses to the maximum value that HW allows, 
            // and use the o_time_us to feedback the caller this is done. 
            if (pulses > 0xFFFF) {
                pulses = 0xFFFF;
                //rc = HB_OVERFLOW_PULSES_PMC;
                // break;
            }

            // calculating real timeout duration 
            // that this procedure is going to set
            *o_time_us = (divider*pulses)/hp_freq;

            // @dyd SW238882 fix
            // in force == 0 && enable == 1 case
            // disable heartbeat first before reset hang pulse predivider 
            // and new heartbeat time value to prevent immediate timeout.
            // if force == 1 then it is intended to be immediate timeout anyway
            // if enable == 0 then it is going to set this bit to zero anyway
            pohr.value = 0;
            pohr.fields.pmc_occ_heartbeat_en = 0;
            if (cfam_id() == CFAM_CHIP_ID_MURANO_10) {
                out32(PMC_OCC_HEARTBEAT_REG, pohr.value);
            }
            out32(PMC_OCC_HEARTBEAT_REG, pohr.value);
        }

        // Note through experiments, the divider=predivider+1 isnt always 
        // in effect in hardware due to missing last pulse in tight timing, 
        // some setup will end up with just divider=predivider; therefore,
        // in order to not result unexpected heartbeat timeout, always
        // set divider to predivider to be safe. 
        if (enable && (! force)) {
            ppr0.value = in32(PMC_PARAMETER_REG0);
            ppr0.fields.hangpulse_predivider = divider;
            out32(PMC_PARAMETER_REG0, ppr0.value);
        }
   
        pohr.value = 0;
        pohr.fields.pmc_occ_heartbeat_en = enable;
        pohr.fields.pmc_occ_heartbeat_time = pulses;
        // Due to Issue HW219480, the heartbeat register needs to be written
        // Twice in order for the heartbeat count value to take correctly.
        // Technically it would not be harmful to just double-write in
        // every case, but this is currently written to only double-write
        // if a Murano dd1.0 part is detected
        if (cfam_id() == CFAM_CHIP_ID_MURANO_10) {
            out32(PMC_OCC_HEARTBEAT_REG, pohr.value);
        }
        out32(PMC_OCC_HEARTBEAT_REG, pohr.value);

    }while(0);
    return rc;

}
    
   

/// Configure/Enable/Disable the pcbs heartbeat registers.
///
/// \param enable 1 = enable, 0 = disable, all other values will cause error.
///
/// \param cores Use this mask to select which cores to update.  This routine
/// will cross reference the current pmc deconfig vector and only update
/// those cores that are both selected here and configured.
///
/// \param hb_reg 32-bit unsigned address of register to setup as the
/// PCBS heartbeat register.  This must be a PCBS address.
/// Ignored unless enable = 1 
///
/// \param req_time_us heartbeat interval time request (in microseconds).
/// If the pcbs does not detect a heartbeat within this time the pcbs will
/// set the corresponding fir bit and enter safe mode.  This interval
/// is the requested value.  The return value will be the actual setting and
/// the procedure will attempt go get as close to possible to this without
/// choosing a setting lower then requested.
/// Legal values: 1 - 16320 (ignored unless enable = 1)
///
/// \param force 1 = force safe mode (debug), 0 = do not force, all other values
/// will cause an error.  In PCBS, the force safe mode is not related to
/// the heartbeat so forcing safe mode while also enabling the heartbeat
/// is allowed.
///
/// \param[out] o_time_us Actual configured time in us.  This represents the
/// actual setting rounded down to the nearest us.  0 if heartbeat was disabled.
///
/// \retval 0 Success

/// \retval -HB_INVALID_ARGUMENT_PCBS One of the arguments was invalid in 
/// some way
///
/// \retval others This API may also return non-0 codes from
/// getscom()/putscom()


int
pcbs_hb_config(unsigned int enable,
               ChipConfigCores cores,
               uint32_t hb_reg,
               unsigned int req_time_us,
               unsigned int force,
               unsigned int *o_time_us)
{
    pcbs_occ_heartbeat_reg_t pohr;
    pcbs_pmgp1_reg_t pp1r;
    pmc_core_deconfiguration_reg_t pcdr;
    uint32_t reg_offset;
    uint32_t pp1r_addr;
    uint64_t pp1r_data;
    ChipConfigCores core_list;
    ChipConfigCores deconfig;
    int core;
    int rc = 0;
    unsigned int pulses;
      
    reg_offset = hb_reg - PCBS_PIB_BASE;
    
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((enable > 1) ||
                     ((reg_offset > 0xFF) && enable) ||
                     ((req_time_us < 64) && enable) ||
                     ((req_time_us > 16320) && enable) ||
                     (force > 1),
                     HB_INVALID_ARGUMENT_PCBS);
    }


    do {

        // calculation based on pcbs_occ_heartbeat_reg defination
        pulses = req_time_us/64;
        if (req_time_us % 64) {
            pulses++;
        }
   
        // @dyd SW238882 fix
        // overflow handling: HW only allows 8 bits in the field.
        // set pulses to maximum allowed value in HW if it overflows,
        // and o_time_us will feedback to caller this is done.
        if (pulses > 0xFF) {
            pulses = 0xFF;
            //rc = HB_PULSES_OVERFLOW_PCBS;
            //break;
        }
        // underflow case, pulses cannot be zero due to undefined HW behavior
        if (pulses < 1) {
            pulses = 1;
            //rc = HB_PULSES_UNDERFLOW_PCBS;
            //break;
        }

        pp1r.value = 0;
        pp1r.fields.force_safe_mode = 1;
        if (force) {
            pp1r_addr = PCBS_PMGP1_REG_OR;
            pp1r_data = pp1r.value;
        } else {
            pp1r_addr = PCBS_PMGP1_REG_AND;
            pp1r_data = ~(pp1r.value);
        }
    
        pcdr.value = in32(PMC_CORE_DECONFIGURATION_REG);
        deconfig = pcdr.fields.core_chiplet_deconf_vector;
    
        pohr.value = 0;
        pohr.fields.occ_heartbeat_enable = enable;
        pohr.fields.occ_heartbeat_time = pulses;
        pohr.fields.occ_heartbeat_reg_addr_offset = reg_offset;
    
        if (enable) {
            *o_time_us = pulses * 64;
        } else {
            *o_time_us = 0;
        }
    
        do {
            core_list = cores & (~deconfig);
            for (core = 0; core < PGP_NCORES; core++, core_list <<= 1) {
                if (core_list & 0x8000) {
                    // read modify write to preserve psafe
                    rc = getscom(CORE_CHIPLET_ADDRESS(PCBS_OCC_HEARTBEAT_REG,
                                                          core), &pohr.value);
                    if (rc) break;
                    pohr.fields.occ_heartbeat_enable = enable;
                    pohr.fields.occ_heartbeat_time = pulses;
                    pohr.fields.occ_heartbeat_reg_addr_offset = reg_offset;
                    rc = putscom(CORE_CHIPLET_ADDRESS(PCBS_OCC_HEARTBEAT_REG,
                                                          core), pohr.value);
                    if (rc) break;
                    rc = putscom(CORE_CHIPLET_ADDRESS(pp1r_addr, core),
                                     pp1r_data);
                    if (rc) break;
                }
            }
        } while (0); 
    
   }while(0); 
   return rc;
}
