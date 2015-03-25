/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occApplet/testApplet/sensorTest.c $                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2015                        */
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

#define SNSR_DEBUG
//*************************************************************************
// Includes
//*************************************************************************
#include <common_types.h>   // imageHdr_t declaration and image header macro
#include <errl.h>           // For error handle
#include "ssx_io.h"         // For printfs
#include <sensor.h>         // Sensor interfaces
#include <occ_service_codes.h> // Reason code
#include <errl.h>           // For errlHndl_t
#include <trac.h>           // For traces
#include <appletManager.h>
#include <sensorQueryList.h>
#include <appletId.h>       // For applet ID
#include <aplt_service_codes.h>         // For test applet module ID
#include <testApltId.h>     // For test applet ID

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define SENSORTESTMAIN_ID  "Sensor Test\0"
#define UINT16_MIN                  0

//sensor test module ID enumeration
typedef enum
{
    TEST_NULL_POINTER = 0x00,
    TEST_GET_SENSOR_BY_GSID = 0x01,
    TEST_COMMON = 0x02,
    TEST_QUERY_LIST = 0x03,

} sensorTestModId;

// sensor test return codes
typedef enum
{
    SUCCESS_RC                  = 0x00000000,
    GET_SNSR_FAILURE            = 0x00000001,
    SNSR_RESET_FAILURE          = 0x00000002,
    SNSR_UPDT_FAILURE           = 0x00000003,
    SNSR_UPDT_FAILURE2          = 0x00000004,
    SNSR_UPDT_FAILURE3          = 0x00000005,
    SNSR_UPDT_FAILURE4          = 0x00000006,
    SNSR_UPDT_FAILURE5          = 0x00000007,
    SNSR_RESET_FAILURE2         = 0x00000008,
    RESET_WITH_VECTORIZE_FAILURE= 0x00000009,
    VECTORIZE_FAILURE           = 0x0000000A,
    VECTORIZE_FAILURE2          = 0x0000000B,
    VECTORIZE_FAILURE3          = 0x0000000C,
    VECTOR_ADD_FAILURE          = 0x0000000D,
    VECTOR_ADD_FAILURE2         = 0x0000000E,
    VECTOR_ADD_FAILURE3         = 0x0000000F,
    VECTOR_UPDT_FAILURE         = 0x00000010,
    VECTOR_UPDT_FAILURE2        = 0x00000011,
    VECTOR_UPDT_FAILURE3        = 0x00000012,
    VECTOR_UPDT_FAILURE4        = 0x00000013,
    VECTOR_UPDT_FAILURE5        = 0x00000014,
    VECTOR_UPDT_FAILURE6        = 0x00000015,
    VECTOR_UPDT_FAILURE7        = 0x00000016,
    VECTOR_UPDT_FAILURE8        = 0x00000017,
    VECTOR_UPDT_FAILURE9        = 0x00000018,
    VECTOR_UPDT_FAILURE10       = 0x00000019,
    VECTOR_ADD_FAILURE4         = 0x0000001A,
    QUERY_LIST_FAILURE          = 0x0000001B,
    QUERY_LIST_FAILURE2         = 0x0000001C,
    QUERY_LIST_FAILURE3         = 0x0000001D,
    QUERY_LIST_FAILURE4         = 0x0000001E,
    QUERY_LIST_FAILURE5         = 0x0000001F,
    QUERY_LIST_FAILURE6         = 0x00000020,
    QUERY_LIST_FAILURE7         = 0x00000021,
    QUERY_LIST_FAILURE8         = 0x00000022,
    QUERY_LIST_FAILURE9         = 0x00000023,
    QUERY_LIST_FAILURE10        = 0x00000024,
    QUERY_LIST_FAILURE11        = 0x00000025,
    QUERY_LIST_FAILURE12        = 0x00000026,
    QUERY_LIST_FAILURE13        = 0x00000027,
    QUERY_LIST_FAILURE14        = 0x00000028,
    GET_SNSR_FAILURE1           = 0x00000029,
    GET_SNSR_FAILURE2           = 0x0000002A,
} sensorTestRc;

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************
// Stack size for test applet thread is only 4000 bytes,
// put big structure here to prevent stack overflow

// ifdef'd out b/c the sensor list is too big to test in applet
#if 0
sensorQueryList_t G_snsrList[NUMBER_OF_SENSORS_IN_LIST];
#endif

//*************************************************************************
// Function Prototypes
//*************************************************************************
uint32_t sensorTestNullPointer();
uint32_t sensorTestGetSensorByGsid();
uint32_t sensorTestCommon();
void printSensor(sensor_t i_snsr);
uint32_t sensorTestQueryList();

//*************************************************************************
// Functions
//*************************************************************************

// Function Specification
//
// Name: sensorTestMain
//
// Description: Entry point function
//
// End Function Specification
errlHndl_t sensorTestMain(void * i_arg)
{
    SNSR_DBG("Enter sensorTestMain\n");
    errlHndl_t l_err = NULL;

    uint32_t l_rc = SUCCESS_RC;
    uint8_t l_modId = 0;

    do
    {
        l_rc = sensorTestNullPointer();
        if( l_rc != SUCCESS_RC)
        {
            l_modId = TEST_NULL_POINTER;
            TRAC_INFO("Failure on null pointer test");
            break;
        }

        l_rc = sensorTestGetSensorByGsid();
        if( l_rc != SUCCESS_RC)
        {
            l_modId = TEST_GET_SENSOR_BY_GSID;
            TRAC_INFO("Failure on get sensor by Gsid test");
            break;
        }

        l_rc = sensorTestCommon();
        if( l_rc != SUCCESS_RC)
        {
            l_modId = TEST_COMMON;
            TRAC_INFO("Failure on common test");
            break;
        }
// If'd out b/c the sensor list is too big for an applet
#if 0
        l_rc = sensorTestQueryList();
        if( l_rc != SUCCESS_RC)
        {
            l_modId = TEST_QUERY_LIST;
            TRAC_INFO("Failure on query list test");
            break;
        }
#endif
    } while (0);

    if( l_rc != SUCCESS_RC)
    {
        SNSR_DBG("**********************************************\n");
        SNSR_DBG("* Sensor Test Failed: ModId: 0x%x,l_rc: 0x%x\n",l_modId,l_rc);
        SNSR_DBG("**********************************************\n");
        /* @
         * @errortype
         * @moduleid       TEST_APLT_MODID_SENSORTEST
         * @reasoncode     INTERNAL_FAILURE
         * @userdata1      Test Applet ID
         * @userdata2      Return Code
         * @userdata4      OCC_NO_EXTENDED_RC
         * @devdesc        Failure executing test applet
         */
        l_err = createErrl(TEST_APLT_MODID_SENSORTEST,
                           INTERNAL_FAILURE,        // @nh001c
                           OCC_NO_EXTENDED_RC,
                           ERRL_SEV_INFORMATIONAL,
                           NULL,
                           0,
                           SNSR_TEST_APLT,
                           l_rc);
    }
    else
    {
        SNSR_DBG("**********************************************\n");
        SNSR_DBG("* Sensor Test Passed\n");
        SNSR_DBG("**********************************************\n");
    }


    SNSR_DBG("Exit sensorTestMain\n");

    return l_err;
}


// Function Specification
//
// Name: sensorTestNullPointer
//
// Description: SensorTestNullPointers
//
// End Function Specification
uint32_t sensorTestNullPointer()
{
    uint32_t l_rc = SUCCESS_RC;
    /****************************************************/
    // Try to clear minmax with NULL sensor pointer. Call will be no-op
    sensor_clear_minmax(NULL);

    /****************************************************/
    // Try to reset sensor with NULL sensor pointer. Call will be no-op
    sensor_reset(NULL);

    /****************************************************/
    // Try to vectorize with NULL sensor pointer and valid vector sensor.
    // Call will be no-op
    vectorSensor_t l_vecsnsr;
    sensor_vectorize(NULL,&l_vecsnsr,0);

    /****************************************************/
    // Try to vectorize with NULL vector sensor pointer and valid sensor pointer
    // Call will be no-op
    sensor_t l_snsr;
    sensor_vectorize(&l_snsr,NULL,0);

    /****************************************************/
    // Try to update sensor with NULL sensor pointer. Call will be no-op
    sensor_update(NULL,1);

    /****************************************************/
    // Try to do vector update with NULL sensor pointer. Call will be no-op
    sensor_vector_update(NULL,1);

    /****************************************************/
    // Try to enable vector  sensor with NULL pointer. Call will be no-op
    sensor_vector_elem_enable(NULL,1,1);

    /****************************************************/
    // Try to add vector element with NULL vector sensor pointer.
    // Call will be no-op
    sensor_t l_sensor;
    sensor_vector_elem_add(NULL,1,&l_sensor);

    /****************************************************/
    // Try to add vector element with NULL element pointer.
    // Call will be no-op
    vectorSensor_t l_vecSensor;
    sensor_vector_elem_add(&l_vecSensor,1,NULL);

    return l_rc;
}


// Function Specification
//
// Name: sensorTestGetSensorByGsid
//
// Description: SensorTestGetSensorByGsid
//
// End Function Specification
uint32_t sensorTestGetSensorByGsid()
{
    uint32_t l_rc = SUCCESS_RC;

    do
    {
        /****************************************************/
        // Try to get sensor with invalid GSID. It should return NULL
        sensor_t * l_sensor = NULL;
        l_sensor = getSensorByGsid(0xFFFF);

        if( l_sensor != NULL)
        {
            l_rc = GET_SNSR_FAILURE;
            break;
        }

        // Good path for this function is already tested with other test cases.

    }while(0);

    return l_rc;
}


// Function Specification
//
// Name: printSensor
//
// Description: printSensor
//
// End Function Specification
void printSensor(sensor_t i_snsr)
{
    SNSR_DBG("Printing sensor information\n");
    SNSR_DBG("*******************************\n");
    SNSR_DBG("GSID: 0x%x\n",i_snsr.gsid);
    SNSR_DBG("sample: 0x%x\n",i_snsr.sample);
    SNSR_DBG("sample_min: 0x%x\n",i_snsr.sample_min);
    SNSR_DBG("sample_max: 0x%x\n",i_snsr.sample_max);
    SNSR_DBG("status.reset: 0x%x\n",i_snsr.status.reset);
    SNSR_DBG("accumulator: 0x%x\n",i_snsr.accumulator);
    SNSR_DBG("update_tag: 0x%x\n",i_snsr.update_tag);
    SNSR_DBG("src_accum_snapshot: 0x%x\n",i_snsr.src_accum_snapshot);
    SNSR_DBG("ipmi sensor id: 0x%x\n",i_snsr.ipmi_sid);  //@fk009c
    SNSR_DBG("vector sensor:\n");
    if( i_snsr.vector != NULL)
    {
        SNSR_DBG("\toperation: 0x%x\n",i_snsr.vector->operation);
        SNSR_DBG("\tsize: 0x%x\n",i_snsr.vector->size);
        SNSR_DBG("\tmin_pos: 0x%x\n",i_snsr.vector->min_pos);
        SNSR_DBG("\tmax_pos: 0x%x\n",i_snsr.vector->max_pos);
        uint16_t i = 0;
        for(; i < i_snsr.vector->size; i++)
        {
            SNSR_DBG("\t[%d].sample:0x%x\n",i,
                   i_snsr.vector->source_ptr[i]->sample);
        } // end for loop
    }
    SNSR_DBG("mini sensor:\n");
    if( i_snsr.mini_sensor != NULL)
    {
        SNSR_DBG("\tmini sensor sample: 0x%x\n",*(i_snsr.mini_sensor));
    }
    SNSR_DBG("*******************************\n");

}


// Function Specification
//
// Name: sensorTestCommon
//
// Description: sensorTestCommon
//
// End Function Specification
uint32_t sensorTestCommon()
{
    uint32_t l_rc = SUCCESS_RC;
    /****************************************************/
    // Use test sensors to avoid the conflict with other components
    //   PROBE250US0, // Internal Sensor for debug via AMESTER
    //   PROBE250US1, // Internal Sensor for debug via AMESTER
    //   PROBE250US2, // Internal Sensor for debug via AMESTER
    //   PROBE250US3, // Internal Sensor for debug via AMESTER
    //   PROBE250US4, // Internal Sensor for debug via AMESTER
    //   PROBE250US5, // Internal Sensor for debug via AMESTER
    //   PROBE250US6, // Internal Sensor for debug via AMESTER
    //   PROBE250US7, // Internal Sensor for debug via AMESTER

    sensor_t *l_sensor=getSensorByGsid(PROBE250US0);
    sensor_t *l_sensor1=getSensorByGsid(PROBE250US1);
    sensor_t *l_sensor2=getSensorByGsid(PROBE250US4);
    sensor_t *l_sensor3=getSensorByGsid(PROBE250US4);  // @cl020 no probe7, so use probe4 again.
    do
    {
        // make sure getSensorByGsid did not return NULL pointer.
        if( (l_sensor == NULL) ||
            (l_sensor1 == NULL) ||
            (l_sensor2 == NULL) ||
            (l_sensor3 == NULL))
        {
            l_rc = GET_SNSR_FAILURE2;
            break;
        }

        //Check sensor gsid
        if((l_sensor->gsid!=PROBE250US0) ||
           (l_sensor1->gsid!=PROBE250US1) ||
           (l_sensor2->gsid!=PROBE250US4) ||
           (l_sensor3->gsid!=PROBE250US4)     )  // @cl020
        {
            l_rc = GET_SNSR_FAILURE1;
            break;
        }

        // Reset sensors before any other test so that we don't
        // end up using old data if exists.
        sensor_reset(l_sensor);
        sensor_reset(l_sensor1);
        sensor_reset(l_sensor2);
        sensor_reset(l_sensor3);

        /****************************************************/
        uint16_t l_miniSnsr = 0;
        // Set minisnsr to test minisnr reset and update
        l_sensor2->mini_sensor = &l_miniSnsr;
        // test update sensors.
        sensor_update(l_sensor,10);
        sensor_update(l_sensor1,11);
        sensor_update(l_sensor2,12);

        if( (l_sensor->sample != 10) ||
            (l_sensor->mini_sensor != NULL) ||
            (l_sensor->sample_max!=10) ||
            (l_sensor->sample_min!=10) ||
            (l_sensor1->sample != 11) ||
            (l_sensor1->mini_sensor != NULL) ||
            (l_sensor1->sample_max!=11) ||
            (l_sensor1->sample_min!=11) ||
            (l_sensor2->sample != 12) ||
            (l_sensor2->mini_sensor == NULL) ||
            (l_sensor2->sample_max!=12) ||
            (l_sensor2->sample_min!=12) ||
            (*(l_sensor2->mini_sensor) != 12) ||
            (l_miniSnsr != 12)
            )
        {
            l_rc = SNSR_UPDT_FAILURE;
            break;
        }

        /****************************************************/
        // test reset sensors without vectorize
        sensor_reset(l_sensor2);
        if( (l_sensor2->sample != 0) ||
            (l_sensor2->status.reset != 0) ||
            (l_sensor2->sample_min != UINT16_MAX) ||
            (l_sensor2->sample_max != UINT16_MIN) ||
            (*(l_sensor2->mini_sensor) != 0) ||
            (l_miniSnsr != 0))
        {
            l_rc = SNSR_RESET_FAILURE;
            break;
        }
        /****************************************************/
        // test update of sensor with reset status bit set
        l_sensor2->status.reset = 1;
        sensor_update(l_sensor2,12);

        if( (l_sensor2->sample != 12) ||
            (l_sensor2->status.reset != 0) ||
            (l_sensor2->sample_min != 12) ||
            (l_sensor2->sample_max != 12) )
        {
            l_rc = SNSR_RESET_FAILURE2;
            break;
        }

        /****************************************************/
        // vectorize with reset status bit set
        l_sensor->status.reset = 1;
        vectorSensor_t l_vecSnsr;

        sensor_vectorize(l_sensor,&l_vecSnsr, VECTOR_OP_MIN);

        if(l_sensor->vector==NULL)
        {
            l_rc = VECTORIZE_FAILURE2;
            break;
        }
        else
        {
           if(l_vecSnsr.operation!=VECTOR_OP_MIN)
           {
               l_rc = VECTORIZE_FAILURE3;
               break;
           }
        }

        /****************************************************/
        // test getSensorByGsid with valid GSID
        sensor_t * l_tempSensor = getSensorByGsid(PROBE250US0);

        if((l_tempSensor == NULL) ||
           (l_tempSensor->gsid!=PROBE250US0))
        {
            l_rc = GET_SNSR_FAILURE;
            break;
        }
        else
        {
            if( l_tempSensor->vector->operation != VECTOR_OP_MIN)
            {
                l_rc = VECTORIZE_FAILURE;
                break;
            }
        }
        /****************************************************/
        // test reset sensor with vectorize
        sensor_reset(l_sensor);
        if((l_sensor->vector->max_pos != VECTOR_SENSOR_DEFAULT_VAL) ||
           (l_vecSnsr.max_pos != VECTOR_SENSOR_DEFAULT_VAL) ||
           (l_sensor->vector->min_pos != VECTOR_SENSOR_DEFAULT_VAL) ||
           (l_vecSnsr.min_pos != VECTOR_SENSOR_DEFAULT_VAL) ||
           (l_sensor->sample_min != UINT16_MAX) ||
           (l_sensor->sample_max != UINT16_MIN) )
        {
            l_rc = RESET_WITH_VECTORIZE_FAILURE;
            break;
        }

        /****************************************************/
        // test update of sensor with different min and max values
        sensor_update(l_sensor,10);

        sensor_update(l_sensor3,5);
        if( (l_sensor3->sample != 5) ||
            (l_sensor3->mini_sensor != NULL) ||
            (l_sensor3->sample_max!=5) ||
            (l_sensor3->sample_min!=5)
            )
        {
            l_rc = SNSR_UPDT_FAILURE2;
            break;
        }
        sensor_update(l_sensor3,2);
        if( (l_sensor3->sample != 2) ||
            (l_sensor3->mini_sensor != NULL) ||
            (l_sensor3->sample_max!=5) ||
            (l_sensor3->sample_min!=2)
            )
        {
            l_rc = SNSR_UPDT_FAILURE3;
            break;
        }
        sensor_update(l_sensor3,8);
        if( (l_sensor3->sample != 8) ||
            (l_sensor3->mini_sensor != NULL) ||
            (l_sensor3->sample_max!=8) ||
            (l_sensor3->sample_min!=2)
            )
        {
            l_rc = SNSR_UPDT_FAILURE4;
            break;
        }
        sensor_update(l_sensor3,5);
        if( (l_sensor3->sample != 5) ||
            (l_sensor3->mini_sensor != NULL) ||
            (l_sensor3->sample_max!=8) ||
            (l_sensor3->sample_min!=2)
            )
        {
            l_rc = SNSR_UPDT_FAILURE5;
            break;
        }

        /****************************************************/
        // test adding vector element with invalid location
        sensor_vector_elem_add(l_sensor->vector,0xFF,l_sensor3);

        if( (l_sensor->vector->size != 0) ||
            (l_vecSnsr.size != 0))
        {
            l_rc = VECTOR_ADD_FAILURE;
            break;
        }

        /****************************************************/
        // test adding vector element with valid location
        sensor_vector_elem_add(l_sensor->vector,0,l_sensor2);

        if( (l_sensor->vector->size != 1) ||
            (l_vecSnsr.size != 1))
        {
            l_rc = VECTOR_ADD_FAILURE4;
            break;
        }

        /****************************************************/
        // test adding vector element with non-contiguous location.
        // It should not add element to the vector
        sensor_vector_elem_add(l_sensor->vector,2,l_sensor1);

        if((l_sensor->vector->size != 1) ||
           (l_vecSnsr.size != 1))
        {
            l_rc = VECTOR_ADD_FAILURE2;
            break;
        }

        /****************************************************/
        // test adding vector element with contiguous location
        sensor_vector_elem_add(l_sensor->vector,1,l_sensor1);
        sensor_vector_elem_add(l_sensor->vector,0,l_sensor3);

        if((l_sensor->vector->size != 2) ||
           (l_vecSnsr.size != 2))
        {
            l_rc = VECTOR_ADD_FAILURE3;
            break;
        }
        sensor_vector_elem_add(l_sensor->vector,2,l_sensor2);

        //so far the source sensors of vector sensor are:
        //   l_sensor3=5
        //   l_sensor1=11
        //   l_sensor2=12

        /****************************************************/
        // test disabling vector element with valid location
        sensor_vector_elem_enable(l_sensor->vector,0,0);

        /****************************************************/
        // test updating vector element with min operation and
        // element disabled

        uint32_t l_tempacc=l_sensor->accumulator;
        sensor_vector_update(l_sensor,0);
        if( (l_sensor->sample != 11) || (l_sensor->sample_min != 10 ) ||
            (l_sensor->sample_max != 11 ) ||
            (l_sensor->accumulator != l_tempacc+11)||
            (l_sensor->vector->min_pos != 1 ))
        {
            l_rc = VECTOR_UPDT_FAILURE;
            break;
        }

        /****************************************************/
         // test enabling vector element with min

        l_tempacc=l_sensor->accumulator;
        sensor_vector_elem_enable(l_sensor->vector,0,1);
        sensor_vector_update(l_sensor,0);

        if( (l_sensor->sample != 5) || (l_sensor->sample_min != 5 ) ||
            (l_sensor->sample_max != 11 ) ||
            (l_sensor->accumulator != l_tempacc+5)||
            (l_sensor->vector->min_pos != 0 ))
        {
            l_rc = VECTOR_UPDT_FAILURE9;
            break;
        }

        /****************************************************/
         // test updating vector element with max operation
        vectorSensor_t l_vecSnsr3;
        sensor_vectorize(l_sensor3,&l_vecSnsr3,VECTOR_OP_MAX);

        sensor_vector_elem_add(l_sensor3->vector,0,l_sensor1);
        sensor_vector_elem_add(l_sensor3->vector,1,l_sensor2);
        sensor_vector_elem_add(l_sensor3->vector,2,l_sensor);


        //so far the source sensors of vector sensor are:
        //   l_sensor1=11
        //   l_sensor2=12
        //   l_sensor=5
        /****************************************************/
         // test enabling vector element with invalid location
        sensor_vector_elem_enable(l_sensor3->vector,0xFF,0);
        sensor_vector_elem_enable(l_sensor3->vector,2,0);

        /****************************************************/
        // test updating vector element with max operation and
        // disabled element
        l_tempacc=l_sensor3->accumulator;
        sensor_vector_update(l_sensor3,0);

        if( (l_sensor3->sample != 12) || (l_sensor3->sample_min != 2 ) ||
            (l_sensor3->sample_max != 12 ) ||
            (l_sensor3->accumulator != l_tempacc+12)||
            (l_sensor3->vector->max_pos != 1))
        {
            l_rc = VECTOR_UPDT_FAILURE2;
            break;
        }

        l_tempacc=l_sensor3->accumulator;
        sensor_vector_elem_enable(l_sensor3->vector,2,1);
        sensor_vector_update(l_sensor3,0);

        if( (l_sensor3->sample != 12) || (l_sensor3->sample_min != 2 ) ||
            (l_sensor3->sample_max != 12 ) ||
            (l_sensor3->accumulator != l_tempacc+12)||
            (l_sensor3->vector->max_pos != 1))
        {
            l_rc = VECTOR_UPDT_FAILURE10;
            break;
        }

        vectorSensor_t l_vecSnsr1;
        /****************************************************/
        // test updating vector element with average operation
        sensor_vectorize(l_sensor1,&l_vecSnsr1,VECTOR_OP_AVG);
        sensor_vector_elem_add(l_sensor1->vector,0,l_sensor2);
        sensor_vector_elem_add(l_sensor1->vector,1,l_sensor);
        /****************************************************/

        //so far the source sensors of vector sensor are:
        //   l_sensor2=12
        //   l_sensor=5

        // test updating vector element with status reset bit set and
        // average op
        l_sensor1->status.reset = 1;
        sensor_vector_update(l_sensor1,11);
        if( (l_sensor1->sample != 12) || (l_sensor1->sample_min != 12 ) ||
            (l_sensor1->sample_max != 12 ) || (l_sensor1->accumulator != 12)||
            (l_sensor1->vector->max_pos != VECTOR_SENSOR_DEFAULT_VAL) ||
            (l_sensor1->vector->min_pos != VECTOR_SENSOR_DEFAULT_VAL))
        {
            l_rc = VECTOR_UPDT_FAILURE3;
            break;
        }

        /****************************************************/
        // test updating vector element with average operation and
        // disabled element
        l_tempacc=l_sensor1->accumulator;
        sensor_vector_elem_enable(l_sensor1->vector,0,0);
        sensor_vector_update(l_sensor1,2);

        if( (l_sensor1->sample != 5) || (l_sensor1->sample_min != 5 ) ||
            (l_sensor1->sample_max != 12 ) ||
            (l_sensor1->accumulator != l_tempacc+5))
        {
            l_rc = VECTOR_UPDT_FAILURE7;
            break;
        }

        sensor_vector_elem_enable(l_sensor1->vector,0,1);
        /****************************************************/
        // test updating vector element with average operation and
        // threshold greater than samples
        l_tempacc=l_sensor1->accumulator;
        sensor_vector_update(l_sensor1,25);

        if( (l_sensor1->sample != 5) || (l_sensor1->sample_min != 5 ) ||
            (l_sensor1->sample_max != 12 ) ||
            (l_sensor1->accumulator != l_tempacc) )
        {
            l_rc = VECTOR_UPDT_FAILURE4;
            break;
        }

        /****************************************************/
        // test updating vector element with average operation and
        // threshold lower than samples
        l_tempacc=l_sensor1->accumulator;
        sensor_vector_update(l_sensor1,2);
        if( (l_sensor1->sample != 8) || (l_sensor1->sample_min != 5 ) ||
            (l_sensor1->sample_max != 12 ) ||
            (l_sensor1->accumulator != l_tempacc+8))
        {
            l_rc = VECTOR_UPDT_FAILURE8;
            break;
        }

        l_sensor2->status.reset = 0;
        /****************************************************/
        // test update without vectorize should not do anything. Call should be
        // noop
        sensor_vector_update(l_sensor2,0);

        if( (l_sensor2->sample != 12))
        {
            l_rc = VECTOR_UPDT_FAILURE5;
            break;
        }

        vectorSensor_t l_vecSnsr2;
        /****************************************************/
        // test updating vector and vectorize with invalid operation
        // should not update anything
        sensor_vectorize(l_sensor2,&l_vecSnsr2,0xFF);

        sensor_vector_elem_add(l_sensor2->vector,0,l_sensor);
        sensor_vector_elem_add(l_sensor2->vector,1,l_sensor3);
        //so far the source sensors of vector sensor are:
        //   l_sensor=5
        //   l_sensor3=12
        sensor_vector_update(l_sensor2,0);

        if( (l_sensor2->sample != 12))
        {
            l_rc = VECTOR_UPDT_FAILURE6;
            break;
        }

    }while(0);

    if( (l_sensor != NULL) &&
        (l_sensor1 != NULL) &&
        (l_sensor2 != NULL) &&
        (l_sensor3 != NULL))
    {
        l_sensor->vector = NULL;
        l_sensor1->vector = NULL;
        l_sensor2->vector = NULL;
        l_sensor3->vector = NULL;
        l_sensor->mini_sensor = NULL;
        l_sensor1->mini_sensor = NULL;
        l_sensor2->mini_sensor = NULL;
        l_sensor3->mini_sensor = NULL;
    }

    return l_rc;
}


// Function Specification
//
// Name: callQuerySensorList
//
// Description: querySensorList: Will call the SenosrQeryList product applet
//
// End Function Specification
errlHndl_t callQuerySensorList(const uint16_t i_startGsid,
                           const uint8_t i_present,
                           const uint16_t i_type,
                           const uint16_t i_loc,
                           uint16_t * io_numOfSensors,
                           sensorQueryList_t * o_sensors,
                           sensor_info_t * o_sensorInfoPtrs
                           )
{
    OCC_APLT_STATUS_CODES l_status = OCC_APLT_SUCCESS;
    errlHndl_t l_errl = NULL;

    querySensorListAppletArg_t l_querySensorListAppletArg={
            i_startGsid,
            i_present,
            i_type,
            i_loc,
            io_numOfSensors,
            o_sensors,
            o_sensorInfoPtrs
    };


    //Call sensor query list applet
    runApplet(OCC_APLT_SNSR_QUERY,   // Applet enum Name
              &l_querySensorListAppletArg,                 // Applet arguments
              TRUE,                 // Blocking call?
              NULL,                 // Applet finished semaphore
              &l_errl,              // Error log handle
              &l_status);           // Error status

    return l_errl;

}


// Function Specification
//
// Name: printQuerySensor
//
// Description: printQuerySensor
//
// End Function Specification
void printQuerySensor( const uint16_t i_count,
                       sensorQueryList_t * i_sensors,
                       sensor_info_t * i_sensorInfos)
{
    int i;
    if(i_count!=0 &&
       ( (i_sensors!=NULL) ||
         (i_sensorInfos!=NULL) ) )
    {
        // Print sensorQueryList_t
        if( i_sensors != NULL )
        {
            SNSR_DBG("Printing sensorQueryList_t information");
            SNSR_DBG("*******************************");

            for(i=0;i<i_count;i++)
            {
                SNSR_DBG("ID:0x%4x  Value:%4d  Name:%s",i_sensors[i].gsid,
                         i_sensors[i].sample, i_sensors[i].name);
            }
            SNSR_DBG("*******************************\n");
        }

        // Print sensor_info_t
        if ( i_sensorInfos != NULL )
        {
            SNSR_DBG("Printing sensor_info_t information");
            SNSR_DBG("*******************************");

            for(i=0;i<i_count;i++)
            {
                SNSR_DBG("Name:%s", i_sensorInfos[i].name);
                SNSR_DBG("Frq:0x%4x", i_sensorInfos[i].sensor.freq);
                SNSR_DBG("Location:0x%4x", i_sensorInfos[i].sensor.location);
                SNSR_DBG("Number:0x%4x", i_sensorInfos[i].sensor.number);
                SNSR_DBG("Scalefactor:0x%4x", i_sensorInfos[i].sensor.scalefactor);
                SNSR_DBG("Type:0x%4x", i_sensorInfos[i].sensor.type);
                SNSR_DBG("Unit:%s", i_sensorInfos[i].sensor.units);
            }
            SNSR_DBG("*******************************\n");
        }
    }
}


// ifdef'd out b/c the sensor list is too big to test this in an applet
#if 0

// Function Specification
//
// Name: sensorTestQueryList
//
// Description: sensorTestQueryList
//
// End Function Specification
uint32_t sensorTestQueryList()
{
    uint32_t l_rc = SUCCESS_RC;
    uint16_t l_count = NUMBER_OF_SENSORS_IN_LIST;
    errlHndl_t l_err = NULL;

    do
    {
        uint16_t l_numOfSensors = 0;
        sensor_info_t l_sensorInfo;
        /****************************************************/
        // Test with invalid GSID. Must return error
        l_err = callQuerySensorList(0xFFFF,1,SENSOR_TYPE_ALL,SENSOR_LOC_ALL,
                                &l_numOfSensors,G_snsrList, NULL);

        if( NULL == l_err)
        {
            l_rc = QUERY_LIST_FAILURE;
            break;
        }
        else if( (l_err != INVALID_ERR_HNDL) &&
                 (l_err->iv_reasonCode != INTERNAL_FAILURE))
        {
            l_rc = QUERY_LIST_FAILURE12;
            break;

        }

        deleteErrl(&l_err);

        /****************************************************/
        // Test with NULL number of sensor pointer. Must return error
        l_err = callQuerySensorList(0,1,SENSOR_TYPE_ALL,SENSOR_LOC_ALL,
                                NULL,G_snsrList, NULL);

        if( NULL == l_err)
        {
            l_rc = QUERY_LIST_FAILURE2;
            break;
        }
        else if( (l_err != INVALID_ERR_HNDL) &&
                 (l_err->iv_reasonCode != INTERNAL_FAILURE))
        {
            l_rc = QUERY_LIST_FAILURE13;
            break;

        }

        deleteErrl(&l_err);

        /****************************************************/
        // Test with NULL sensor list pointer. Must return error
        l_err = callQuerySensorList(0,1,SENSOR_TYPE_ALL,SENSOR_LOC_ALL,
                                &l_numOfSensors,NULL, NULL);

        if( NULL == l_err)
        {
            l_rc = QUERY_LIST_FAILURE3;
            break;
        }
        else if( (l_err != INVALID_ERR_HNDL) &&
                 (l_err->iv_reasonCode != INTERNAL_FAILURE))
        {
            l_rc = QUERY_LIST_FAILURE14;
            break;

        }

        deleteErrl(&l_err);

        /****************************************************/
        // Query 0 # of sensors. Must return 0 sensors and no error
        l_err = callQuerySensorList(0,1,SENSOR_TYPE_ALL,SENSOR_LOC_ALL,
                                &l_numOfSensors,G_snsrList,NULL);

        if( (l_err != NULL) || (l_numOfSensors != 0))
        {
            l_rc = QUERY_LIST_FAILURE4;
            break;
        }

        /****************************************************/
        // Query All sensors except last one. Must return no error
        l_numOfSensors = l_count -1; //(-1) to leave out last sensor
        l_err = callQuerySensorList(0,1,SENSOR_TYPE_ALL,SENSOR_LOC_ALL,
                                &l_numOfSensors,G_snsrList, NULL);


        if( (l_err != NULL) || (l_numOfSensors > (l_count-1)))
        {
            l_rc = QUERY_LIST_FAILURE5;
            break;
        }

        /****************************************************/
        // Query last sensors. Must return no error. Trying to get
        // more sensors but should return only once since start GSID
        // is of last sensor
        l_numOfSensors = 5;
        sensor_t *l_sensor=getSensorByGsid(l_count-1);
        sensor_update(l_sensor, 10);

        l_err = callQuerySensorList(l_count-1,1,SENSOR_TYPE_ALL,
                        SENSOR_LOC_ALL,&l_numOfSensors,G_snsrList, NULL);

        if( (l_err != NULL) || (l_numOfSensors != 1) ||
            (l_sensor->gsid!=l_count-1))
        {
            l_rc = QUERY_LIST_FAILURE6;

            break;
        }

        /****************************************************/
        // Query sensors that are not present Must return no error.
        l_numOfSensors = 10;
        l_err = callQuerySensorList(PROBE250US0,0,SENSOR_TYPE_ALL,
                        SENSOR_LOC_ALL,&l_numOfSensors,G_snsrList, NULL);
        //printQuerySensor(l_numOfSensors,G_snsrList, NULL );
        if( (l_err != NULL) || (l_numOfSensors == 0))
        {
            l_rc = QUERY_LIST_FAILURE7;
            break;
        }

        /****************************************************/
        // Query sensors that are present, type and location
        // does not match any. Must return no error and no sensors
        l_numOfSensors = 10;
        l_err = callQuerySensorList(PROBE250US0,1,0x0,0x0,
                                    &l_numOfSensors,G_snsrList, NULL);
        printQuerySensor(l_numOfSensors,G_snsrList, NULL );

        if( (l_err != NULL) || (l_numOfSensors != 0))
        {
            l_rc = QUERY_LIST_FAILURE8;
            break;
        }

        /****************************************************/
        // Query sensors that are present,type=AMEC_SENSOR_TYPE_GENERIC,
        // loc=AMEC_SENSOR_LOC_OCC. Must return no error.
        l_numOfSensors = 10;
        l_err = callQuerySensorList(PROBE250US0,1,AMEC_SENSOR_TYPE_GENERIC,
                                    AMEC_SENSOR_LOC_OCC,
                                    &l_numOfSensors,G_snsrList, NULL);

        if( (l_err != NULL) || (l_numOfSensors == 0))
        {
            l_rc = QUERY_LIST_FAILURE9;
            break;
        }

        /****************************************************/
        // Query sensors that are present,type=AMEC_SENSOR_TYPE_GENERIC and
        // any location. Must return no error.
        l_numOfSensors = 10;
        l_err = callQuerySensorList(PROBE250US0,1,AMEC_SENSOR_TYPE_GENERIC,
                                    SENSOR_LOC_ALL,
                                    &l_numOfSensors,G_snsrList, NULL);

        if( (l_err != NULL) || (l_numOfSensors == 0))
        {
            l_rc = QUERY_LIST_FAILURE10;
            break;
        }

        /****************************************************/
        // Query sensors that are present,any type and
        // location=AMEC_SENSOR_LOC_OCC. Must return no error.
        l_numOfSensors = 10;
        l_err = callQuerySensorList(PROBE250US0,1,SENSOR_TYPE_ALL,
                                    AMEC_SENSOR_LOC_OCC,&l_numOfSensors,
                                    G_snsrList, NULL);

        if( (l_err != NULL) || (l_numOfSensors == 0))
        {
            l_rc = QUERY_LIST_FAILURE11;
            break;
        }

        /****************************************************/
        // Query sensors to get sensor info,any type and
        // location=AMEC_SENSOR_LOC_OCC. Must return no error.
        l_numOfSensors = 1;
        l_err = callQuerySensorList(PROBE250US0,1,SENSOR_TYPE_ALL,
                                    SENSOR_LOC_ALL,&l_numOfSensors,
                                    NULL, &l_sensorInfo);
        if( (l_err != NULL) || (l_numOfSensors == 0))
        {
            l_rc = QUERY_LIST_FAILURE11;
            break;
        }
        printQuerySensor(l_numOfSensors,NULL, &l_sensorInfo );

    }while(0);

    if( l_err != NULL)
    {
        deleteErrl(&l_err);
    }

    return l_rc;
}
#endif


/*****************************************************************************/
// Image Header
/*****************************************************************************/
// call macro with Applet ID arg
IMAGE_HEADER (G_sensorTestMain,sensorTestMain,SENSORTESTMAIN_ID,OCC_APLT_TEST);

