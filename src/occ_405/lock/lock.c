/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/lock/lock.c $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2019                        */
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

// Debug trace
//#define LOCK_DEBUG
#ifdef LOCK_DEBUG
  #define LOCK_DBG(frmt,args...)  \
          TRAC_INFO(frmt,##args)
#else
  #define LOCK_DBG(frmt,args...)
#endif

#include <ssx.h>
#include <occhw_async.h>
#include <gpe_export.h>

#include <trac_interface.h>
#include <trac.h>
#include <occ_common.h>
#include <comp_ids.h>
#include <occ_service_codes.h>
#include "lock.h"
#include "common.h"
#include "state.h"
#include "i2c.h"
#include <amec_sys.h>

extern bool G_mem_monitoring_allowed;

typedef enum
{
    LOCK_RELEASE = 0x00,
    LOCK_ACQUIRE = 0x01
} lockOperation_e;


#ifdef DEBUG_LOCK_TESTING
// DEBUG: Simulate I2C lock request from the host
void host_i2c_lock_request()
{
    ocb_occflg_t occ_flags = {0};
    TRAC_INFO("host_i2c_lock_request called (tick %d)", CURRENT_TICK);
    occ_flags.fields.i2c_engine3_lock_host = 1;
    TRAC_INFO("host_i2c_lock_request - writing %04X to _OR(0x%08X)", occ_flags.value, OCB_OCCFLG_OR);
    out32(OCB_OCCFLG_OR, occ_flags.value);

    occ_flags.value = in32(OCB_OCCFLG);
    //TRAC_INFO("host_i2c_lock_request - 0x%08X returned value=0x%04X", OCB_OCCFLG, occ_flags.value);
    if (occ_flags.fields.i2c_engine3_lock_host != 1)
    {
        TRAC_INFO("ERROR: host_i2c_lock_request - host not locked! (0x%08X value=0x%04X)", OCB_OCCFLG, occ_flags.value);
        occ_flags.fields.i2c_engine3_lock_host = 1;
        occ_flags.fields.i2c_engine3_lock_occ = 1;
        out32(OCB_OCCFLG, occ_flags.value);
        occ_flags.value = in32(OCB_OCCFLG);
        TRAC_INFO("host_i2c_lock_request - write+read 0x%08X returned value=0x%04X", OCB_OCCFLG, occ_flags.value);
    }
}

// DEBUG: Simulate I2C lock release from host
void host_i2c_lock_release()
{
    TRAC_INFO("host_i2c_lock_release called (tick %d)", CURRENT_TICK);

    ocb_occmisc_t occmiscreg = {0};
    ocb_occflg_t occ_flags = {0};
    // clear external interrupt (so OCC can notify host when lock released)
    occmiscreg.fields.core_ext_intr = 1;
    out32(OCB_OCCMISC_CLR, occmiscreg.value);

    // Clear the host request
    occ_flags.fields.i2c_engine3_lock_host = 1;
    TRAC_INFO("host_i2c_lock_release - writing %04X to _CLR(0x%08X)", occ_flags.value, OCB_OCCFLG_CLR);
    out32(OCB_OCCFLG_CLR, occ_flags.value);

    occ_flags.value = in32(OCB_OCCFLG);
    //TRAC_INFO("host_i2c_lock_release - 0x%08X returned value=0x%04X", OCB_OCCFLG, occ_flags.value);
    if (occ_flags.fields.i2c_engine3_lock_host != 0)
    {
        TRAC_INFO("ERROR: host_i2c_lock_release - host not released! (0x%08X value=0x%04X)", OCB_OCCFLG, occ_flags.value);
        occ_flags.fields.i2c_engine3_lock_host = 0;
        occ_flags.fields.i2c_engine3_lock_occ = 1;
        out32(OCB_OCCFLG, occ_flags.value);
        occ_flags.value = in32(OCB_OCCFLG);
        TRAC_INFO("host_i2c_lock_release - write+read 0x%08X returned value=0x%04X", OCB_OCCFLG, occ_flags.value);
    }
}
#endif


// Update I2C log information for specified engine
// i_op values:
//      LOC_ACQUIRE = OCC should take ownership of lock
//      LOC_RELEASE = OCC should release ownership of lock and notify host
void update_i2c_lock(const lockOperation_e i_op, const uint8_t i_engine)
{
    ocb_occflg_t occ_flags = {0};

#ifdef DEBUG_LOCK_TESTING
    ocb_occflg_t flag;
    flag.value = in32(OCB_OCCFLG);
    if (LOCK_RELEASE == i_op)
    {
        LOCK_DBG("update_i2c_lock: I2C engine %d RELEASE - host=%d, occ=%d, tick=%d",
                 i_engine, flag.fields.i2c_engine3_lock_host, flag.fields.i2c_engine3_lock_occ, CURRENT_TICK);
    }
    else
    {
        LOCK_DBG("update_i2c_lock: I2C engine %d LOCK    - host=%d, occ=%d, tick=%d",
                 i_engine, flag.fields.i2c_engine3_lock_host, flag.fields.i2c_engine3_lock_occ, CURRENT_TICK);
    }
#endif

    if (PIB_I2C_ENGINE_E == i_engine)
    {
        occ_flags.fields.i2c_engine3_lock_occ = 1;
    }
    else if (PIB_I2C_ENGINE_D == i_engine)
    {
        occ_flags.fields.i2c_engine2_lock_occ = 1;
    }
    else if (PIB_I2C_ENGINE_C == i_engine)
    {
        occ_flags.fields.i2c_engine1_lock_occ = 1;
    }

    if (LOCK_RELEASE == i_op)
    {
        out32(OCB_OCCFLG_CLR, occ_flags.value);

        // OCC had the lock and host wants it, so send interrupt to host
        notify_host(INTR_REASON_I2C_OWNERSHIP_CHANGE);

        TRAC_IMP("update_i2c_lock: OCC has released lock for I2C engine %d", i_engine);
    }
    else // LOCK_ACQUIRE
    {
        out32(OCB_OCCFLG_OR, occ_flags.value);

        TRAC_IMP("update_i2c_lock: OCC has acquired lock for I2C engine %d", i_engine);
    }

} // end update_i2c_lock()


// Release the OCC lock indefinitely
// This should be called when OCC goes into safe mode or will be reset
// to allow the host to use the specified I2C engines.
// If no engine is specified, locks for all I2C engines will be released
void occ_i2c_lock_release(const uint8_t i_engine)
{
    TRAC_INFO("occ_i2c_lock_release(engine %d) called", i_engine);

    if ((PIB_I2C_ENGINE_ALL == i_engine) ||
        (PIB_I2C_ENGINE_E == i_engine) || (PIB_I2C_ENGINE_D == i_engine) || (PIB_I2C_ENGINE_C == i_engine))
    {
        if ((PIB_I2C_ENGINE_E == i_engine) || (PIB_I2C_ENGINE_ALL == i_engine))
        {
            update_i2c_lock(LOCK_RELEASE, PIB_I2C_ENGINE_E);
        }
        if ((PIB_I2C_ENGINE_D == i_engine) || (PIB_I2C_ENGINE_ALL == i_engine))
        {
            update_i2c_lock(LOCK_RELEASE, PIB_I2C_ENGINE_D);
        }
        if ((PIB_I2C_ENGINE_C == i_engine) || (PIB_I2C_ENGINE_ALL == i_engine))
        {
            update_i2c_lock(LOCK_RELEASE, PIB_I2C_ENGINE_C);
        }
    }
    else
    {
        INTR_TRAC_ERR("occ_i2c_lock_release: Invalid engine specified: 0x%02X", i_engine);
    }

} // end occ_i2c_lock_release()


// Determine if the OCC currently owns the lock
// Returns true if OCC owns the lock, else false
bool occ_owns_i2c_lock(const ocb_occflg_t i_flags, const uint8_t i_engine)
{
    bool ownsLock = false;
    if (PIB_I2C_ENGINE_E == i_engine)
    {
        ownsLock = i_flags.fields.i2c_engine3_lock_occ;
    }
    else if (PIB_I2C_ENGINE_D == i_engine)
    {
        ownsLock = i_flags.fields.i2c_engine2_lock_occ;
    }
    else if (PIB_I2C_ENGINE_C == i_engine)
    {
        ownsLock = i_flags.fields.i2c_engine1_lock_occ;
    }
    return ownsLock;
}


// Determine if the Host wants the i2c lock
// Returns true if Host wants the lock, else false
bool host_wants_i2c_lock(const ocb_occflg_t i_flags, const uint8_t i_engine)
{
    bool wantsLock = false;
    if (PIB_I2C_ENGINE_E == i_engine)
    {
        wantsLock = i_flags.fields.i2c_engine3_lock_host;
    }
    else if (PIB_I2C_ENGINE_D == i_engine)
    {
        wantsLock = i_flags.fields.i2c_engine2_lock_host;
    }
    else if (PIB_I2C_ENGINE_C == i_engine)
    {
        wantsLock = i_flags.fields.i2c_engine1_lock_host;
    }
    return wantsLock;
}


// Check and update lock ownership for the specified i2c engine.
// Returns true if OCC owns the lock, or false if host owns lock
//
// If host has requesed the i2c lock, it will be released and an external interrupt
// will be generated/queued and function will return false.
// If the host has not released the lock, function will return false.
// If the host cleared its lock bit, OCC will take back ownership and return true.
//
bool check_and_update_i2c_lock(const uint8_t i_engine)
{
    bool occ_owns_lock = true;

    if ((PIB_I2C_ENGINE_E == i_engine) ||
        (PIB_I2C_ENGINE_D == i_engine) ||
        (PIB_I2C_ENGINE_C == i_engine))
    {
        bool needRetry = false;
        do
        {
            ocb_occflg_t original_occflags;
            original_occflags.value = in32(OCB_OCCFLG);

            LOCK_DBG("check_and_update_i2c_lock: I2C engine %d - host=%d, occ=%d (tick=%d)",
                     i_engine, original_occflags.fields.i2c_engine3_lock_host, original_occflags.fields.i2c_engine3_lock_occ, CURRENT_TICK);
            if (occ_owns_i2c_lock(original_occflags, i_engine))
            {
                if (host_wants_i2c_lock(original_occflags, i_engine))
                {
                    // Host requested lock, clear the OCC lock and notify host
                    update_i2c_lock(LOCK_RELEASE, i_engine);
                    occ_owns_lock = false;
                }
                // else OCC already owns the lock
            }
            else
            {
                // OCC does not own the lock
                occ_owns_lock = false;
                if (false == host_wants_i2c_lock(original_occflags, i_engine))
                {
                    // Host is not requesting the lock, acquire lock for OCC
                    update_i2c_lock(LOCK_ACQUIRE, i_engine);
                    occ_owns_lock = true;
                }
                // else Host still holds the lock
            }

            if ((occ_owns_lock) &&
                (original_occflags.fields.i2c_engine1_lock_host == 0) &&
                (original_occflags.fields.i2c_engine1_lock_occ == 0))
            {
                // If neither lock bit is set, we must read back the register to make
                // sure the host did not set at same time (lock conflict)
                ocb_occflg_t verify_occflags;
                verify_occflags.value = in32(OCB_OCCFLG);
                if (host_wants_i2c_lock(verify_occflags, i_engine))
                {
                    // Host wrote their lock bit at same time, clear OCC lock and notify host
                    update_i2c_lock(LOCK_RELEASE, i_engine);
                    occ_owns_lock = false;
                }
                else
                {
                    if (false == occ_owns_i2c_lock(verify_occflags, i_engine))
                    {
                        // ERROR - OCC OWNERSHIP BIT DID NOT GET SET
                        INTR_TRAC_ERR("check_and_update_i2c_lock: I2C lock bit did not get set (OCCFLAGS reg: 0x%08X)",
                                      verify_occflags.value);

                        if (needRetry)
                        {
                            // After one retry, log error and goto safe
                            /*
                             * @errortype
                             * @moduleid    I2C_LOCK_UPDATE
                             * @reasoncode  OCI_WRITE_FAILURE
                             * @userdata1   I2C engine number
                             * @userdata2   OCC Flags register
                             * @devdesc     OCI write failure setting I2C ownership bit
                             */
                            errlHndl_t err = createErrl(I2C_LOCK_UPDATE,
                                                        OCI_WRITE_FAILURE,
                                                        OCC_NO_EXTENDED_RC,
                                                        ERRL_SEV_PREDICTIVE,
                                                        NULL,
                                                        DEFAULT_TRACE_SIZE,
                                                        i_engine,
                                                        verify_occflags.value);

                            //Callout firmware
                            addCalloutToErrl(err,
                                             ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                             ERRL_COMPONENT_ID_FIRMWARE,
                                             ERRL_CALLOUT_PRIORITY_MED);

                            //Callout processor
                            addCalloutToErrl(err,
                                             ERRL_CALLOUT_TYPE_HUID,
                                             G_sysConfigData.proc_huid,
                                             ERRL_CALLOUT_PRIORITY_LOW);

                            REQUEST_RESET(err);
                            occ_owns_lock = false;
                            break;
                        }
                        needRetry = true;
                    }
                    // else verify succeeded (OCC owns lock)
                }
            }
        } while (needRetry);
    }
    else
    {
        // Invalid engine
        INTR_TRAC_ERR("check_and_update_i2c_lock: Invalid engine specified: 0x%02X", i_engine);
    }

    return occ_owns_lock;

} // end check_and_update_i2c_lock()
