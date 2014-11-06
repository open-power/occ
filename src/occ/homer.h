/******************************************************************************
 * @file homer.h
 * @brief homer header file
 *****************************************************************************/

/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section homer.h HOMER.H
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *           905504     sbroyles  11/06/13    Created
 *   @gs041  942203     gjsilva   10/17/2014  Support for HTMGT/BMC interface
 *
 *  @endverbatim
 *
 *****************************************************************************/

#ifndef _homer_h
#define _homer_h

//*************************************************************************
// Includes
//*************************************************************************

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

/*
 * Offset into the HOMER of the host data section and the size
 */
#define HOMER_HD_OFFSET       0x00100000
#define HOMER_HD_SZ           (128 * 1024)
/*
 * Version of HOMER host data currently supported
 */
#define HOMER_HD_VERSION_SUPPORT    2

/*
 * ID of host data variables
 */
typedef enum homer_read_var
{
    HOMER_VERSION,                  // 1
    HOMER_NEST_FREQ,                // 2
    HOMER_INT_TYPE,                 // 3
    HOMER_LAST_VAR
} homer_read_var_t;

/*
 * HOMER methods return codes
 */
typedef enum homer_rc
{
    HOMER_SUCCESS,                  // 1
    HOMER_UNSUPPORTED_HD_VERSION,   // 2
    HOMER_BAD_PARM,                 // 3
    HOMER_UNKNOWN_ID,               // 4
    HOMER_SSX_MAP_ERR,              // 5
    HOMER_SSX_UNMAP_ERR,            // 6
    HOMER_LAST_RC
} homer_rc_t;

//*************************************************************************
// Structures
//*************************************************************************

/*
 * Current version of the layout for the Host Config Data section of the HOMER
 */
struct occHostConfigDataArea
{
    uint32_t version;
    uint32_t nestFrequency;
    uint32_t occInterruptType;
    uint8_t  __reserved[(1024*128)-12];
}__attribute__ ((__packed__));
typedef struct occHostConfigDataArea occHostConfigDataArea_t;

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************

homer_rc_t homer_hd_map_read_unmap(const homer_read_var_t, uint32_t *, int *);

//*************************************************************************
// Functions
//*************************************************************************

#endif // _homer_h
