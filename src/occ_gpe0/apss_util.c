
#include "pk.h"
#include "ppe42_scom.h"
#include "pss_constants.h"
#include "apss_util.h"
#define SPIPSS_P2S_ONGOING_MASK 0x8000000000000000

/*
 * Function Specification
 *
 * Name: apss_set_ffdc
 *
 * Description: Fills up the error struct with the given data.
 *
 * End Function Specification
 */
void apss_set_ffdc(GpeErrorStruct *o_error, uint32_t i_addr, uint32_t i_rc, uint64_t i_ffdc)
{

    o_error->addr = i_addr;
    //Return codes defined in apss_struct.h
    o_error->rc = i_rc;
    o_error->ffdc = i_ffdc;

}

/*
 * Function Specification:
 *
 * Name: wait_spi_completion
 *
 * Description:  Read the specified register (SPIPSS_P2S_STATUS_REG
 *               or SPIPSS_ADC_STATUS_REG), and check if it's p2s_ongoing
 *               bit is 0 (operations done).  If not, wait
 *               up to the timeout usec (~1usec per retry).
 *               If still not clear, continue looping,
 *               If error/reserved bits are set, a return code will be sent back
 *
 * Inputs:       error:    a pointer to a GpeErrorStruct, to populate rc, ffdc,
 *                         and address, in case a scom get error happens
 *               timeout:  # usec to wait for ongoing bit to clear
 *               Register: SPIPSS_P2S_STATUS_REG or SPIPSS_ADC_STATUS_REG
 *
 * return:       0 -> Success: spi transaction completed within timeout limit
 *               not 0 -> timeout: spi transaction did not complete within timeout
 *                        bits 0:31 are masked, and returned back for potential
 *                        analysis of the reason that the transaction timed out
 *
 * End Function Specification
 */

int wait_spi_completion(GpeErrorStruct *error, uint32_t reg, uint8_t timeout)
{
    int         i;
    int         rc;
    uint64_t    status;

    if((reg != SPIPSS_P2S_STATUS_REG) && (reg != SPIPSS_ADC_STATUS_REG))
    {
        PK_TRACE("gpe0:wait_spi_completion failed: Invalid Register 0x%08x", reg);
        rc = APSS_RC_INVALID_REG;
        apss_set_ffdc(error, reg, rc, 0x00);
    }
    else
    {
        // Keep polling the P2S_ONGOING bits for timeout
        for (i = 0; i< timeout; i++)
        {
            rc = getscom_abs(reg, &status);
            if(rc)
            {
                PK_TRACE("gpe0:wait_spi_completion failed with rc = 0x%08x", rc);
                apss_set_ffdc(error, reg, APSS_RC_SCOM_GET_FAILED, rc);
                rc = APSS_RC_SCOM_GET_FAILED;
                break;
            }


            // bit zero is the P2s_ONGOING / HWCTRL_ONGOING
            // set to 1: means operation is in progress (ONGOING)
            // set to 0: means operation is complete, therefore exit for loop.
            if((status & SPIPSS_P2S_ONGOING_MASK) == 0)
            {
                rc = 0;
                break;
            }

            // sleep for 1 microsecond before retry
            busy_wait(1);
        }
    }

    //Timed out waiting on P2S_ONGOING / HWCTRL_ONGOING bit.
    if (i >= timeout)
    {
        PK_TRACE("gpe0:wait_spi_completion Timed out waiting for p2s_ongoing to clear.");
        apss_set_ffdc(error, reg, APSS_RC_SPI_TIMEOUT, rc);
        rc = APSS_RC_SPI_TIMEOUT;
    }

    return rc;
}

/*
 * Function Specification:
 *
 * Name: busy_wait
 *
 * Description:  a counting loop to simulate sleep calls, and is ISR safe.
 *
 * Inputs:       t_microseconds: time to sleep in microseconds
 *
 * return:       none
 *
 * End Function Specification
 */

// result based on busy_wait(1) calibration against pk_sleep(1)
#define BUSY_LOOP_CONSTANT 71

void busy_wait(uint32_t t_microseconds)
{
    int j;
    volatile uint32_t i;
//    ppe42_mullw macro works fine, but compiler parameter settings seems broken, use nested loop instead
//    int loop = t * 100;  // Assuming loop iteration takes 6 ppe cycles = 0.01 microseconds

    for(j = 0; j < BUSY_LOOP_CONSTANT; j++)
        for(i = 0; i < t_microseconds; i++);

}
