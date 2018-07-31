#if !defined(_GPE_CENTAUR_H)
#define _GPE_CENTAUR_H

#include "ipc_structs.h"
#include "centaur_structs.h"

// IPC interface
void gpe_centaur_scom(ipc_msg_t* i_cmd, void* i_arg);
void gpe_centaur_data(ipc_msg_t* i_cmd, void* i_arg);
void gpe_centaur_init(ipc_msg_t* i_cmd, void* i_arg);

// HCODE interface
/**
 * Populate a CentaurConfiguration object
 * @param[out] 8 byte aligned pointer to the CentaurConfiguration object.
 * @return  [0 | return code]
 * @note  The CentaurConfiguration object is shared with the 405 so
 * it needs to be in non-cacheable sram.
 */
int gpe_centaur_configuration_create(CentaurConfiguration_t * o_config);

/**
 * Scom all of the centaurs with the same SCOM address.
 * @param[in] The CentaurConfig object
 * @param[in] The SCOM address
 * @param[out] The array of data collected. Must be large enough to hold
 *             uint64_t data from each centaur.
 * @return [0 | return code]
 */
int centaur_get_scom_vector(CentaurConfiguration_t* i_config,
                            uint32_t i_scom_address,
                            uint64_t* o_data);

/**
 * Scom one or more centaurs
 * @param[in] The CentaurConfig object
 * @param[in/out] The Centaur Scom Parms object
 * @return The return code is part of the Centaur Scom Parms object
 */
void gpe_scom_centaur(CentaurConfiguration_t* i_config,
                      CentaurScomParms_t* i_parms);


/**
 * Collect the centaur thermal data
 * @param[in] The CentaurConfig object
 * @param[in/out] The Centaur data parm object
 * @return [0 | return code]
 */
int centaur_get_mem_data(CentaurConfiguration_t* i_config,
                         CentaurGetMemDataParms_t* i_parms);

/**
 * Check for channel checkstop
 * @param[in] The ordinal centaur number
 * @return [0 | return code]
 */
int check_channel_chkstp(unsigned int i_centaur);

extern uint32_t g_centaur_access_state;
#endif
