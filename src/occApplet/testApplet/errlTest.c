/******************************************************************************
// @file errlTest.c
// @brief OCC errl component test applet
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section errlTest.c ERRLTEST.c
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @at002             alvinwan  02/10/2012  Created
 *   @nh001             neilhsu   05/23/2012  Add missing error log tags 
 *   @at012  868019     alvinwan  01/25/2014  TRAC_get_buffer_partial() can result in TLB Miss Exception
 *   @jh001  881996     joshych   05/07/2013  Support SRAM error log format
 *   @jh003  890574     joshych   15/07/2013  Fix errlTest Applet
 *   @rt001  901927     tapiar    10/02/2013  Update error log to use unique module id
 *   @fk006  914801     fmkassem  01/05/2013  Remove wrong reference to a reasoncode
 *   @sb100  916174     sbroyles  02/18/2014  Remove rand.h include for ssx release release20140214
 *
 *  @endverbatim
 *
 *///*************************************************************************/

#define ERRL_DEBUG
/*****************************************************************************/
// Includes
/*****************************************************************************/
#include <common_types.h>   // imageHdr_t declaration and image header macro
#include "ssx.h"
#include "ssx_io.h"         // For ERRL_DBGs
#include <errl.h>
#include <appletId.h>       // For applet ID
// #include <rand.h> @sb100
#include <trac.h>           // For traces
#include <occ_service_codes.h> // Reason code
#include <cmdh_fsp.h>       // Needed for rc codes.
#include <trac_interface.h>
#include <aplt_service_codes.h>         // For test applet module ID
#include <testApltId.h>     // For test applet ID

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************
#define CHECK_CONDITION(cond, rv) \
    if( !(cond) ) \
    { \
        rv = __LINE__; \
        break; \
    }

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define ERRLTESTMAIN_ID  "errl Test\0"
#define TRAC_PATTERN 0x55
#define MAX_BUFFER_SIZE MAX_ERRL_CALL_HOME_SZ
#define TEST_MODULE_ID 0x1616                           // @nh001a

// sensor test module ID enumeration
typedef enum
{
    TEST_ERROR_HANDLING = 0x00,
    TEST_CREATE_COMMIT_DELETE_LOG = 0x01,
    TEST_ADD_USRDTLS_TO_ERRL = 0x02,
    TEST_ADD_TRACE_TO_ERRL = 0x03,
    TEST_TIME = 0x04,
    TEST_CREATE2INFO_CALLHOMELOG = 0x05,
    TEST_CREATE_MAX_LOGS = 0x06,
    TEST_CALLOUTS = 0x07,
    TEST_SET_ERRLSEV_TO_INFO = 0x08,
    TEST_ERRL_TEST_WORD_ALIGN = 0x09
} errlTestModId;

// errl test return codes
typedef enum
{
    SUCCESS_RC = 0x00000000,
} errlTestRc;
        
//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************
// TRACE: Trace buffers initialized
uint8_t G_data[ MAX_BUFFER_SIZE];
//*************************************************************************
// Function Prototypes
//*************************************************************************
uint32_t errlTestErrorHandling();
uint32_t errlTestAddUsrDtlsToErrl();
uint32_t errlTestAddTraceToErrl();
uint32_t errlTestDtlSizeLimit();
uint32_t errlTestTime();
uint32_t errlTestCreateCommitDeleteLog();
uint32_t errlTestCreate2InfoCallhomeLog();
uint32_t errlTestCreateMaxLogs();
uint32_t errlTestCallouts();
uint32_t errlTestSetErrlSevToInfo();
uint32_t errlTestWordAlign();
void dumpLog( errlHndl_t i_log, uint32_t i_len );
void ppdumpslot(void);

//*************************************************************************
// Functions
//*************************************************************************
// Function errlTestMain
//
// Name: sensorTestMain
//
// Description: Entry point function
//
// Flow:              FN=None
//
// End Function Specification
errlHndl_t errlTestMain(void * i_arg)
{

    errlHndl_t l_err = NULL;
    uint16_t l_modId = 0;
    uint32_t l_rc = ERRL_RC_SUCCESS;

    ERRL_DBG("Enter errlTestMain\n");

    do
    {
        l_rc = errlTestErrorHandling();
        l_modId = TEST_ERROR_HANDLING;

        if( l_rc != ERRL_RC_SUCCESS)
        {
            TRAC_INFO("Failure on error handling test");
            break;
        };

        l_rc = errlTestCreateCommitDeleteLog();
        l_modId = TEST_CREATE_COMMIT_DELETE_LOG ;
        if( l_rc != ERRL_RC_SUCCESS)
        {
            TRAC_INFO("Failure on Log test");
            break;
        }

        l_rc = errlTestAddUsrDtlsToErrl();
        l_modId = TEST_ADD_USRDTLS_TO_ERRL ;
        if( l_rc != ERRL_RC_SUCCESS)
        {
            TRAC_INFO("Failure on add user detail test");
            break;
        }

        l_rc = errlTestAddTraceToErrl();
        l_modId = TEST_ADD_TRACE_TO_ERRL ;
        if( l_rc != ERRL_RC_SUCCESS)
        {
            TRAC_INFO("Failure on add trace test");
            break;
        }

        l_rc = errlTestTime();
        l_modId = TEST_TIME ;
        if( l_rc != ERRL_RC_SUCCESS)
        {
            TRAC_INFO("Failure on time test");
            break;
        }

        l_rc = errlTestCreate2InfoCallhomeLog();
        l_modId = TEST_CREATE2INFO_CALLHOMELOG ;
        if( l_rc != ERRL_RC_SUCCESS)
        {
            TRAC_INFO("Failure on create call home log test");
            break;
        }

        l_rc = errlTestCreateMaxLogs();
        l_modId = TEST_CREATE_MAX_LOGS ;
        if( l_rc != ERRL_RC_SUCCESS)
        {
            TRAC_INFO("Failure on create max logs test");
            break;
        }

        l_rc = errlTestCallouts();
        l_modId = TEST_CALLOUTS ;
        if( l_rc != ERRL_RC_SUCCESS)
        {
            TRAC_INFO("Failure on callouts test");
            break;
        }

        l_rc = errlTestSetErrlSevToInfo();
        l_modId = TEST_SET_ERRLSEV_TO_INFO ;
        if( l_rc != ERRL_RC_SUCCESS)
        {
            TRAC_INFO("Failure on SetErrlSevToInfo test");
            break;
        }

        // @jh003c
        // comment out the test case since we no longer add the alignment in addUsrDtlsToErrl
        //l_rc = errlTestWordAlign(); 
        //l_modId = TEST_ERRL_TEST_WORD_ALIGN ;
        //if( l_rc != ERRL_RC_SUCCESS)
        //{
        //    TRAC_INFO("Failure on word alignment test");
        //    break;
        //}
    } while (0);

    if( l_rc != ERRL_RC_SUCCESS)
    {
        ERRL_DBG("**********************************************");
        ERRL_DBG("* errl Test Failed (errlTest.c): line: %d",l_rc);
        ERRL_DBG("**********************************************");
        /* @
         * @errortype
         * @moduleid       TEST_APLT_MODID_ERRLTEST
         * @reasoncode     INTERNAL_FAILURE
         * @userdata1      Test Applet ID
         * @userdata2      Return Code
         * @userdata4      OCC_NO_EXTENDED_RC
         * @devdesc        Failure executing test applet
         */
        l_err = createErrl(TEST_APLT_MODID_ERRLTEST,
                           INTERNAL_FAILURE,        // @nh001c
                           OCC_NO_EXTENDED_RC,
                           ERRL_SEV_INFORMATIONAL,
                           NULL,
                           0,
                           ERRL_TEST_APLT,
                           l_rc);
    }
    else
    {
        ERRL_DBG("**********************************************");
        ERRL_DBG("* errl Test Passed (errlTest.c)");
        ERRL_DBG("**********************************************");
    }

    ERRL_DBG("Exit errlTestMain\n");

    return l_err;
}


// Function Specification
//
// Name: errlTestErrorHandling
//
// Description: errlTestErrorHandling
//
// Flow:              FN=None
//
// End Function Specification
uint32_t errlTestErrorHandling()
{
    uint32_t l_rc = 0;
    errlHndl_t l_errlHnd = NULL;
    uint8_t l_dataPtr[10];
    uint16_t l_entrySizeBefore = 0;
    uint16_t l_entrySizeAfter = 0;

    ERRL_DBG(" START");
    do
    {
        /****************************************************/
        // Test createErrl  with incorrect parameter
        // Set ERRL_SEVERITY to 0x04, out of range so log won't be created
        l_errlHnd = createErrl(TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, 0x04, NULL, 0, 0x01, 0x02);        // @nh001c
        CHECK_CONDITION( l_errlHnd == INVALID_ERR_HNDL, l_rc);

         /****************************************************/
        // Test addTraceToErrl  with incorrect parameter
        // Create a log
        l_errlHnd = createErrl(TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, ERRL_SEV_PREDICTIVE, NULL, 0, 0x01, 0x02);        // @nh001c
        CHECK_CONDITION( l_errlHnd != INVALID_ERR_HNDL, l_rc);

        // i_trace = NULL, so entry size doesn't change
        l_entrySizeBefore = l_errlHnd->iv_userDetails.iv_entrySize;
        addTraceToErrl(NULL, 5, l_errlHnd);
        l_entrySizeAfter = l_errlHnd->iv_userDetails.iv_entrySize;
        CHECK_CONDITION(l_entrySizeBefore == l_entrySizeAfter, l_rc);

        // i_traceSz = 0, entry size doesn't change
        l_entrySizeBefore = l_errlHnd->iv_userDetails.iv_entrySize;
        addTraceToErrl(g_trac_inf, 0, l_errlHnd); // @at012c
        l_entrySizeAfter = l_errlHnd->iv_userDetails.iv_entrySize;
        CHECK_CONDITION( l_entrySizeBefore == l_entrySizeAfter, l_rc);

        // io_err = NULL, entry size doesn't change
        l_entrySizeBefore = l_errlHnd->iv_userDetails.iv_entrySize;
        addTraceToErrl(g_trac_inf, 32, NULL); // @at012c
        l_entrySizeAfter = l_errlHnd->iv_userDetails.iv_entrySize;
        CHECK_CONDITION( l_entrySizeBefore == l_entrySizeAfter, l_rc);

        // test addTraceToErrl after log is comitted so entry size doesn't change
        errlHndl_t l_errlHndx = l_errlHnd;
        commitErrl(&l_errlHnd);
        l_entrySizeBefore = l_errlHndx->iv_userDetails.iv_entrySize;
        addTraceToErrl(g_trac_inf, 32, l_errlHndx); // @at012c
        l_entrySizeAfter = l_errlHndx->iv_userDetails.iv_entrySize;
        CHECK_CONDITION( l_entrySizeBefore == l_entrySizeAfter, l_rc);

        deleteErrl(&l_errlHndx);
        CHECK_CONDITION( l_errlHndx == NULL, l_rc);

        // io_err = INVALID_ERR_HNDL
        // We are making sure that this function
        // handles a INVALID_ERR_HNDL being passed, and that we can't verify if
        // an error occured by checking anything. (It will just cause
        // a TLB exception)
        l_errlHnd = INVALID_ERR_HNDL;
        addTraceToErrl(g_trac_inf, 32, l_errlHnd); // @at012c

        /****************************************************/
        // Test commitErrl  with incorrect parameter
        // io_err = NULL
        // We are making sure that this function
        // handles a NULL being passed, and that we can't verify if
        // an error occured by checking anything. (It will just cause
        // a TLB exception)
        commitErrl( NULL);

        // l_errlHnd should be set to NULL
        l_errlHnd = INVALID_ERR_HNDL;
        commitErrl(&l_errlHnd);
        CHECK_CONDITION( l_errlHnd == NULL, l_rc);

        /****************************************************/
        // Test deleteErrl  with incorrect parameter
        // io_err = NULL
        // We are making sure that this function
        // handles a NULL being passed, and that we can't verify if
        // an error occured by checking anything. (It will just cause
        // a TLB exception)
        deleteErrl( NULL);

        // l_errlHnd should be set to NULL
        l_errlHnd = INVALID_ERR_HNDL;
        deleteErrl(&l_errlHnd);
        CHECK_CONDITION( l_errlHnd == NULL, l_rc);

        /****************************************************/
        // Test addCalloutToErrl  with incorrect parameter
        // Set io_err to NULL
        // We are making sure that this function
        // handles a NULL being passed, and that we can't verify if
        // an error occured by checking anything. (It will just cause
        // a TLB exception)
        addCalloutToErrl(NULL, ERRL_CALLOUT_TYPE_HUID, 0, ERRL_CALLOUT_PRIORITY_LOW); // @jh001c

        // Set io_err to INVALID_ERR_HNDL
        // We are making sure that this function
        // handles a INVALID_ERR_HNDL being passed, and that we can't verify if
        // an error occured by checking anything. (It will just cause
        // a TLB exception)
        addCalloutToErrl(INVALID_ERR_HNDL, ERRL_CALLOUT_TYPE_HUID, 0, ERRL_CALLOUT_PRIORITY_LOW); // @jh001c

        /****************************************************/
        // Test addUsrDtlsToErrl with incorrect parameter
        // Create a log
        l_errlHnd = createErrl(TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, ERRL_SEV_PREDICTIVE, NULL, 0, 0x01, 0x02);        // @nh001c
        CHECK_CONDITION( l_errlHnd != INVALID_ERR_HNDL, l_rc);

        // io_err = NULL
        // We are making sure that this function
        // handles a NULL being passed, and that we can't verify if
        // an error occured by checking anything. (It will just cause
        // a TLB exception)
        addUsrDtlsToErrl(NULL, l_dataPtr, 10, ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_TRACE_DATA);

        // io_err = INVALID_ERR_HNDL
        // We are making sure that this function
        // handles a INVALID_ERR_HNDL being passed, and that we can't verify if
        // an error occured by checking anything. (It will just cause
        // a TLB exception)
        addUsrDtlsToErrl(INVALID_ERR_HNDL, l_dataPtr, 10, ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_TRACE_DATA);

        // i_dataPtr = NULL so entry size doesn't change
        l_entrySizeBefore = l_errlHnd->iv_userDetails.iv_entrySize;
        addUsrDtlsToErrl(l_errlHnd, NULL, 10, ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_TRACE_DATA);
        l_entrySizeAfter = l_errlHnd->iv_userDetails.iv_entrySize;
        CHECK_CONDITION( l_entrySizeBefore == l_entrySizeAfter, l_rc);

        // i_size = 0 so so entry size doesn't change
        l_entrySizeBefore = l_errlHnd->iv_userDetails.iv_entrySize;
        addUsrDtlsToErrl(l_errlHnd, l_dataPtr, 0, ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_TRACE_DATA);
        l_entrySizeAfter = l_errlHnd->iv_userDetails.iv_entrySize;
        CHECK_CONDITION( l_entrySizeBefore == l_entrySizeAfter, l_rc);

        // test addUsrDtlsToErrl after log is committed so entry size doesn't change
        l_errlHndx = l_errlHnd;
        commitErrl(&l_errlHnd);
        l_entrySizeBefore = l_errlHndx->iv_userDetails.iv_entrySize;
        addUsrDtlsToErrl(l_errlHndx, l_dataPtr, 10, ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_TRACE_DATA);
        l_entrySizeAfter = l_errlHndx->iv_userDetails.iv_entrySize;
        CHECK_CONDITION( l_entrySizeBefore == l_entrySizeAfter, l_rc);

        deleteErrl(&l_errlHndx);
        CHECK_CONDITION( l_errlHndx == NULL, l_rc);

        /****************************************************/
        // Test setErrlSevToInfo  with incorrect parameter
        // Set io_err to NULL.
        // We are making sure that this function
        // handles a NULL being passed, and that we can't verify if
        // an error occured by checking anything. (It will just cause
        // a TLB exception)
        setErrlSevToInfo(NULL);

        // Set io_err to INVALID_ERR_HNDL
        // We are making sure that this function
        // handles a INVALID_ERR_HNDL being passed, and that we can't verify if
        // an error occured by checking anything. (It will just cause
        // a TLB exception)
        setErrlSevToInfo(INVALID_ERR_HNDL);
    }while(0);

    return l_rc;
}

// Function Specification
//
// Name: errlTestCreateCommitDeleteLog
//
// Description: errlTestCreateCommitDeleteLog
//
// Flow:              FN=None
//
// End Function Specification
uint32_t errlTestCreateCommitDeleteLog()
{
    ERRL_DBG("START");
    uint32_t l_rc = 0;

    do
    {
        /****************************************************/
        // Test create log
        errlHndl_t l_handle = NULL;
        l_handle = createErrl( TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, ERRL_SEV_CALLHOME_DATA, g_trac_inf, 512, 0x1, 0x2);        // @nh001c @at012c
        CHECK_CONDITION( l_handle != INVALID_ERR_HNDL, l_rc);

        ERRL_DBG("Slots after Creating call home log" );
        ppdumpslot();

        /****************************************************/
        // Test commit log
        errlHndl_t l_handle2 = l_handle;
        commitErrl( &l_handle );
        CHECK_CONDITION( (l_handle == NULL) &&
                         (l_handle2->iv_userDetails.iv_committed == 1), l_rc);

        ERRL_DBG("Slots after Commiting call home log" );
        dumpLog( l_handle2, l_handle2->iv_userDetails.iv_entrySize );
        ppdumpslot();

        /****************************************************/
        // Test delete log
        deleteErrl(&l_handle2);
        CHECK_CONDITION( l_handle2 == NULL, l_rc);

        ERRL_DBG("Slots after delete Log" );
        ppdumpslot();

        ERRL_DBG("END \n");

    }while(0);

    return l_rc;
}

// Function Specification
//
// Name: errlTestAddUsrDtlsToErrl
//
// Description: errlTestAddUsrDtlsToErrl
//
// Flow:              FN=None
//
// End Function Specification
uint32_t errlTestAddUsrDtlsToErrl()
{
    uint32_t l_rc = 0;
    ERRL_DBG("START");
    uint16_t l_entrySizeBefore = 0;
    uint16_t l_entrySizeAfter = 0;

    do
    {
        // Create three err logs
        errlHndl_t l_handle = NULL;
        errlHndl_t l_handle2 = NULL;
        errlHndl_t l_handle3 = NULL;

        l_handle = createErrl( TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, ERRL_SEV_UNRECOVERABLE, NULL, 512, 0x1, 0x2);        // @nh001c
        l_handle2 = createErrl( TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, ERRL_SEV_CALLHOME_DATA, NULL, 512, 0x1, 0x2);        // @nh001c
        l_handle3 = createErrl( TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, ERRL_SEV_INFORMATIONAL, NULL, 512, 0x1, 0x2);        // @nh001c

        // l_handle will set to NULL after calling the commitErrl, so we need to store it
        errlHndl_t l_handleX = l_handle;
        errlHndl_t l_handle2X = l_handle2;
        errlHndl_t l_handle3X = l_handle3;
        ERRL_DBG("Slots after Create - 3 slots should be used (one of each");
        ppdumpslot();

        CHECK_CONDITION( (l_handle != INVALID_ERR_HNDL) &&
                         (l_handle2 != INVALID_ERR_HNDL) &&
                         (l_handle3 != INVALID_ERR_HNDL), l_rc);

        /****************************************************/
        // Test sizelimit for addUsrDtlsToErrl
        // Add "user details" data that exceeds the max size for l_handle
        l_entrySizeBefore = l_handle->iv_userDetails.iv_entrySize;
        memset( G_data, 0xCC, sizeof( G_data ) );
        addUsrDtlsToErrl( l_handle, G_data, sizeof( G_data ), ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_TRACE_DATA );
        l_entrySizeAfter = l_handle->iv_userDetails.iv_entrySize;
        CHECK_CONDITION( l_entrySizeAfter == MAX_ERRL_ENTRY_SZ, l_rc);

        // Add "user details" data that exceeds the max size for l_handle2
        l_entrySizeBefore = l_handle2->iv_userDetails.iv_entrySize;
        memset( G_data, 0xDD, sizeof( G_data ) );
        addUsrDtlsToErrl( l_handle2, G_data, sizeof( G_data ), ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_CALLHOME_DATA );
        l_entrySizeAfter = l_handle2->iv_userDetails.iv_entrySize;
        CHECK_CONDITION( l_entrySizeAfter == MAX_ERRL_CALL_HOME_SZ, l_rc);

        // Add "user details" with size 76 for l_handle3
        l_entrySizeBefore = l_handle3->iv_userDetails.iv_entrySize;
        memset( G_data, 0xEE, sizeof( G_data ) );
        addUsrDtlsToErrl( l_handle3, G_data, 76, ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_TRACE_DATA );
        l_entrySizeAfter = l_handle3->iv_userDetails.iv_entrySize;
        // (header + 76) is the size that add to entry
        CHECK_CONDITION( l_entrySizeAfter == (l_entrySizeBefore+sizeof(ErrlUserDetailsEntry_t)+76), l_rc);

        dumpLog( l_handle, l_handle->iv_userDetails.iv_entrySize );
        dumpLog( l_handle2, l_handle2->iv_userDetails.iv_entrySize );
        dumpLog( l_handle3, l_handle3->iv_userDetails.iv_entrySize );

        commitErrl( &l_handle );
        commitErrl( &l_handle2 );
        commitErrl( &l_handle3 );
        ERRL_DBG("Slots after Commit -  3 slots should be used/committed");
        ppdumpslot();

        deleteErrl(&l_handleX);
        deleteErrl(&l_handle2X);
        deleteErrl(&l_handle3X);
        CHECK_CONDITION( (l_handleX == NULL) &&
                         (l_handle2X == NULL) &&
                         (l_handle3X == NULL), l_rc);

        ERRL_DBG("Slots after delete Log - All slots should be empty");
        ppdumpslot();

        /****************************************************/
        // Test sizelimit for addUsrDtlsToErrl with continuous calls
        // Create log with 512 bytes trace
        l_handle = createErrl( TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, ERRL_SEV_PREDICTIVE, g_trac_inf, 512, 0x1, 0x2);        // @nh001c @at012c
        CHECK_CONDITION( l_handle != INVALID_ERR_HNDL, l_rc);

        // l_handle will set to NULL after calling the commitErrl, so we need to store it
        l_handleX = l_handle;
        ppdumpslot();

        // add 256 bytes of "user details" (512+256)
        l_entrySizeBefore = l_handle->iv_userDetails.iv_entrySize;
        memset( G_data, 0xAA, sizeof( G_data ) );
        addUsrDtlsToErrl( l_handle, G_data, 256, ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_TRACE_DATA );
        l_entrySizeAfter = l_handle->iv_userDetails.iv_entrySize;
        ERRL_DBG("Slots after create + 256 bytes" );
        ppdumpslot();
        // (header + 256) is the size that add to entry
        CHECK_CONDITION( l_entrySizeAfter == (l_entrySizeBefore+sizeof(ErrlUserDetailsEntry_t)+256), l_rc);

        // add 512 bytes of "user details" (512+256+512)
        l_entrySizeBefore = l_handle->iv_userDetails.iv_entrySize;
        memset( G_data, 0xBB, sizeof( G_data ) );
        addUsrDtlsToErrl( l_handle, G_data, 512, ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_TRACE_DATA );
        l_entrySizeAfter = l_handle->iv_userDetails.iv_entrySize;
        ERRL_DBG("Slots after create + 256 + 512 bytes");
        ppdumpslot();
        // (header + 512) is the size that add to entry
        CHECK_CONDITION( l_entrySizeAfter == (l_entrySizeBefore+sizeof(ErrlUserDetailsEntry_t)+512), l_rc);

        // add 1024 bytes of "user details" (512+256+512+1024), the entry size is more than 2048 now
        l_entrySizeBefore = l_handle->iv_userDetails.iv_entrySize;
        memset( G_data, 0xCC, sizeof( G_data ) );
        addUsrDtlsToErrl( l_handle, G_data, 1024, ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_TRACE_DATA );
        l_entrySizeAfter = l_handle->iv_userDetails.iv_entrySize;
        ERRL_DBG("Slots after create + 256 + 512 +1024 bytes");
        ppdumpslot();
        // (header + 1024) is the size that add to entry
        CHECK_CONDITION( l_entrySizeAfter <= MAX_ERRL_ENTRY_SZ, l_rc); // @at012c

        commitErrl( &l_handle );
        deleteErrl(&l_handleX);
        ERRL_DBG("Slots should now be empty");
        ppdumpslot();
        ERRL_DBG("END \n");
    }while(0);

    return l_rc;
}

// Function Specification
//
// Name: errlTestAddTraceToErrl
//
// Description: errlTestAddTraceToErrl
//
// Flow:              FN=None
//
// End Function Specification
uint32_t errlTestAddTraceToErrl()
{
    uint32_t l_rc = 0;
    uint16_t l_entrySizeBefore = 0;
    uint16_t l_entrySizeAfter = 0;
    ERRL_DBG("START");

    do
    {
        // Create one err log
        errlHndl_t l_handle = NULL;
        l_handle = createErrl( TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, ERRL_SEV_PREDICTIVE, NULL, 512, 0x1, 0x2);        // @nh001c
        CHECK_CONDITION( l_handle != INVALID_ERR_HNDL, l_rc);

        // l_handle will set to NULL after calling the commitErrl, so we need to store it
        errlHndl_t l_handleX = l_handle;
        ERRL_DBG("Slots after Create - 1 slots should be used (one of each");
        ppdumpslot();

        /****************************************************/
        // Test sizelimit for addTraceToErrl
        // Add "trace" data that exceeds the max size
        l_entrySizeBefore = l_handle->iv_userDetails.iv_entrySize;
        addTraceToErrl(g_trac_inf, MAX_BUFFER_SIZE, l_handle); // @at012c
        l_entrySizeAfter = l_handle->iv_userDetails.iv_entrySize;
        CHECK_CONDITION( l_entrySizeAfter <= MAX_ERRL_ENTRY_SZ, l_rc); // @at012c

        dumpLog( l_handle, l_handle->iv_userDetails.iv_entrySize );
        commitErrl( &l_handle );
        ERRL_DBG("Slots after Commit -  1 slots should be used/committed");
        ppdumpslot();

        deleteErrl(&l_handleX);
        ERRL_DBG("Slots after delete Log - All slots should be empty");
        ppdumpslot();

        /****************************************************/
        // Test sizelimit for addTraceToErrl with continuous calls
        // Create log with 512 bytes trace
        l_handle = createErrl( TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, ERRL_SEV_PREDICTIVE, g_trac_inf, 512, 0x1, 0x2);        // @nh001c @at012c
        CHECK_CONDITION( l_handle != INVALID_ERR_HNDL, l_rc);

        // l_handle will set to NULL after calling the commitErrl, so we need to store it
        l_handleX = l_handle;
        ppdumpslot();

        // Add 256 bytes of trace (512+256)
        l_entrySizeBefore = l_handle->iv_userDetails.iv_entrySize;
        addTraceToErrl(g_trac_inf, 256, l_handle); // @at012c
        l_entrySizeAfter = l_handle->iv_userDetails.iv_entrySize;
        ERRL_DBG("Slots after create + 256 bytes" );
        ppdumpslot();
        // (header + 256) is the size that add to entry
        CHECK_CONDITION( l_entrySizeAfter <= (l_entrySizeBefore+sizeof(ErrlUserDetailsEntry_t)+256), l_rc); // @at012c

        // Add 512 bytes of trace (512+256+512)
        l_entrySizeBefore = l_handle->iv_userDetails.iv_entrySize;
        addTraceToErrl(g_trac_inf, 512, l_handle); // @at012c
        l_entrySizeAfter = l_handle->iv_userDetails.iv_entrySize;
        ERRL_DBG("Slots after create + 256 + 512 bytes");
        ppdumpslot();
        CHECK_CONDITION( l_entrySizeAfter <= (l_entrySizeBefore+sizeof(ErrlUserDetailsEntry_t)+512), l_rc); // @at012c

        // Add 1024 bytes of trace (512+256+512+1024), the entry size is more than 2048 now
        l_entrySizeBefore = l_handle->iv_userDetails.iv_entrySize;
        addTraceToErrl(g_trac_inf, 1024, l_handle); // @at012c
        l_entrySizeAfter = l_handle->iv_userDetails.iv_entrySize;
        ERRL_DBG("Slots after create + 256 + 512 bytes");
        ppdumpslot();
        CHECK_CONDITION( l_entrySizeAfter <= MAX_ERRL_ENTRY_SZ, l_rc); // @at012c

        commitErrl( &l_handle );
        deleteErrl(&l_handleX);
        ERRL_DBG("Slots should now be empty");
        ppdumpslot();
        ERRL_DBG("END \n");

    }while(0);

    return l_rc;
}

// Function Specification
//
// Name: errlTestTime
//
// Description: errlTestTime
//
// Flow:              FN=None
//
// End Function Specification
uint32_t errlTestTime()
{
    uint32_t l_rc = 0;

    do
    {
        ERRL_DBG("START");
        errlHndl_t l_handle = NULL;
        uint64_t l_start = 0;
        uint64_t l_end = 0;


        /****************************************************/
        // Check timeStamp
        // Create one log
        l_start = ssx_timebase_get();
        l_handle = createErrl( 0x1716, 0x08, OCC_NO_EXTENDED_RC, ERRL_SEV_CALLHOME_DATA, g_trac_inf, 128, 0x1, 0x2);        // @nh001c @at012c
        CHECK_CONDITION( l_handle != INVALID_ERR_HNDL, l_rc);


        // check time stamp
        errlHndl_t l_handle2 = l_handle;
        commitErrl( &l_handle );
        l_end = ssx_timebase_get();
        CHECK_CONDITION( (l_handle2->iv_userDetails.iv_timeStamp >= l_start) &&
                         (l_handle2->iv_userDetails.iv_timeStamp <= l_end ), l_rc);

        deleteErrl(&l_handle2);
        ERRL_DBG("END \n");

    }while(0);

    return l_rc;
}

/*****************************************************************************/
// errlTestCreate2InfoCallhomeLog
/*****************************************************************************/
uint32_t errlTestCreate2InfoCallhomeLog()
{
    ERRL_DBG("START" );
    uint32_t l_rc = 0;

    do
    {
        /****************************************************/
        // Check creating Info logs twice
        // Create first Info log
        errlHndl_t l_handle = NULL;
        errlHndl_t l_handle2= NULL;
        l_handle = createErrl( TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, ERRL_SEV_INFORMATIONAL,g_trac_inf, 32, 0x1, 0x2);        // @nh001c @at012c
        CHECK_CONDITION( l_handle != INVALID_ERR_HNDL, l_rc);

        // Create second Info log and it should fail
        l_handle2 = createErrl( 0x2727, 0x19, OCC_NO_EXTENDED_RC, ERRL_SEV_INFORMATIONAL, g_trac_inf, 32, 0x2, 0x3);        // @nh001c @at012c
        CHECK_CONDITION( l_handle2 == INVALID_ERR_HNDL, l_rc);

        deleteErrl(&l_handle);

        /****************************************************/
        // Check creating Callhome logs twice
        // Create first Callhome log
        l_handle = NULL;
        l_handle2= NULL;
        l_handle = createErrl( TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, ERRL_SEV_CALLHOME_DATA,g_trac_inf, 32, 0x1, 0x2);        // @nh001c @at012c
        CHECK_CONDITION( l_handle != INVALID_ERR_HNDL, l_rc);

        // Create second Callhome log and it should fail
        l_handle2 = createErrl( 0x2727, 0x19, OCC_NO_EXTENDED_RC, ERRL_SEV_CALLHOME_DATA, g_trac_inf, 32, 0x2, 0x3);        // @nh001c @at012c
        CHECK_CONDITION( l_handle2 == INVALID_ERR_HNDL, l_rc);

        deleteErrl(&l_handle);

        ERRL_DBG("END \n");
    }while(0);

    return l_rc;
}

// Function Specification
//
// Name: errlTestCreateMaxLogs
//
// Description: errlTestCreateMaxLogs
//
// Flow:              FN=None
//
// End Function Specification
uint32_t errlTestCreateMaxLogs()
{
    uint32_t l_rc = 0;

    ERRL_DBG("START");
    do
    {

        /****************************************************/
        // Check max logs
        ERRL_SEVERITY l_sev = 0;
        errlHndl_t l_backupHandle[ERRL_MAX_SLOTS-2];
        errlHndl_t l_handle = NULL;

        uint32_t l_index = 0;
        // Create 7 ERRL_SEV_PREDICTIVE or ERRL_SEV_UNRECOVERABLE slots randomly
        for(l_index =0; l_index < ERRL_MAX_SLOTS-2; l_index++)
        {

            uint64_t l_time = ssx_timebase_get();
            l_sev = l_time%2 ? ERRL_SEV_PREDICTIVE : ERRL_SEV_UNRECOVERABLE;
            l_handle = createErrl( TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, l_sev, g_trac_inf, 512, 0x1, l_index);        // @nh001c @at012c
            CHECK_CONDITION( (l_handle != INVALID_ERR_HNDL) &&
                             (l_handle != NULL), l_rc);

            // backup handle
            l_backupHandle[l_index] = l_handle;

            ERRL_DBG("Log Created @ %p with Sev: %d\n",l_handle, l_sev );
            // addUsrDtlsToErrl
            memset( G_data, l_index, sizeof( G_data ) );
            addUsrDtlsToErrl( l_handle, G_data, sizeof(G_data), ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_TRACE_DATA );

            // commitErrl( &l_handle );
        }
        // check if something wrong in for loop
        if(l_rc != 0)
        	break;

        // Create one more and it should fail
        l_handle = createErrl( TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, l_sev, g_trac_inf, 512, 0x1, l_index);        // @nh001c @at012c
        CHECK_CONDITION( l_handle == INVALID_ERR_HNDL, l_rc);

        // delete errl
        for(l_index = 0; l_index < ERRL_MAX_SLOTS-2; l_index++)
        {
            deleteErrl(&l_backupHandle[l_index]);
        }
        ppdumpslot();

        /****************************************************/
        // Check log id overflow
         for(l_index = 0; l_index < 256; l_index++)
        {
            l_handle = createErrl( TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, l_sev, g_trac_inf, 512, 0x1, l_index);        // @nh001c @at012c
            CHECK_CONDITION( (l_handle != INVALID_ERR_HNDL) &&
                             (l_handle != NULL), l_rc);

            deleteErrl(&l_handle);
        }

        ERRL_DBG("END \n");
    }while(0);

    return l_rc;
}

// Function Specification
//
// Name: errlTestCallouts
//
// Description: errlTestCallouts
//
// Flow:              FN=None
//
// End Function Specification
// @jh001c
uint32_t errlTestCallouts()
{
    uint32_t l_rc = 0;
    ERRL_DBG("START");

    do
    {
        errlHndl_t l_handle = NULL;
        ERRL_DBG("--------------------------------\n");

        /****************************************************/
        // Check max callouts
        l_handle = createErrl( TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, ERRL_SEV_PREDICTIVE,g_trac_inf, 128, 0x1, 0x2);        // @nh001c @at012c
        CHECK_CONDITION( l_handle != INVALID_ERR_HNDL, l_rc);

        ERRL_CALLOUT_PRIORITY l_array[8] = {
                                             ERRL_CALLOUT_PRIORITY_HIGH,
                                             ERRL_CALLOUT_PRIORITY_MED,
                                             ERRL_CALLOUT_PRIORITY_LOW,
                                             ERRL_CALLOUT_PRIORITY_HIGH,
                                             ERRL_CALLOUT_PRIORITY_MED,
                                             ERRL_CALLOUT_PRIORITY_MED,
                                             ERRL_CALLOUT_PRIORITY_LOW,
                                             ERRL_CALLOUT_PRIORITY_LOW,
                                            };

        ERRL_CALLOUT_TYPE l_type[8] = {
                                        ERRL_CALLOUT_TYPE_HUID,
                                        ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                        ERRL_CALLOUT_TYPE_HUID,
                                        ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                        ERRL_CALLOUT_TYPE_HUID,
                                        ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                        ERRL_CALLOUT_TYPE_HUID,
                                        ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                       };

        // add 6 (ERRL_MAX_CALLOUTS) callouts
        uint8_t l_index = 0;
        for(l_index = 0; l_index < ERRL_MAX_CALLOUTS; l_index++)
        {
            ERRL_DBG("current callouts %d attempting to add callout # %d with type %d ,priority %d", l_handle->iv_numCallouts, l_index, l_type[l_index], l_array[l_index] );
            addCalloutToErrl(l_handle,l_type[l_index],l_index,l_array[l_index]);
        }
        CHECK_CONDITION( l_handle->iv_numCallouts == ERRL_MAX_CALLOUTS, l_rc);

        // add one more callout and it should fail
        addCalloutToErrl(l_handle,l_type[0],l_index,l_array[0]);
        CHECK_CONDITION( l_handle->iv_numCallouts == ERRL_MAX_CALLOUTS, l_rc);

        dumpLog( l_handle, l_handle->iv_userDetails.iv_entrySize );
        deleteErrl( &l_handle );
        ppdumpslot();

        /****************************************************/
        // Check callouts after errl is committed
        // Create log
        l_handle = createErrl( TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, ERRL_SEV_PREDICTIVE,g_trac_inf, 32, 0x1, 0x2);        // @nh001c @at012c
        errlHndl_t l_log = l_handle;
        CHECK_CONDITION( l_handle != INVALID_ERR_HNDL, l_rc);

        // Commit log and add callout. But adding callout should fail
        commitErrl( &l_handle );
        addCalloutToErrl(l_handle,l_type[0],0,l_array[0]);
        CHECK_CONDITION( l_log->iv_numCallouts == ERRL_MAX_CALLOUTS, l_rc); // @jh003c

        deleteErrl(&l_log);

        /****************************************************/
        // Check addCalloutToErrl for ERRL_SEV_INFORMATIONAL log
        // Create ERRL_SEV_INFORMATIONAL log
        l_handle = createErrl( TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, ERRL_SEV_INFORMATIONAL,g_trac_inf, 128, 0x1, 0x2);        // @nh001c @at012c
        CHECK_CONDITION( l_handle != INVALID_ERR_HNDL, l_rc);
        if(l_handle == INVALID_ERR_HNDL)

        // add one callout and it should fail
        addCalloutToErrl(l_handle,l_type[0],l_index,l_array[0]);
        CHECK_CONDITION( l_handle->iv_numCallouts == 0, l_rc);

        dumpLog( l_handle, l_handle->iv_userDetails.iv_entrySize );
        deleteErrl( &l_handle );
        ppdumpslot();

        ERRL_DBG("END \n");
    }while(0);

    return l_rc;
}

// Function Specification
//
// Name: errlTestSetErrlSevToInfo
//
// Description: errlTestSetErrlSevToInfo
//
// Flow:              FN=None
//
// End Function Specification
uint32_t errlTestSetErrlSevToInfo()
{
    uint32_t l_rc = 0;
    ERRL_DBG("START");

    do
    {
        errlHndl_t l_handle = NULL;

        /****************************************************/
        // Check setErrlSevToInfo
        // Create ERRL_SEV_PREDICTIVE log
        l_handle = createErrl( TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, ERRL_SEV_PREDICTIVE,g_trac_inf, 128, 0x1, 0x2);        // @nh001c @at012c
        CHECK_CONDITION( l_handle != INVALID_ERR_HNDL, l_rc);

        // Add callout
        addCalloutToErrl(l_handle,ERRL_CALLOUT_TYPE_HUID,0x00,ERRL_CALLOUT_PRIORITY_LOW); // @jh001c
        CHECK_CONDITION( l_handle->iv_numCallouts == 1, l_rc);

        // Call setErrlSevToInfo. Callouts within log should be cleared and
        // iv_severity should be set to ERRL_SEV_INFORMATIONAL
        setErrlSevToInfo(l_handle);
        CHECK_CONDITION( (l_handle->iv_numCallouts == 0) &&
                         (l_handle->iv_severity == ERRL_SEV_INFORMATIONAL), l_rc);

        deleteErrl( &l_handle );
        ppdumpslot();

        /****************************************************/
        // Check setErrlSevToInfo after errl is committed
        // Create log
        l_handle = createErrl( TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, ERRL_SEV_PREDICTIVE,g_trac_inf, 128, 0x1, 0x2);        // @nh001c @at012c
        CHECK_CONDITION( l_handle != INVALID_ERR_HNDL, l_rc);

        errlHndl_t l_log = l_handle;

        // Add callout
        addCalloutToErrl(l_handle,ERRL_CALLOUT_TYPE_HUID,0x00,ERRL_CALLOUT_PRIORITY_LOW); // @jh001c
        CHECK_CONDITION( l_handle->iv_numCallouts == 1, l_rc);

        // Commit log and call setErrlSevToInfo. But setErrlSevToInfo will do nothing
        commitErrl( &l_handle );
        setErrlSevToInfo(l_handle);
        CHECK_CONDITION( (l_log->iv_numCallouts == ERRL_MAX_CALLOUTS) && // @jh003c
                         (l_log->iv_severity == ERRL_SEV_PREDICTIVE), l_rc);

        deleteErrl(&l_log);
        ERRL_DBG("END \n");

    }while(0);

    return l_rc;
}

// Function Specification
//
// Name: errlTestWordAlign
//
// Description: errlTestWordAlign
//
// Flow:              FN=None
//
// End Function Specification
uint32_t errlTestWordAlign()
{
    uint32_t l_rc = 0;
    uint16_t l_entrySizeBefore = 0;
    uint16_t l_entrySizeAfter = 0;
    ERRL_DBG("START");

    do
    {
        /****************************************************/
        // Test word align for addUsrDtlsToErrl
        // Create log
        errlHndl_t l_handle = createErrl( TEST_MODULE_ID, 0x08, OCC_NO_EXTENDED_RC, ERRL_SEV_PREDICTIVE, NULL, 0, 0x1, 0x2);        // @nh001c
        CHECK_CONDITION( l_handle != INVALID_ERR_HNDL, l_rc);

        // l_handle will set to NULL after calling the commitErrl, so we need to store it
        errlHndl_t l_handleX = l_handle;
        ppdumpslot();

        // add 13 bytes of "user details"
        l_entrySizeBefore = l_handle->iv_userDetails.iv_entrySize;
        memset( G_data, 0xAA, sizeof( G_data ) );
        addUsrDtlsToErrl( l_handle, G_data, 13, ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_TRACE_DATA );
        l_entrySizeAfter = l_handle->iv_userDetails.iv_entrySize;
        ERRL_DBG("Slots after create + 13 bytes" );
        ppdumpslot();
        // (header + WORDALIGN(13)) is the size that add to entry
        CHECK_CONDITION( l_entrySizeAfter == (l_entrySizeBefore+sizeof(ErrlUserDetailsEntry_t)+16), l_rc);

        /****************************************************/
        // Test word align for addTraceToErrl
        // add 21 bytes of trace
        l_entrySizeBefore = l_handle->iv_userDetails.iv_entrySize;
        addTraceToErrl(g_trac_inf, 21, l_handle); // @at012c
        l_entrySizeAfter = l_handle->iv_userDetails.iv_entrySize;
        ERRL_DBG("Slots after create + 21 bytes" );
        ppdumpslot();
        // (header + WORDALIGN(21)) is the size that add to entry
        CHECK_CONDITION( l_entrySizeAfter <= (l_entrySizeBefore+sizeof(ErrlUserDetailsEntry_t)+24), l_rc); // @at012c

        commitErrl( &l_handle );
        deleteErrl(&l_handleX);
        ERRL_DBG("Slots should now be empty");
        ppdumpslot();
        ERRL_DBG("END \n");
    }while(0);

    return l_rc;
}


// Function Specification
//
// Name: dumpLog
//
// Description: dumpLog
//
// Flow:              FN=None
//
// End Function Specification
void dumpLog( errlHndl_t i_log, uint32_t i_len )
{
    uint32_t    l_written = 0;
    uint32_t    l_counter = 0;
    uint8_t *   l_data = (uint8_t*) i_log;

    printf("----------%p---------- \n", i_log );

    if ( i_log == NULL )
        return;

    while ( l_counter < i_len)
    {
        printf("|   %08X     ", (uint32_t) l_data + l_counter);

        // Display 16 bytes in Hex with 2 spaces in between
        l_written = 0;
        uint8_t i = 0;
        for ( i = 0; i < 16 && l_counter < i_len; i++ )
        {
            l_written += printf("%02X",l_data[l_counter++]);

            if ( ! ( l_counter % 4 ) )
            {
                l_written += printf("  ");
            }
        }

        // Pad with spaces
        uint8_t l_space[64] = {0};
        memset( l_space, 0x00, sizeof( l_space ));
        memset( l_space, ' ', 43-l_written);
        printf("%s", l_space );

        // Display ASCII
        l_written = 0;
        uint8_t l_char = 0;
        for ( ; i > 0 ; i-- )
        {
            l_char = l_data[ l_counter-i ];

            if ( isprint( l_char ) &&
                 ( l_char != '&' ) &&
                 ( l_char != '<' ) &&
                 ( l_char != '>' )
               )
            {
                l_written += printf("%c",l_char );
            }
            else
            {
                l_written += printf("." );
            }
        }

        // Pad with spaces
        uint8_t l_space2[64] = {0};
        memset( l_space2, 0x00, sizeof( l_space2 ));
        memset( l_space2, ' ', 19-l_written);
        printf("%s\n", l_space2 );
    }
   printf("----------%p---------- \n", i_log );

}

// Function Specification
//
// Name: ppdumpslot
//
// Description: ppdumpslot
//
// Flow:              FN=None
//
// End Function Specification
void ppdumpslot(void)
{
    errlHndl_t l_array[ERRL_MAX_SLOTS] = {
                            (errlHndl_t)G_errslot1,
                            (errlHndl_t)G_errslot2,
                            (errlHndl_t)G_errslot3,
                            (errlHndl_t)G_errslot4,
                            (errlHndl_t)G_errslot5,
                            (errlHndl_t)G_errslot6,
                            (errlHndl_t)G_errslot7,
                            (errlHndl_t)G_infoslot,
                            (errlHndl_t)G_callslot,
                            };


    printf("-------- \n");

    uint8_t l_index = 0;
    for(l_index = 0; l_index < ERRL_MAX_SLOTS; l_index++)
    {
        if(l_array[l_index]->iv_version != 0)
        {
            printf("slot[%01d] sz[%04d] id[%03d] @[%p] \n",l_index,l_array[l_index]->iv_userDetails.iv_entrySize, l_array[l_index]->iv_entryId, l_array[l_index]);
        }
        else
        {
            printf("slot[%01d] [0] \n",l_index);
        }
    }
    printf("-------- \n");
}

/*****************************************************************************/
// Image Header
/*****************************************************************************/
IMAGE_HEADER (G_errlTestMain,errlTestMain,ERRLTESTMAIN_ID,OCC_APLT_TEST);

