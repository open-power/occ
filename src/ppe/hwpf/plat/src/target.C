/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwpf/plat/src/target.C $                              */
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

#include <target.H>
#include <new>
#include <utility> // For move
// Global Vector containing ALL targets.  This structure is referenced by
// fapi2::getChildren to produce the resultant returned vector from that
// call.
std::vector<fapi2::plat_target_handle_t> G_vec_targets;

namespace fapi2
{

    #ifndef __noRC__
    ReturnCode current_err;
    #endif

    /// @brief Function to initialize the G_targets vector based on partial good
    ///      attributes ///  this will move to plat_target.H formally
    fapi2::ReturnCode plat_TargetsInit()
    {

        // This is workaround. Currently we do not have code to initialise
        // global objects. So initializing global objects against using local
        // initialized object
        std::vector<fapi2::plat_target_handle_t> targets1;
        G_vec_targets = std::move(targets1);
        std::vector<fapi2::plat_target_handle_t>::iterator tgt_iter;
        uint32_t l_beginning_offset;

        FAPI_DBG("Platform target initialization.  Target Count = %u", TARGET_COUNT);
        // Initialize all entries to NULL
        for (uint32_t i = 0; i < TARGET_COUNT; ++i)
        {
            G_vec_targets.push_back((fapi2::plat_target_handle_t)0x0);
            FAPI_DBG("Nulling G_vec_targets[%u] hi value=0x%08X",
                        i, (uint32_t)(G_vec_targets.at(i)>>32));
    //        FAPI_DBG("Nulling G_vec_targets[%u] lo value=0x%08X",
    //                    i, (uint32_t)(G_vec_targets.at(i)&0x00000000ffffffffull));
        }
         FAPI_DBG("Vector size: %u", G_vec_targets.size());

        // Chip Target is the first one
        FAPI_DBG("Chip Target info: CHIP_TARGET_OFFSET %u CHIP_TARGET_COUNT %u ",
                      CHIP_TARGET_OFFSET,CHIP_TARGET_COUNT);


        l_beginning_offset = CHIP_TARGET_OFFSET;
        FAPI_DBG("Chip beginning offset =%u", l_beginning_offset);
        for (uint32_t i = 0; i < CHIP_TARGET_COUNT; ++i)
        {
            fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> target_name((fapi2::plat_target_handle_t)i);
            G_vec_targets.at(l_beginning_offset+i) = revle64((fapi2::plat_target_handle_t)(target_name.get()));
    //         FAPI_DBG("Chip Target initialization: %u G_vec_targets[%u] value=0x%08X",
    //                     i,
    //                    (l_beginning_offset+i),
    //                    (uint32_t)(G_vec_targets.at(l_beginning_offset+i)>>32));
        }

        // Chip Level Pervasive Targets
        FAPI_DBG("Pervasive Target info: PERV_TARGET_OFFSET %u PERV_TARGET_COUNT %u",
                      PERV_TARGET_OFFSET, PERV_TARGET_COUNT);

        l_beginning_offset = PERV_TARGET_OFFSET;
        FAPI_DBG("Perv beginning offset =%u", l_beginning_offset);
        for (uint32_t i = 0; i < PERV_TARGET_COUNT; ++i)
        {
            fapi2::Target<fapi2::TARGET_TYPE_PERV> target_name((fapi2::plat_target_handle_t)i);
            FAPI_DBG("target_name hi word = 0x%08X", (uint32_t)(target_name.get()>>32));

            G_vec_targets.at(l_beginning_offset+i) = revle64((fapi2::plat_target_handle_t)(target_name.get()));
    //         FAPI_DBG("Pervasive Target initialization: %u G_vec_targets[%u] value=0x%08X",
    //                     i,
    //                    (l_beginning_offset+i),
    //                    (uint32_t)(G_vec_targets.at(l_beginning_offset+i)>>32));
        }

        // Cache (EQ) Targets
        FAPI_DBG("EQ Target info: EQ_TARGET_OFFSET %u EQ_TARGET_COUNT %u",
                      EQ_TARGET_OFFSET,  EQ_TARGET_COUNT);
        l_beginning_offset = EQ_TARGET_OFFSET;
        FAPI_DBG("EQ beginning offset =%u", l_beginning_offset);
        for (uint32_t i = 0; i < EQ_TARGET_COUNT; ++i)
        {
            fapi2::Target<fapi2::TARGET_TYPE_EQ> target_name((fapi2::plat_target_handle_t)i);
            FAPI_DBG("target_name hi word = 0x%08X", (uint32_t)(target_name.get()>>32));
            G_vec_targets.at(l_beginning_offset+i) = revle64((fapi2::plat_target_handle_t)(target_name.get()));
    //         FAPI_DBG("EQ Target initialization: %u G_vec_targets[%u] value=%16llX",
    //                      i,
    //                    (l_beginning_offset+i),
    //                    revle64((uint64_t)G_vec_targets[l_beginning_offset+i]));
        }

        // Core (EC) Targets
        FAPI_DBG("Core Target info: CORE_TARGET_OFFSET %u CORE_TARGET_COUNT %u",
                      CORE_TARGET_OFFSET, CORE_TARGET_COUNT);

        l_beginning_offset = CORE_TARGET_OFFSET;
        FAPI_DBG("Core beginning offset =%u", l_beginning_offset);
        for (uint32_t i = 0; i < CORE_TARGET_COUNT; ++i)
        {
            fapi2::Target<fapi2::TARGET_TYPE_CORE> target_name((fapi2::plat_target_handle_t)i);
            FAPI_DBG("target_name hi word = 0x%08X", (uint32_t)(target_name.get()>>32));
            G_vec_targets.at(l_beginning_offset+i) = revle64((fapi2::plat_target_handle_t)(target_name.get()));
    //         FAPI_DBG("Core Target initialization: %u G_vec_targets[%u] value=0x%08X",
    //                     i,
    //                    (l_beginning_offset+i),
    //                    (uint32_t)(G_vec_targets.at(l_beginning_offset+i)>>32));
        }

        // Memroy Controller Synchronous (MCS) Targets
        FAPI_DBG("MCS Target info: MCS_TARGET_OFFSET %u MCS_TARGET_COUNT %u",
                      MCS_TARGET_OFFSET, MCS_TARGET_COUNT);

        l_beginning_offset = MCS_TARGET_OFFSET;
        FAPI_DBG("MCS beginning offset =%u", l_beginning_offset);
        for (uint32_t i = 0; i < MCS_TARGET_COUNT; ++i)
        {
            fapi2::Target<fapi2::TARGET_TYPE_MCS> target_name((fapi2::plat_target_handle_t)i);
            FAPI_DBG("target_name hi word = 0x%08X", (uint32_t)(target_name.get()>>32));
            G_vec_targets.at(l_beginning_offset+i) = revle64((fapi2::plat_target_handle_t)(target_name.get()));
    //         FAPI_DBG("MCS Target initialization: %u G_vec_targets[%u] value=0x%08X",
    //                     i,
    //                    (l_beginning_offset+i),
    //                    (uint32_t)(G_vec_targets.at(l_beginning_offset+i)>>32));
        }

        // Trace all entries
        uint32_t c = 0;
        for (tgt_iter = G_vec_targets.begin(); tgt_iter != G_vec_targets.end(); ++tgt_iter)
        {
            FAPI_DBG("Trace hi word G_vec_targets[%u] value=%08X",
                       c, (uint32_t)((*tgt_iter)>>32));
            ++c;
        }

        return fapi2::current_err;
    }

    /// @brief Function to initialize the G_targets vector based on partial good
    ///        attributes
    fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> plat_getChipTarget()
    {

        // Get the chip specific target
        return ((fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>)G_vec_targets.at(0));
    }   

};

