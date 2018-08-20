/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/gpe_membuf.h $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2018                        */
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
#if !defined(_GPE_MEMBUF_H)
#define _GPE_MEMBUF_H

#include "ipc_structs.h"
#include "membuf_structs.h"


// Which GPE controls the PBASLAVE
#define OCI_MASTER_ID_GPE1 1


#define INBAND_ACCESS_READ 1
#define INBAND_ACCESS_WRITE 2


// IPC interface
void gpe_membuf_scom(ipc_msg_t* i_cmd, void* i_arg);
void gpe_membuf_data(ipc_msg_t* i_cmd, void* i_arg);
void gpe_membuf_init(ipc_msg_t* i_cmd, void* i_arg);

// HCODE interfaces
/**
 * Reset the PBA slave controller
 * @param[in] pba_parms
 */
void pbaslvctl_reset(GpePbaParms* i_pba_parms);

/**
 * Configure the PBA Slave
 * @param[in] pba_parms
 */
uint64_t pbaslvctl_setup(GpePbaParms* i_pba_parms);


/**
 * Access the MEMBUF mmio over the PBA slave.
 * @param[in] Memory buffer configuration
 * @param[in] The membuf ordinal number
 * @param[in] The OCI mapped address
 * @param[io] The data to move
 * @param[in] The operation to perform
 */
int inband_access(MemBufConfiguration_t* i_config,
                   uint32_t i_instance,
                   uint32_t i_oci_addr,
                   uint64_t * io_data,
                   int      i_read_write);

/**
 * Populate the MemBufConfiguration object for Centaur
 * @param[out] 8 byte aligned pointer to the CentaurConfiguration object.
 * @return  [0 | return code]
 * @note  The CentaurConfiguration object is shared with the 405 so
 * it needs to be in non-cacheable sram.
 */
int gpe_centaur_configuration_create(MemBufConfiguration_t * o_config);

/**
 * Scom all of the membufs with the same SCOM address.
 * @param[in] The CentaurConfig object
 * @param[in] The SCOM address
 * @param[out] The array of data collected. Must be large enough to hold
 *             uint64_t data from each membuf.
 * @return [0 | return code]
 */
int membuf_get_scom_vector(MemBufConfiguration_t* i_config,
                            uint32_t i_scom_address,
                            uint64_t* o_data);

/**
 * Scom one or more membuf modules
 * @param[in] The MemBufConfiguration object
 * @param[in/out] The Scom Parms object
 * @return The return code is part of the MemBufScomParms object
 */
void gpe_inband_scom(MemBufConfiguration_t* i_config,
                     MemBufScomParms_t* i_parms);


/**
 * Collect the centaur thermal data
 * @param[in] The CentaurConfig object
 * @param[in/out] The Centaur data parm object
 * @return [0 | return code]
 */
int get_centaur_sensorcache(MemBufConfiguration_t* i_config,
                            MemBufGetMemDataParms_t* i_parms);

/**
 * Check for channel checkstop
 * @param[in] The ordinal centaur number
 * @return [0 | return code]
 */
int check_centaur_channel_chkstp(unsigned int i_centaur);

/**
 * Send SYNC to centaur to effectuate the thottle values
 * @param[in] the membuf configuration
 */
int centaur_throttle_sync(MemBufConfiguration_t* i_config);

/**
 * Create PBA slave configuration parameters.
 * @param[in] ptr tor param data area to be filled out.
 * @param[in] PBA slave to use.
 * @param[in] write ttype (@see occhw_pba_common.h)
 * @param[in] write tsize (@see occhw_pba_common.h)
 * @param[in] read_ttype  (@see occhw_pba_common.h)
 * @return [SUCCESS | return code]
 */
int gpe_pba_parms_create(GpePbaParms* parms,
                         int slave,
                         int write_ttype,
                         int write_tsize,
                         int read_ttype);

/**
 * Configure the PBABAR for inband access
 * @param[in] Configuration information
 */
int configure_pba_bar_for_inband_access(MemBufConfiguration_t * i_config);


extern uint32_t g_inband_access_state;

/**
 * Populate a CentaurConfiguration object for ocmb
 * @param[out] 8 byte aligned pointer to the CentaurConfiguration object.
 * @return  [0 | return code]
 * @note  The CentaurConfiguration object is shared with the 405 so
 * it needs to be in non-cacheable sram.
 */
int gpe_ocmb_configuration_create(MemBufConfiguration_t * o_config);

/**
 * Check for OCMBr sensor errors
 * @param[in] Configuration information
 * @param[in] The ordinal membuf number
 * @return [0 | return code]
 */
int check_and_reset_mmio_fir(MemBufConfiguration_t * i_config,unsigned int i_membuf);

/**
 * Send SYNC to ocmb to effectuate the thottle values
 * @param[in] the membuf configuration
 */
int ocmb_throttle_sync(MemBufConfiguration_t* i_config);

int get_ocmb_sensorcache(MemBufConfiguration_t* i_config,
                             MemBufGetMemDataParms_t* i_parms);


#endif
