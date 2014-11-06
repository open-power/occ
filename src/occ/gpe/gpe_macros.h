/******************************************************************************
// @file gpe_macros.h
// @brief Common macros for the APSS/DPSS code
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section gpe_macros.h GPE_MACROS.H
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

        //--------------------------------------------------------------------
        // Macro Specification:
        // 
        // Name: _saveffdc
        //
        // Description:  Save FFDC into error structure
        //               
        //     struct {
        //         uint64_t rc;          // This should be read as 63:32=addr, 31:0=rc
        //         uint64_t ffdc;        // Whatever GPE program puts in for FFDC data
        //     } PoreGpeErrorStruct;
        //
        //               - Copy D0 into PoreGpeErrorStruct->ffdc
        //               - Copy \gpe_id into PoreGpeErrorStruct->rc[63:32]
        //               - Copy \rc into PoreGpeErrorStruct->rc[31:0]
        //
        // Inputs:       \gpe_id - Unique GPE program number to identify what prog failed
        //               \rc - Unique GPE error code that will indicate failure
        //               ETR         - Assumed to be set to base address of 
        //                             passed argument structure
        //
        // End Macro Specification
        //--------------------------------------------------------------------
        .macro _saveffdc, gpe_id, rc

        // Make sure passed Structure Pointer is loaded into A1
        mr      A1, ETR
        
        // Save address to send back (upper word of D1)
        srdi    D1, D1, 32
        // TODO: Simcs does not support SPRG0 yet, so use 24 bit CTR for now
        //mr      SPRG0, D1
        mr      CTR, D1

        // Check if rc was already saved...
        ld      D1, 0, A1
        andi    D1, D1, 0xFFFFFFFF00000000
        branz   D1, 1f

        // Save D0 into FFDC of error structure
        std     D0, 8, A1  
        ld      D0, 8, A1

        // Save address to send back... (truncated to 24 bits)
        mr      D0, CTR
        // Save GPE progam ID
        li      D1, \gpe_id
        sldi    D1, D1, 16
        // and return code
        ori     D1, D1, \rc
        sldi    D1, D1, 32
        // combine GPE program/RC with address
        or      D0, D0, D1

        // Save data into error structure 
        std     D0, 0, A1
        ld      D0, 8, A1
        ld      D0, 0, A1

1:
        .endm


        //--------------------------------------------------------------------
        // Macro Specification:
        // 
        // Name: _getscom
        // 
        // Description:  Get a SCOM based on passed in Address, put it in D0
        //
        // Inputs:       SCOM Address
        //
        // Outputs:      D0 - Result of SCOM
        //
        // End Macro Specification
        //--------------------------------------------------------------------
	.macro _getscom, address

	lpcs    P0, \address
        ld      D0, \address, P0

	.endm

        //--------------------------------------------------------------------
        // Macro Specification:
        // 
        // Name: _putscom
        // 
        // Description:  Store data into SCOM based on passed in Address
        //
        // Inputs:       SCOM Address, Data
        //
        // Outputs:      None
        //
        // End Macro Specification
        //--------------------------------------------------------------------
	.macro _putscom, address, data

	lpcs    P0, \address
        li      D0, \data
        std     D0, \address, P0

	.endm

        //--------------------------------------------------------------------
        // Macro Specification:
        // 
        // Name: _putscom_d0
        // 
        // Description:  Store D0 into SCOM based on passed in Address
        //
        // Inputs:       SCOM Address
        //
        // Outputs:      None
        //
        // End Macro Specification
        //--------------------------------------------------------------------
	.macro _putscom_d0, address

	lpcs    P0, \address
        std     D0, \address, P0

	.endm
	
//*************************************************************************
// Defines/Enums
//*************************************************************************
        #define TOD_VALUE_REG                 0x00040020

        // Constant for use in wait statments: waits (5 * MICROSECONDS)
        #define MICROSECONDS 600
		
//*************************************************************************
// Structures
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
