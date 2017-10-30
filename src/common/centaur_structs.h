#if !defined(_CENTAUR_STRUCTS_H)
#define _CENTAUR_STRUCTS_H

#include "stdint.h"
#include "gpe_export.h"
#include "centaur_configuration.h"

// IPC message payload for call to IPC_ST_CENTAUR_INIT_FUNCID
typedef struct
{
    GpeErrorStruct error;
    CentaurConfiguration_t * centaurConfiguration;

} CentaurConfigParms_t;

// IPC message payload for call to IPC_ST_CENTAUR_SCOM_FUNCID
typedef struct
{
    GpeErrorStruct error;

    /**
     * Input: The SCOM list
     * This is a pointer to an array of scomList_t objects
     * describing the sequence of commands to execute.
     */
    scomList_t* scomList;

    /**
     * Input: The number of entries in the scomList.
     * @note It is considered an error if \a entries is 0, under the
     * assumption that the caller must have neglected to initialize the
     * structure.
     */
    int     entries;

} CentaurScomParms_t;

// IPC message payload for call to IPC_ST_CENTAUR_DATA_FUNCID
typedef struct
{
    GpeErrorStruct error;

    /**
     * The index (0 .. PGP_NCENTAUR - 1) of the Centaur whose sensor cache
     * data to collect, or -1 to bypass collection.
     */
    int collect;

    /**
     * The index (0 .. PGP_NCENTAUR - 1) of the Centaur to "poke" to cause it
     * to begin collecting the next round of data into its sensor cache, or
     * -1 to bypass updating
     */
    int update;

    /**
     * Pointer to data collected.  Needs to be set if collect != -1
     * otherwise it's not used.
     */
    uint64_t * data;
} CentaurGetMemDataParms_t;


#endif
