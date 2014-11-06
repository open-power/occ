/******************************************************************************
// @file amec_controller.c
// @brief Amester interface
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _amec_controller_c amec_controller.c
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @ry002  862116     ronda     11/26/2012  Thermal controller for processor
 *   @ry003  870734     ronda     02/20/2013  Thermal controller for memory
 *   @ry004  872358     ronda     02/28/2013  Update function description
 *   @fk001  879727     fmkassem  04/16/2013  Power capping support.  
 *   @cl001             lefurgy   07/24/2013  Fix overflow in l_throttle
 *   @gs009  897228     gjsilva   08/28/2013  Fix sensor name for proc thermal control
 *   @gm016  909061     milesg    12/10/2013  Support memory throttling due to temperature
 *   @gs023  912003     gjsilva   01/16/2014  Generate VRHOT signal and control loop
 *
 *  @endverbatim
 *
 *///*************************************************************************/

//*************************************************************************
// Includes
//*************************************************************************
//@pb00Ec - changed from common.h to occ_common.h for ODE support
#include <occ_common.h>
#include <sensor.h>
#include <amec_sys.h>

//*************************************************************************
// Externs
//*************************************************************************
extern uint8_t G_dimm_temp_expired_bitmap;
extern uint8_t G_cent_temp_expired_bitmap;
//*************************************************************************
// Macros
//*************************************************************************

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
// Function Specification
//
// Name: amec_controller_proc_thermal
//
// Description: This function implements the Proportional Controller for the 
//              processor thermal control. Although it doesn't return any 
//              results, it populates the thermal vote in the field 
//              g_amec->thermalproc.speed_request.
//
//
// Flow:    5-13-13    FN=amec_thrm_proc_control.odg
//
// Task Flags:
//
// End Function Specification
void amec_controller_proc_thermal()
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                      l_thermal_winner = 0;
    uint16_t                      l_residue = 0;
    uint16_t                      l_old_residue = 0;
    int16_t                       l_error = 0;
    int16_t                       l_cpu_speed = 0;
    int16_t                       l_throttle_chg = 0;
    int32_t                       l_throttle = 0;  // @cl001 
    sensor_t                    * l_sensor = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    // Get TEMP2MSP0PEAK sensor, which is hottest core temperature in OCC processor
    l_sensor = getSensorByGsid(TEMP2MSP0PEAK);
    
    // Use the highest temperature of all processors in 0.1 degrees C
    l_thermal_winner = l_sensor->sample * 10;
   
    // Check if there is an error
    if (g_amec->thermalproc.setpoint == l_thermal_winner) 
        return;

    // Calculate the thermal control error
    l_error = g_amec->thermalproc.setpoint - l_thermal_winner;

    // TODO: Get with Malcolm to migrate to 16-bit multiply
    // Proportional Controller for the thermal control loop
    l_throttle = (int32_t) l_error * g_amec->thermalproc.Pgain;
    l_residue = (uint16_t) l_throttle;
    l_throttle_chg = (int16_t) (l_throttle >> 16);

    if ((int16_t) l_throttle_chg > (int16_t) g_amec->sys.speed_step_limit)
    {
        l_throttle_chg = g_amec->sys.speed_step_limit;
    }
    else
    {
        if ((int16_t) l_throttle_chg < ((int16_t) (-g_amec->sys.speed_step_limit)))
        {
            l_throttle_chg = (int16_t)(-g_amec->sys.speed_step_limit);
        }
    }

    // Calculate the new thermal CPU speed request
    l_cpu_speed = g_amec->thermalproc.speed_request +
        (int16_t) l_throttle_chg * g_amec->sys.speed_step;

    // Proceed with residue summation to correctly follow set-point
    l_old_residue = g_amec->thermalproc.total_res;
    g_amec->thermalproc.total_res += l_residue;
    if (g_amec->thermalproc.total_res < l_old_residue)
    {
        l_cpu_speed += g_amec->sys.speed_step;
    }

    // Enforce actuator saturation limits
    if (l_cpu_speed > g_amec->sys.max_speed)
        l_cpu_speed = g_amec->sys.max_speed;
    if (l_cpu_speed < g_amec->sys.min_speed)
        l_cpu_speed = g_amec->sys.min_speed;

    // Generate the new thermal speed request
    g_amec->thermalproc.speed_request = l_cpu_speed; 
    // Calculate frequency request based on thermal speed request
    g_amec->thermalproc.freq_request = amec_controller_speed2freq(
            g_amec->thermalproc.speed_request,
            g_amec->sys.fmax);	// @fk001m
}      

//*************************************************************************
// Function Specification
//
// Name: amec_controller_dimm_thermal
//
// Description: This function implements the Proportional Controller for the 
//              DIMM thermal control. Although it doesn't return any 
//              results, it populates the thermal vote in the field 
//              g_amec->thermaldimm.speed_request.
//
//
// Flow:    2-15-13    FN=amec_thrm_dimm_control.odg
//
// Task Flags:
//
// End Function Specification
void amec_controller_dimm_thermal()
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                      l_thermal_winner = 0;
    uint16_t                      l_residue = 0;
    uint16_t                      l_old_residue = 0;
    int16_t                       l_error = 0;
    int16_t                       l_mem_speed = 0;
    int16_t                       l_throttle_chg = 0;
    int32_t                       l_throttle = 0;
    sensor_t                    * l_sensor = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    // Get TEMP2MSDIMM sensor value
    l_sensor = getSensorByGsid(TEMP2MSDIMM);
   
    if(G_dimm_temp_expired_bitmap) //gm016
    {
       //we were not able to read one or more dimm temperatures.
       //Assume temperature is at the setpoint plus 1 degree C.
       l_thermal_winner = g_amec->thermaldimm.setpoint + 10;
    }
    else 
    {
        // Use the highest temperature of all DIMMs in 0.1 degrees C
        l_thermal_winner = l_sensor->sample * 10;
    }

    // Check if there is an error
    if (g_amec->thermaldimm.setpoint == l_thermal_winner)
        return;

    // Calculate the thermal control error
    l_error = g_amec->thermaldimm.setpoint - l_thermal_winner;

    // Proportional Controller for the thermal control loop based on DIMM
    // temperatures
    l_throttle = (int32_t) l_error * g_amec->thermaldimm.Pgain;
    l_residue = (uint16_t) l_throttle;
    l_throttle_chg = (int16_t) (l_throttle >> 16);

    if ((int16_t) l_throttle_chg > AMEC_MEMORY_SPEED_CHANGE_LIMIT)
    {
        l_throttle_chg = AMEC_MEMORY_SPEED_CHANGE_LIMIT;
    }
    else
    {
        if ((int16_t) l_throttle_chg < (-AMEC_MEMORY_SPEED_CHANGE_LIMIT))
        {
            l_throttle_chg = -AMEC_MEMORY_SPEED_CHANGE_LIMIT;
        }
    }

    // Calculate the new thermal speed request for DIMMs
    l_mem_speed = g_amec->thermaldimm.speed_request +
        (int16_t) l_throttle_chg * AMEC_MEMORY_STEP_SIZE;

    // Proceed with residue summation to correctly follow set-point
    l_old_residue = g_amec->thermaldimm.total_res;
    g_amec->thermaldimm.total_res += l_residue;
    if (g_amec->thermaldimm.total_res < l_old_residue)
    {
        l_mem_speed += AMEC_MEMORY_STEP_SIZE;
    }

    // Enforce actuator saturation limits
    if (l_mem_speed > AMEC_MEMORY_MAX_STEP)
        l_mem_speed = AMEC_MEMORY_MAX_STEP;
    if (l_mem_speed < AMEC_MEMORY_MIN_STEP)
        l_mem_speed = AMEC_MEMORY_MIN_STEP;

    // Generate the new thermal speed request
    g_amec->thermaldimm.speed_request = (uint16_t) l_mem_speed;
}


//*************************************************************************
// Function Specification
//
// Name: amec_controller_centaur_thermal
//
// Description: This function implements the Proportional Controller for the 
//              centaur thermal control. Although it doesn't return any 
//              results, it populates the thermal vote in the field 
//              g_amec->thermalcent.speed_request.
//
//
// Flow:    2-15-13    FN=amec_thrm_centaur_control.odg
//
// Task Flags:
//
// End Function Specification    
void amec_controller_centaur_thermal()
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                      l_thermal_winner = 0;
    uint16_t                      l_residue = 0;
    uint16_t                      l_old_residue = 0;
    int16_t                       l_error = 0;
    int16_t                       l_mem_speed = 0;
    int16_t                       l_throttle_chg = 0;
    int32_t                       l_throttle = 0;
    sensor_t                    * l_sensor = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    // Get TEMP2MSCENT sensor value
    l_sensor = getSensorByGsid(TEMP2MSCENT);
   
    if(G_cent_temp_expired_bitmap) //gm016
    {
        //we were not able to get a valid temperature.  Assume it is 1 degree
        //over the setpoint.
        l_thermal_winner = g_amec->thermalcent.setpoint + 10;
    }
    else
    { 
        // Use the highest temperature of all Centaur in 0.1 degrees C
        l_thermal_winner = l_sensor->sample * 10;
    }

    // Check if there is an error
    if (g_amec->thermalcent.setpoint == l_thermal_winner)
        return;

    // Calculate the thermal control error
    l_error = g_amec->thermalcent.setpoint - l_thermal_winner;

    // Proportional Controller for the thermal control loop based on CENTAUR
    // temperatures
    l_throttle = (int32_t) l_error * g_amec->thermalcent.Pgain;
    l_residue = (uint16_t) l_throttle;
    l_throttle_chg = (int16_t) (l_throttle >> 16);

    if ((int16_t) l_throttle_chg > AMEC_MEMORY_SPEED_CHANGE_LIMIT)
    {
        l_throttle_chg = AMEC_MEMORY_SPEED_CHANGE_LIMIT;
    }
    else
    {
        if ((int16_t) l_throttle_chg < (-AMEC_MEMORY_SPEED_CHANGE_LIMIT))
        {
            l_throttle_chg = -AMEC_MEMORY_SPEED_CHANGE_LIMIT;
        }
    }

    // Calculate the new thermal speed request for Centaurs
    l_mem_speed = g_amec->thermalcent.speed_request +
        (int16_t) l_throttle_chg * AMEC_MEMORY_STEP_SIZE;

    // Proceed with residue summation to correctly follow set-point
    l_old_residue = g_amec->thermalcent.total_res;
    g_amec->thermalcent.total_res += l_residue;
    if (g_amec->thermalcent.total_res < l_old_residue)
    {
        l_mem_speed += AMEC_MEMORY_STEP_SIZE;
    }

    // Enforce actuator saturation limits
    if (l_mem_speed > AMEC_MEMORY_MAX_STEP)
        l_mem_speed = AMEC_MEMORY_MAX_STEP;
    if (l_mem_speed < AMEC_MEMORY_MIN_STEP)
        l_mem_speed = AMEC_MEMORY_MIN_STEP;

    // Generate the new thermal speed request
    g_amec->thermalcent.speed_request = (uint16_t) l_mem_speed;
}

// Function Specification
//
// Name: amec_controller_vrhotproc
//
// Description: This function implements a Single-step Controller for the 
//              processor VRHOT signal. Although it doesn't return any 
//              results, it populates the frequency vote in the field 
//              g_amec->vrhotproc.freq_request
//
//
// End Function Specification
void amec_controller_vrhotproc()
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    int16_t                       l_cpu_speed = 0;
    int16_t                       l_throttle_chg = 0;
    sensor_t                      *l_sensor = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    // Get VRHOT sensor, which is really a counter of consecutive times the
    // VRHOT signal has been asserted
    l_sensor = getSensorByGsid(VRHOT250USPROC);
    
    // Single-step controller:
    //   If sensor VRHOT250USPROC is above the set-point, we need to step down.
    //   Else, do a single step up.
    if (l_sensor->sample >= g_amec->vrhotproc.setpoint)
    {
        l_throttle_chg = (int16_t)(-1);
    }
    else
    {
        l_throttle_chg = (int16_t)(1);
    }

    // Calculate the new VRHOTPROC speed request
    l_cpu_speed = g_amec->vrhotproc.speed_request +
        (int16_t) l_throttle_chg * g_amec->sys.speed_step;

    // Never allow negative speed requests
    if (l_cpu_speed < 0)
    {
        l_cpu_speed = 0;
    }

    // Enforce actuator saturation limits
    if (l_cpu_speed > g_amec->sys.max_speed)
    {
        l_cpu_speed = g_amec->sys.max_speed;
    }
    if (l_cpu_speed < g_amec->sys.min_speed)
    {
        l_cpu_speed = g_amec->sys.min_speed;
    }

    // Generate the new VRHOTPROC frequency request
    g_amec->vrhotproc.speed_request = l_cpu_speed;
    g_amec->vrhotproc.freq_request = 
        amec_controller_speed2freq(g_amec->vrhotproc.speed_request,
                                   g_amec->sys.fmax);

}      


// Function Specification
//
// Name: amec_controller_speed2freq
//
// Description: Helper function to convert speed to MHz.
//
// Flow:              FN= None
//
// End Function Specification
uint16_t amec_controller_speed2freq (const uint16_t i_speed, const uint16_t i_fmax)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                    l_tempreg = 0;
    uint16_t                    l_temp16 = 0;
    uint32_t                    l_temp32 = 0;
    uint16_t                    l_freq = 0;
    uint32_t                    l_divide32[2] = {0, 0};

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    l_temp16 = i_fmax;
    l_tempreg = (uint16_t)i_speed;
    l_temp32 = ((uint32_t)l_tempreg)*((uint32_t)l_temp16);
    l_temp16 = (uint16_t)1000;
    l_divide32[1] = (uint32_t)l_temp16;
    l_divide32[0] = (uint32_t)l_temp32;
    l_divide32[0] /= l_divide32[1];
    l_temp32 = l_divide32[0];
    l_freq = (uint16_t)l_temp32; /* freq will always fit in 16 bits */
    
    return l_freq;
}
