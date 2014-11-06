/******************************************************************************
// @file pss_macros.h
// @brief Common macros for the APSS/DPSS code
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section pss_macros.h PSS_MACROS.H
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @gm032  918715     milesg    03/19/2014  Improved APSS recovery
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
      // Name: _wait_for_spi_ops_complete (MACRO)
      //
      // Description:  Read SPIPSS_P2S_STATUS_REG and check if p2s_ongoing
      //               bit is 0 (operations done).  If not, wait
      //               up to timeout usec (~1usec per retry).
      //               If still not clear, branch to timeout_label
      //               If error/reserved bits are set, a branch will be
      //               done to label: "error_status_reg"
      //
      // Inputs:       timeout - # usec to wait for ongoing bit to clear
      //               timeout_label - label to branch to after timeout
      //
      // Outputs:      None (on error, D0 will contain status register)
      //
      // Modifies:     CTR, D0, D1
      //--------------------------------------------------------------------
      .macro _wait_for_spi_ops_complete, timeout, timeout_label

        _wait_for_ops_complete SPIPSS_P2S_STATUS_REG, \timeout, \timeout_label

      .endm


      //--------------------------------------------------------------------
      // Name: _wait_for_adc_ops_complete (MACRO)
      //
      // Description:  Read SPIPSS_ADC_STATUS_REG and check if adc_ongoing
      //               bit is 0 (operations done).  If not, wait
      //               up to timeout usec (~1usec per retry).
      //               If still not clear, branch to timeout_label
      //               If error/reserved bits are set, a branch will be
      //               done to label: "error_status_reg"
      //
      // Inputs:       timeout - # usec to wait for ongoing bit to clear
      //               timeout_label - label to branch to after timeout
      //
      // Outputs:      None (on error, D0 will contain status register)
      //
      // Modifies:     CTR, D0, D1
      //--------------------------------------------------------------------
      .macro _wait_for_adc_ops_complete, timeout, timeout_label

        _wait_for_ops_complete SPIPSS_ADC_STATUS_REG, \timeout, \timeout_label

      .endm


      //--------------------------------------------------------------------
      // Name: _wait_for_ops_complete (MACRO)
      //
      // Description:  Read specified register and check if ongoing bit (MSB)
      //               is 0 (operations done).  If not, wait
      //               up to timeout usec (~1usec per retry).
      //               If still not clear, branch to timeout_label
      //               If error/reserved bits are set, a branch will be
      //               done to label: "error_status_reg"
      //
      // Inputs:       register - SCOM register to read for ongong bit
      //               timeout - # usec to wait for ongoing bit to clear
      //               timeout_label - label to branch to after timeout
      //
      // Outputs:      None (on error, D0 will contain status register)
      //
      // Modifies:     CTR, D0, D1
      //--------------------------------------------------------------------
      .macro _wait_for_ops_complete, register, timeout, timeout_label

        // Load CTR with approximate timeout value (1usec delay per retry)
        li CTR, (\timeout - 1)
1:
        // Read spiadc_p2s_ongoing bit into D0
        _getscom \register

        // Don't fail on these status bits being set.  Only fail if we get back invalid power data -- gm032
        // Verify other error bits are not set
        // andi    D1, D0, 0x7FFFFFFFFFFFFFFF
        // branz   D1, error_statusreg

        // Operation finished? (spiadc_p2s_ongoing = 0)
        andi    D1, D0, 0x8000000000000000
        braz    D1, 2f

        // no, wait 1usec and retry
        waits (1 * MICROSECONDS)
        loop     1b

        // Timeout waiting for spiadc_p2s_ongoing
        bra     \timeout_label
2:
      .endm



      // TODO - Subroutine would be more efficient than macro, however:
      // Subroutines do not currently clear the stack after a halt
      // so we can NOT use subroutine (SCOM error, branch on fail, etc..)
#if 0
      //--------------------------------------------------------------------
      // Name: sub_wait_for_spi_ops_complete (SUBROUTINE)
      //
      // Description:  Read SPIPSS_P2S_STATUS_REG and check if p2s_ongoing
      //               bit is 0 (operations done).  If not it will wait
      //               for 9 additional retries (~10usec).  If still not
      //               done, a branch will be done to timeout code.
      //               If error/reserved bits are set, a branch will be
      //               done to error_status_reg.
      //
      // Inputs:       None
      //
      // Outputs:      None (on error, D0 will contain status register)
      //
      // Modifies:     CTR, D0, D1
      //--------------------------------------------------------------------
sub_wait_for_spi_ops_complete:
        // Wait up to 10usec
        li CTR, 10

read_status:
             // Read spiadc_p2s_ongoing bit into D0
        _getscom SPIPSS_P2S_STATUS_REG

        // Verify other error bits are not set
        andi    D1, D0, 0x7FFFFFFFFFFFFFFF
        branz   D1, error_statusreg

        // Ready to send command? (spiadc_p2s_ongoing = 0)
        andi    D1, D0, 0x8000000000000000
        braz    D1, ops_are_complete

        // no, wait 1usec and retry
        waits (1 * MICROSECONDS)
        loop     read_status

        // Timeout waiting for spiadc_p2s_ongoing
        bra     error_timeout
ops_are_complete:
        ret
#endif
//*************************************************************************
// Defines/Enums
//*************************************************************************

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


