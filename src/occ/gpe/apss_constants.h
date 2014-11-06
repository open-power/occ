/******************************************************************************
// @file apss_constants.h
// @brief Constants used by APSS communications GPE programs
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section apss_constants.h APSS_CONSTANTS.H
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *    
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *
 *  @endverbatim
 *
 *///*************************************************************************/

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
#define SPIPSS_REGISTER_BASE          0x00020000
#define SPIPSS_ADC_CTRL_REG0          (SPIPSS_REGISTER_BASE + 0x00) 
#define SPIPSS_ADC_CTRL_REG1          (SPIPSS_REGISTER_BASE + 0x01) 
#define SPIPSS_ADC_CTRL_REG2          (SPIPSS_REGISTER_BASE + 0x02) 
#define SPIPSS_ADC_STATUS_REG         (SPIPSS_REGISTER_BASE + 0x03)
#define SPIPSS_ADC_COMMAND_REG        (SPIPSS_REGISTER_BASE + 0x04)
#define SPIPSS_ADC_WDATA_REG          (SPIPSS_REGISTER_BASE + 0x10) 
#define SPIPSS_ADC_RDATA_REG0         (SPIPSS_REGISTER_BASE + 0x20) 
#define SPIPSS_ADC_RDATA_REG1         (SPIPSS_REGISTER_BASE + 0x21) 
#define SPIPSS_ADC_RDATA_REG2         (SPIPSS_REGISTER_BASE + 0x22) 
#define SPIPSS_ADC_RDATA_REG3         (SPIPSS_REGISTER_BASE + 0x23) 
#define SPIPSS_P2S_CTRL_REG0          (SPIPSS_REGISTER_BASE + 0x40)
#define SPIPSS_P2S_CTRL_REG1          (SPIPSS_REGISTER_BASE + 0x41)
#define SPIPSS_P2S_CTRL_REG2          (SPIPSS_REGISTER_BASE + 0x42)
#define SPIPSS_P2S_STATUS_REG         (SPIPSS_REGISTER_BASE + 0x43)
#define SPIPSS_P2S_COMMAND_REG        (SPIPSS_REGISTER_BASE + 0x44)
#define SPIPSS_P2S_WDATA_REG          (SPIPSS_REGISTER_BASE + 0x50)
#define SPIPSS_P2S_RDATA_REG          (SPIPSS_REGISTER_BASE + 0x60)

//*************************************************************************
// Strustures
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
