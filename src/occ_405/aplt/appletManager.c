/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/aplt/appletManager.c $                                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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

//*************************************************************************/
// Includes
//*************************************************************************/
#include "ssx.h"
#include <trac_interface.h>
#include <appletManager.h>
#include <aplt_service_codes.h>
#include <occ_service_codes.h>
#include <occ_common.h>
#include <comp_ids.h>
#include <thread.h>
#include <trac.h>
#include <errl.h>
#include <state.h>
#include "pba_firmware_registers.h"
#include "occhw_pba.h"
#include "occhw_async.h"
#if PPC405_MMU_SUPPORT
#include "ppc405_mmu.h"
#endif
#include "occhw.h"
#include "cmdh_fsp.h"

//*************************************************************************/
// Externs
//*************************************************************************/

//*************************************************************************/
// Macros
//*************************************************************************/

//*************************************************************************/
// Defines/Enums
//*************************************************************************/
#define TEST_APPLET_ADDR      (uint32_t)&_APPLET1_SECTION_BASE
#define PRDT_APPLET_ADDR      (uint32_t)&_APPLET0_SECTION_BASE
#define TEST_APPLET_MAX_SIZE  (uint32_t)&_APPLET1_SECTION_SIZE
#define PRDT_APPLET_MAX_SIZE  (uint32_t)&_APPLET0_SECTION_SIZE
#define APPLET_IMG_SIZE_ALIGN 256
#define APPLET_RO_SIZE_ALIGN 1024
#define SRAM_START_ADDRESS              0xFFF80000

//*************************************************************************
// Structures
//*************************************************************************
typedef struct
{
    // Running Semaphore (tells user if there is an applet running)
    SsxSemaphore    Running;
    // Wake up Semaphore (wakes up the applet manager thread)
    SsxSemaphore    Wakeup;
    // Finished Semaphore (tells caller, if blocking, when applet is done)
    SsxSemaphore    Finished;
} ApltSem_t;

// Applet information structure
//
// Added callerSem to hold caller semaphore parameter to the
// runApplet call. This semaphore will be used by startApplet to notify
// user of completion of the applet execution.
// Added status to get back error status from startApplet and
// return it to the caller for the blocking call.
struct ApltInfo
{
    void *          param;
    bool            isBlocking;
    OCC_APLT        appId;
    OCC_APLT        previousAppId;
    errlHndl_t      errorHandle;
    // Applet has readonly and writable sections
    Ppc405MmuMap    mmuMapWritePermission;
    Ppc405MmuMap    mmuMapReadPermission;
    SsxSemaphore *  callerSem;
    OCC_APLT_STATUS_CODES status;
} __attribute__ ((__packed__));

typedef struct ApltInfo ApltInfo_t;

//*************************************************************************
// Globals
//*************************************************************************
// Global semaphores for product and test applet
ApltSem_t G_ApltSemaphore;
ApltSem_t G_TestApltSemaphore;

// Applet Id global array
ApltAddress_t   G_ApltAddressTable[ OCC_APLT_LAST];

// Global product applet information with defaults
ApltInfo_t  G_ApltInfo =
{
    .param = NULL,
    .isBlocking = FALSE,
    .appId = OCC_APLT_INVALID,
    .previousAppId = OCC_APLT_INVALID,
    .errorHandle = NULL,
    .mmuMapWritePermission = 0,
    .mmuMapReadPermission = 0,
    .callerSem = NULL,
    .status = OCC_APLT_SUCCESS
};

// Global test applet information with defaults
ApltInfo_t G_TestApltInfo =
{
    .param = NULL,
    .isBlocking = FALSE,
    .appId = OCC_APLT_INVALID,
    .previousAppId = OCC_APLT_INVALID,
    .errorHandle = NULL,
    .mmuMapWritePermission = 0,
    .mmuMapReadPermission = 0,
    .callerSem = NULL,
    .status = OCC_APLT_SUCCESS
};

//*************************************************************************
// Function Prototypes
//*************************************************************************
// Have to have this declaration here so the pre-processor handles the
// section attribute correctly.
errlHndl_t initAppletAddr( void ) INIT_SECTION;

//*************************************************************************
// Functions
//*************************************************************************

// Function Specification
//
// Name:  initAppletAddr
//
// Description: initialize the Applet address by traversing through OCC signed
//              image in main memory
//
// End Function Specification
//

// NOTE: Optimization of O1 is needed for this function due to the l_bootLoaderHeader pointer
// pointing to a 0 address (which is considered NULL by the compiler) and thus with newer
// gcc compilers (4.9.0 and above), a new optimization flag issolate-erroneous-paths-dereference
// the compiler will set a trap in the code that will stop it from running.
// Setting the Optimization to 1 will disable this flag when compiling with gcc 4.9 and above.

errlHndl_t __attribute__((optimize("O1"))) initAppletAddr( void )
{
    errlHndl_t l_err = NULL;
    // 1. Applets count = OCC_APLT_TEST
    // 2. Start reading header addresses
    // 3. Skip first 2 ( boot, main ) addresses
    imageHdr_t *l_bootLoaderHeader = (void *) 0;
    imageHdr_t *l_mainAppHeader = (void *)l_bootLoaderHeader +
                    l_bootLoaderHeader->image_size;
    imageHdr_t *l_appHeader = (void *) l_mainAppHeader +
                    l_mainAppHeader->image_size;

    // 4. Save off each applet start address and size to global applet array
    // 5. Verify before saving off info (save off only valid addresses)
    uint32_t    l_cnt = 0;
    bool        l_foundInvalid = FALSE;

    for (; l_cnt < OCC_APLT_TEST ; l_cnt++ )
    {
        // Enable if debug is neeed:
        // TRAC_INFO("%s: Validating applet %d; address: %p", __FUNCTION__, l_cnt, l_appHeader);

        // 5a. Value of l_appHeader should be reasonable.
        //     Means that image sizes should be non-zero.
        //     TODO: Check l_appHeader vs. max. sane memory address.
        if( l_bootLoaderHeader->image_size == 0 )
        {
            l_foundInvalid =  TRUE;
            TRAC_ERR("Bad image size in boot loader header.  applet count: %d", l_cnt);
            break;
        }
        else if( l_mainAppHeader->image_size == 0 )
        {
            l_foundInvalid =  TRUE;
            TRAC_ERR("Bad image size in main app header.  applet count: %d", l_cnt);
            break;
        }

        // 5b. sram_repair_reserved holds APLT_MAGIC_NUMBER
        uint8_t l_magic_num[SRAM_REPAIR_RESERVE_SZ] = APLT_MAGIC_NUMBER; // Identifies this as a production applet
        int i=0;

        for(; i < SRAM_REPAIR_RESERVE_SZ; i++)
        {
            if(l_appHeader->sram_repair_reserved[i] != l_magic_num[i])
            {
                l_foundInvalid =  TRUE;
                break;
            }
        }
        if(l_foundInvalid == TRUE)
        {
#ifndef NO_TRAC_STRINGS
            uint32_t l_srr_0 = CONVERT_UINT8_ARRAY_UINT32(l_appHeader->sram_repair_reserved[0],
                                                          l_appHeader->sram_repair_reserved[1],
                                                          l_appHeader->sram_repair_reserved[2],
                                                          l_appHeader->sram_repair_reserved[3]);

            uint32_t l_srr_1 = CONVERT_UINT8_ARRAY_UINT32(l_appHeader->sram_repair_reserved[4],
                                                          l_appHeader->sram_repair_reserved[5],
                                                          l_appHeader->sram_repair_reserved[6],
                                                          l_appHeader->sram_repair_reserved[7]);

            uint32_t l_srr_2 = CONVERT_UINT8_ARRAY_UINT32(l_appHeader->sram_repair_reserved[8],
                                                          l_appHeader->sram_repair_reserved[9],
                                                          l_appHeader->sram_repair_reserved[10],
                                                          l_appHeader->sram_repair_reserved[11]);

            uint32_t l_srr_3 = CONVERT_UINT8_ARRAY_UINT32(l_appHeader->sram_repair_reserved[12],
                                                          l_appHeader->sram_repair_reserved[13],
                                                          l_appHeader->sram_repair_reserved[14],
                                                          l_appHeader->sram_repair_reserved[15]);

            // TEMP -- SSX ONLY SUPPORTS MAX OF 5 PARAMS
            //TRAC_ERR("wrong magic number.  applet count: %d, header addr: %p, magic no: 0x%08x%08x%08x%08x",
            //          l_cnt, l_appHeader, l_srr_0, l_srr_1, l_srr_2, l_srr_3);
#endif
            break;
        }

        // 5c. verify that aplt_id < OCC_APLT_TEST
        if( l_appHeader->aplt_id >= OCC_APLT_TEST )
        {
            l_foundInvalid =  TRUE;
// TEMP -- ERROR TRACING THE POINTER FOR SOME REASON
//            TRAC_ERR("applet ID out of range.  applet count: %d, header addr: %p, id: %d",
//                      l_cnt, l_appHeader, l_appHeader->aplt_id);
            break;
        }

        // 5d. 0 < image_size <= PRDT_APPLET_MAX_SIZE
        if( (l_appHeader->image_size == 0) || (l_appHeader->image_size > PRDT_APPLET_MAX_SIZE) )
        {
            l_foundInvalid =  TRUE;
// TEMP --- TRACE ERRORS
//            TRAC_ERR("bad image_size.  applet count: %d, applet ID: 0x%02x, header addr: %p, image_size: %d, avail space: %d",
//                      l_cnt, l_appHeader->aplt_id, l_appHeader, l_appHeader->image_size, PRDT_APPLET_MAX_SIZE);
            break;
        }

        // 5e. image_size is APPLET_IMG_SIZE_ALIGN byte aligned
        if( (l_appHeader->image_size % APPLET_IMG_SIZE_ALIGN) > 0 )
        {
            l_foundInvalid =  TRUE;
            TRAC_ERR("image size not aligned.  applet count: %d, applet ID: 0x%02x, header addr: %p, image_size: %d",
                      l_cnt, l_appHeader->aplt_id, l_appHeader, l_appHeader->image_size);
            break;
        }

        // 5f. readonly_size is APPLET_RO_SIZE_ALIGN byte aligned
        if( (l_appHeader->readonly_size % APPLET_RO_SIZE_ALIGN) > 0 )
        {
            l_foundInvalid =  TRUE;
            TRAC_ERR("readonly size not aligned.  applet count: %d, applet ID: 0x%02x, header addr: %p, readonly_size: %d",
                      l_cnt, l_appHeader->aplt_id, l_appHeader, l_appHeader->readonly_size);
            break;
        }

        // Each applet is loaded at address PRDT_APPLET_ADDR, and executed on-demand.

        // 5g. start_addr == the applet load address in SRAM
        if( l_appHeader->start_addr != PRDT_APPLET_ADDR )
        {
            l_foundInvalid =  TRUE;
// TEMP -- TRACE ERROR
//            TRAC_ERR("bad start_addr.  applet count: %d, applet ID: 0x%02x, header addr: %p, expected addr: 0x%08x, actual addr: 0x%08x",
//                      l_cnt, l_appHeader->aplt_id, l_appHeader, PRDT_APPLET_ADDR, l_appHeader->start_addr);
            break;
        }

        // 5h. ep_addr is between the header and the end of the applet image
        uint32_t l_ep_addr_min = PRDT_APPLET_ADDR + sizeof(imageHdr_t); // Lowest entry point an applet can reference
        uint32_t l_ep_addr_max = PRDT_APPLET_ADDR + l_appHeader->image_size;  // Upper bound on applet entry point

        if( (l_appHeader->ep_addr < l_ep_addr_min) || (l_appHeader->ep_addr >= l_ep_addr_max) )
        {
            l_foundInvalid =  TRUE;
// TEMP -- TOO MANY ARGS FOR TRACE
//            TRAC_ERR("entry point out of range.  applet count: %d, applet ID: 0x%02x, header addr: %p, ep_addr: 0x%08x, ep_range: 0x%08x - 0x%08x",
//                      l_cnt, l_appHeader->aplt_id, l_appHeader, l_appHeader->ep_addr, l_ep_addr_min, l_ep_addr_max);
            break;
        }

        // 5i. we haven't already seen this applet before
        //     (note the '!' at the start of the test condition)
        if( !((G_ApltAddressTable[l_appHeader->aplt_id].iv_aplt_address == 0x00000000) &&
             (G_ApltAddressTable[l_appHeader->aplt_id].iv_size == 0x00000000)) )
        {
            // This entry in the global table is not in its initialized state.
            l_foundInvalid =  TRUE;
            TRAC_ERR("Duplicate applet.  applet count: %d, applet ID: 0x%02x, header addr: %p",
                      l_cnt, l_appHeader->aplt_id, l_appHeader);
            break;
        }

        // The gauntlet is run.  This image is good.

        // Load the G_ApltAddressTable.
        G_ApltAddressTable[l_appHeader->aplt_id].iv_aplt_address = (uint32_t)l_appHeader;
        G_ApltAddressTable[l_appHeader->aplt_id].iv_size = l_appHeader->image_size;

        // Jump to the next applet.
        l_appHeader = (void *) l_appHeader + l_appHeader->image_size;

    } // end for loop

    // did we find any problems?
    if ( l_foundInvalid )
    {
        // We will want to return the ID of the failing applet in our error log,
        // if that ID exists.  Otherwise, return the "invalid" ID.
        uint16_t l_errApltId = OCC_APLT_INVALID;

        if( l_appHeader )
        {
            l_errApltId = l_appHeader->aplt_id;
        }

        /*
         * @errortype
         * @moduleid    APLT_MID_INIT_APPLET_ADDR
         * @reasoncode  INTERNAL_FAILURE
         * @userdata1   failing applet ID
         * @userdata2   failing applet main store address
         * @userdata4   ERC_APLT_INIT_FAILURE
         * @devdesc     Internal initialization failure in energy management
         */
        l_err = createErrl(APLT_MID_INIT_APPLET_ADDR,        //modId
                           INTERNAL_FAILURE,                 //reasoncode
                           ERC_APLT_INIT_FAILURE,            //Extended reason code
                           ERRL_SEV_UNRECOVERABLE,           //Severity
                           NULL,                             //Trace Buf
                           DEFAULT_TRACE_SIZE,               //Trace Size
                           l_errApltId,                      //userdata1
                           (uint32_t)l_appHeader);           //userdata2

        // Callout to firmware
        addCalloutToErrl(l_err,
                        ERRL_CALLOUT_TYPE_COMPONENT_ID,
                        ERRL_COMPONENT_ID_FIRMWARE,
                        ERRL_CALLOUT_PRIORITY_HIGH);
    }
    else
    {
        // No problems with any production applets.
        // 6. Save off the test applet start address and size to global applet array
        G_ApltAddressTable[OCC_APLT_TEST].iv_aplt_address = (uint32_t)l_appHeader;
        G_ApltAddressTable[OCC_APLT_TEST].iv_size = TEST_APPLET_MAX_SIZE;
    }

    // copy SSX mmu map to ApltInfo structure
    // startApplet() will use it to clear tlb entry
#if PPC405_MMU_SUPPORT
    G_ApltInfo.mmuMapWritePermission = G_applet0_mmu_map;
    G_TestApltInfo.mmuMapWritePermission = G_applet1_mmu_map;

    // Clear SSX mmu map.
    // The original tlb indexes may point to different SRAM regions when OCC runs
    // mmu map/unmap several times.
    G_applet0_mmu_map = 0;
    G_applet1_mmu_map = 0;
#endif
    return l_err;
}

// Function Specification
//
// Name:  initAppletManager
//
// Description: initialize the Applet Manager, called by main
//
// End Function Specification
void initAppletManager( void )
{
    errlHndl_t  l_rc = NULL;
    Ppc405MmuMap l_mmuMap;
    bool l_mmuUnmap = FALSE;
    do
    {
        // initialized all the semaphores applet manager controls
        // create the Running Semaphore, starting at 1 with a max count of 1
        int l_ssxrc1 = ssx_semaphore_create(&G_ApltSemaphore.Running, 1, 1);
        // create the Wake Up Semaphore, starting at 0 with a max count of 1
        int l_ssxrc2 = ssx_semaphore_create(&G_ApltSemaphore.Wakeup, 0, 1);
        // create the Finished Semaphore, starting at 0 with a max count of 1
        int l_ssxrc3 = ssx_semaphore_create(&G_ApltSemaphore.Finished, 0, 1);
        // create the Running Semaphore, starting at 1 with a max count of 1
        int l_ssxrc4 = ssx_semaphore_create(&G_TestApltSemaphore.Running, 1, 1);
        // create the Wake Up Semaphore, starting at 0 with a max count of 1
        int l_ssxrc5 = ssx_semaphore_create(&G_TestApltSemaphore.Wakeup, 0, 1);
        // create the Finished Semaphore, starting at 0 with a max count of 1
        int l_ssxrc6 = ssx_semaphore_create(&G_TestApltSemaphore.Finished, 0,1);
        // check for any errors from initializing our semaphores
        // NOTE: Applet Global Array list is left empty if semaphores
        //       are not initialized correctly
        if (( l_ssxrc1 != SSX_OK ) || ( l_ssxrc2 != SSX_OK ) ||
            ( l_ssxrc3 != SSX_OK ) || ( l_ssxrc4 != SSX_OK ) ||
            ( l_ssxrc5 != SSX_OK ) || ( l_ssxrc6 != SSX_OK ))
        {
            TRAC_INFO("Running  Semaphore SsxRc[0x%08X]", -l_ssxrc1 );
            TRAC_INFO("Wakeup   Semaphore SsxRc[0x%08X]", -l_ssxrc2 );
            TRAC_INFO("Finished Semaphore SsxRc[0x%08X]", -l_ssxrc3 );
            TRAC_INFO("Running  Test Semaphore SsxRc[0x%08X]", -l_ssxrc4);
            TRAC_INFO("Wakeup   Test Semaphore SsxRc[0x%08X]", -l_ssxrc5 );
            TRAC_INFO("Finished Test Semaphore SsxRc[0x%08X]", -l_ssxrc6 );

            /*
             * @errortype
             * @moduleid    APLT_MID_INIT_APPLET_MNGR
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   Create Running product aplt semaphore rc
             * @userdata2   Create Finished product aplt semaphore rc
             * @userdata4   ERC_CREATE_SEM_FAILURE
             * @devdesc     SSX semaphore related failure
             */
            l_rc = createErrl(APLT_MID_INIT_APPLET_MNGR,        //modId
                              SSX_GENERIC_FAILURE,              //reasoncode
                              ERC_CREATE_SEM_FAILURE,           //Extended reason code
                              ERRL_SEV_UNRECOVERABLE,           //Severity
                              NULL,                             //Trace Buf
                              DEFAULT_TRACE_SIZE,               //Trace Size
                              l_ssxrc1,                         //userdata1
                              l_ssxrc3);                        //userdata2
            break;
        }

        CHECKPOINT(APP_SEMS_CREATED);
#if PPC405_MMU_SUPPORT
        // Map mainstore to oci space so that we can walk through
        // OCC signed image to get applet addresses
        int l_ssxRc = ppc405_mmu_map(0,         //Mainstore address 0x0
                                     0,         //OCI address 0x0
                                     1048576,   //Max size = 1 Mb
                                     0,         // TLB hi flags
                                     0,         // TLB lo flags
                                     &l_mmuMap);// map pointer

        if ( l_ssxRc != SSX_OK )
        {
            TRAC_ERR("mmu map failure SsxRc[0x%08X]", -l_ssxRc );

            /*
             * @errortype
             * @moduleid    APLT_MID_INIT_APPLET_MNGR
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   MMU map return code
             * @userdata4   ERC_MMU_MAP_FAILURE
             * @devdesc     Failure mapping OCI space
             */
            l_rc = createErrl(APLT_MID_INIT_APPLET_MNGR,        //modId
                              SSX_GENERIC_FAILURE,              //reasoncode
                              ERC_MMU_MAP_FAILURE,              //Extended reason code
                              ERRL_SEV_UNRECOVERABLE,           //Severity
                              NULL,                             //Trace Buf
                              DEFAULT_TRACE_SIZE,               //Trace Size
                              l_ssxRc,                          //userdata1
                              0);                               //userdata2
            break;
        }
#endif /* PPC405_MMU_SUPPORT */
        CHECKPOINT(APP_MEM_MAPPED);

        // set to indicate to do mmu unmap after we are done using it.
        l_mmuUnmap = TRUE;

        // Traverse through OCC signed image to get applet address
        l_rc = initAppletAddr();

        CHECKPOINT(APP_ADDR_INITIALIZED);

        if( NULL != l_rc)
        {
            TRAC_ERR("Failure initializing applet addresses");
            break;
        }

    }while(FALSE);

    if( TRUE == l_mmuUnmap)
    {
        CHECKPOINT(APP_MEM_UNMAP);

#if PPC405_MMU_SUPPORT

        // Unmap mmu mapping for main store to oci space.
        // NOTE: This needs to be unmapped because 1) PowerBus disruptions
        // could leave OCC hung for several uS if main memory accessed at
        // some later point. 2) Any thing accessing address 0 should result
        // in 405 exception so need to unmap it.
        int l_ssxRc = ppc405_mmu_unmap(&l_mmuMap);

        if ( l_ssxRc != SSX_OK )
        {
            TRAC_ERR("mmu unmap failure SsxRc[0x%08X]", -l_ssxRc );

            /*
             * @errortype
             * @moduleid    APLT_MID_INIT_APPLET_MNGR
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   MMU unmap return code
             * @userdata4   ERC_MMU_UNMAP_FAILURE
             * @devdesc     Failure unmapping OCI space
             */
            errlHndl_t l_err = createErrl(APLT_MID_INIT_APPLET_MNGR,    //modId
                                          SSX_GENERIC_FAILURE,          //reasoncode
                                          ERC_MMU_UNMAP_FAILURE,        //Extended reason code
                                          ERRL_SEV_UNRECOVERABLE,       //Severity
                                          NULL,                         //Trace Buf
                                          DEFAULT_TRACE_SIZE,           //Trace Size
                                          l_ssxRc,                      //userdata1
                                          0);                           //userdata2
            // If there were not previous errors, return this error to the
            // caller. Otherwise, log this error and return previous error to
            // the caller.
            if( l_rc == NULL)
            {
                l_rc = l_err;
                deleteErrl(&l_err);
            }
            else
            {
                commitErrl(&l_err);
            }
        }
#endif /* PPC405_MMU_SUPPORT */
    }

    if( NULL != l_rc )
    {
        REQUEST_RESET(l_rc);
    }

}


// Function Specification
//
// Name:  runApplet
//
// Description: run an Applet
//
// End Function Specification
void runApplet( OCC_APLT                i_applet,
                void *                  i_parms,
                bool                    i_block,
                SsxSemaphore            *io_appletComplete,
                errlHndl_t              *o_errHndl,
                OCC_APLT_STATUS_CODES   *o_status )
{
    uint32_t l_reasonCode = 0;
    uint32_t l_extReasonCode = 0;
    int      l_ssxrc = 0;
    bool     l_postStartFailure = FALSE;

    // Depending on applet id use product or test applet globals
    ApltSem_t * l_sem =
           (i_applet == OCC_APLT_TEST? &G_TestApltSemaphore : &G_ApltSemaphore);
    ApltInfo_t * l_info =
           (i_applet == OCC_APLT_TEST? &G_TestApltInfo : &G_ApltInfo);

    //Block other runApplet caller until this request is complete
    l_ssxrc = ssx_semaphore_pend(&l_sem->Running, SSX_WAIT_FOREVER);

    if ( l_ssxrc != SSX_OK )
    {
        /*
         * @errortype
         * @moduleid    APLT_MID_RUN_APPLET
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   Failing applet ID
         * @userdata2   Caller parameter to determine block or not.
         * @userdata4   ERC_RUNNING_SEM_PENDING_FAILURE
         * @devdesc     SSX semaphore related failure
         */
        TRAC_ERR("Running Semaphore Pending Failure on blocking caller: SsxRc[0x%08X]", -l_ssxrc );
        l_reasonCode = SSX_GENERIC_FAILURE;
        l_extReasonCode = ERC_RUNNING_SEM_PENDING_FAILURE;
        l_info->status = OCC_APLT_PRE_START_FAILURE;
    }
    else
    {
        // First save off all the Globals

        // Save off the current Applet user wants to run
         l_info->appId = i_applet;

        // Save off whether or not we are blocking on this applet execution
         l_info->isBlocking = i_block;

        // save off pointer to parameters
         l_info->param = i_parms;

        // Point callerSem to the caller passed in semaphore
         l_info->callerSem = io_appletComplete;

        // clean out the handle
        l_info->errorHandle = NULL;

        l_info->status = OCC_APLT_SUCCESS;

        // This happens too often, we cannot trace it
        // TRAC_INFO("Waking up the Applet Manager Thread" );

        // Post wakeup semaphore so that applet manager can start
        // applet
        l_ssxrc = ssx_semaphore_post( &l_sem->Wakeup );
        if ( l_ssxrc != SSX_OK )
        {
            /*
             * @errortype
             * @moduleid    APLT_MID_RUN_APPLET
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   Failing applet ID
             * @userdata2   Caller parameter to determine block or not.
             * @userdata4   ERC_WAKEUP_SEM_POSTING_FAILURE
             * @devdesc     SSX semaphore related failure
             */
            TRAC_ERR("Wakeup Semaphore Post Failure: SsxRc[0x%08X]", -l_ssxrc );
            l_reasonCode = SSX_GENERIC_FAILURE;
            l_extReasonCode = ERC_WAKEUP_SEM_POSTING_FAILURE;

            //post on Running Semaphore so that user can retry
            l_ssxrc = ssx_semaphore_post( &l_sem->Running );
            if ( l_ssxrc != SSX_OK )
            {
               /*
                * @errortype
                * @moduleid    APLT_MID_RUN_APPLET
                * @reasoncode  SSX_GENERIC_FAILURE
                * @userdata1   Failing applet ID
                * @userdata2   Caller parameter to determine block or not.
                * @userdata4   ERC_RUNNING_SEM_POSTING_FAILURE
                * @devdesc     SSX semaphore related failure
                */
                TRAC_ERR("Error posting the Applet running semaphore rc=[%08X]",l_ssxrc);
                l_reasonCode = SSX_GENERIC_FAILURE;
                l_extReasonCode = ERC_RUNNING_SEM_POSTING_FAILURE;
            }
            // set rc to failed to start
            l_info->status = OCC_APLT_PRE_START_FAILURE;
        }
        // is caller expecting us to block then wait for finished semaphore
        // to be posted. Finished semaphore is posted after applet execution
        // is done.
        else if ( i_block )
        {
            l_ssxrc = ssx_semaphore_pend(&l_sem->Finished, SSX_WAIT_FOREVER);
            if ( l_ssxrc != SSX_OK )
            {
                /*
                 * @errortype
                 * @moduleid    APLT_MID_RUN_APPLET
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   Failing applet ID
                 * @userdata2   Caller parameter to determine block or not.
                 * @userdata4   ERC_FINISHED_SEM_PENDING_FAILURE
                 * @devdesc     SSX semaphore related failure
                 */
                TRAC_ERR("Finished Semaphore Pending Failure: SsxRc[0x%08X]", -l_ssxrc );
                l_reasonCode = SSX_GENERIC_FAILURE;
                l_extReasonCode = ERC_FINISHED_SEM_PENDING_FAILURE;
                // set flag to return post start failure status
                l_postStartFailure = TRUE;
            }
        }
    }

    if ( (l_reasonCode != 0) && (o_errHndl != NULL))
    {
        // TODO use correct trace
        tracDesc_t  l_trace = NULL;
        // Error from runApplet
        *o_errHndl = createErrl(APLT_MID_RUN_APPLET,        //modId
                                l_reasonCode,               //reasoncode
                                l_extReasonCode,            //Extended reason code
                                ERRL_SEV_UNRECOVERABLE,     //Severity
                                l_trace,                    //Trace Buf
                                DEFAULT_TRACE_SIZE,         //Trace Size
                                i_applet,                   //userdata1
                                i_block);                   //userdata2

        // Callout firmware
        addCalloutToErrl(*o_errHndl,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

    }
    else if ( (l_info->errorHandle) && (o_errHndl != NULL))
    {
        // Error handle from start applet
        *o_errHndl = l_info->errorHandle;
    }

    // set the status if requested by caller.
    if( o_status != NULL )
    {
        if(l_postStartFailure == TRUE )
        {
            *o_status = OCC_APLT_POST_START_FAILURE;
        }
        else
        {
            // Return status to the caller
            *o_status = l_info->status;
        }
    }

    //Running, Finished/user passed in semaphore is posted by start applet
};


// Function Specification
//
// Name:  startApplet
//
// Description: Start Applet
//
// End Function Specification
void startApplet( const OCC_APLT_TYPE i_isTestAplt )
{
    errlHndl_t  l_rc = NULL;
    int         l_ssxrc = 0;
    uint32_t    l_reasonCode = 0;
    uint32_t    l_extReasonCode = 0;
    // Depending on i_isTestAplt use product or test applet globals
    ApltInfo_t * l_info =
                    (i_isTestAplt == TRUE ? &G_TestApltInfo : &G_ApltInfo);
    ApltSem_t * l_sem =
               (i_isTestAplt == TRUE ? &G_TestApltSemaphore : &G_ApltSemaphore);

    OCC_APLT_STATUS_CODES l_status = OCC_APLT_SUCCESS;

    // Get default applet section size for mmu
    size_t l_applet_section_size =
           (i_isTestAplt == TRUE ? (size_t)&_APPLET1_SECTION_SIZE : (size_t)&_APPLET0_SECTION_SIZE);

    // For storing applet readonly size, this value is located in applet header in SRAM.
    // The value should be stored in another place before unmapping the applet memory region.
    size_t l_applet_readonly_size = 0;

    // 1. set tlb protection to write to SRAM:
    //    currently already writable

    do
    {
        // Get SRAM applet address depending on the applet type
        uint32_t  l_apltSramAddress =
                 (i_isTestAplt == TRUE? TEST_APPLET_ADDR: PRDT_APPLET_ADDR);

        // Set header to point to SRAM address
        imageHdr_t * l_apltHeader     = (void *) l_apltSramAddress;
        imageHdr_t *l_mainAppHeader   = (void *) SRAM_START_ADDRESS;

        // This happens too often, we cannot trace it
        //TRAC_INFO("Attempting to run applet at address [0x%08X], size: 0x%x",
        //          G_ApltAddressTable[l_info->appId].iv_aplt_address,
        //          G_ApltAddressTable[l_info->appId].iv_size);

        // check that we are not calling what is already saved off
        // re-load the test applet every time since they all use the same applet ID.
        if ( (l_info->appId == OCC_APLT_TEST) || (l_info->appId != l_info->previousAppId) )
        {

#if PPC405_MMU_SUPPORT

            // 1a. check applet read-only/executable permissions, unmap it if existed.
            //     whole applet memory region in SRAM should be free first, then set it to
            //     writable for copyimage and calculate checksum.
            if ( l_info->mmuMapReadPermission != 0 )
            {
                l_ssxrc = ppc405_mmu_unmap(&l_info->mmuMapReadPermission);

                if(l_ssxrc)
                {
                    TRAC_ERR("Error unmapping/changing permissions on applet text/data section rc=[%08X]", -l_ssxrc);

                    /*
                     * @errortype
                     * @moduleid    APLT_MID_START_APPLET
                     * @reasoncode  SSX_GENERIC_FAILURE
                     * @userdata1   Applet start status
                     * @userdata4   ERC_MMU_UNMAP_APPLET_READ_FAILURE
                     * @devdesc     Failure unmapping applet read-only/executable permissions
                     */
                    l_reasonCode = SSX_GENERIC_FAILURE;
                    l_extReasonCode = ERC_MMU_UNMAP_APPLET_READ_FAILURE;

                    // bail out
                    break;
                }
            }

            // 1b. check applet writable section permissions, unmap it if existed
            //     whole applet memory region in SRAM should be free first, then set it to
            //     writable for copyimage and calculate checksum.
            if ( l_info->mmuMapWritePermission != 0 )
            {
                l_ssxrc = ppc405_mmu_unmap(&l_info->mmuMapWritePermission);

                if(l_ssxrc)
                {
                    TRAC_ERR("Error unmapping/changing permissions on applet text/data section rc=[%08X]", -l_ssxrc);

                    /*
                     * @errortype
                     * @moduleid    APLT_MID_START_APPLET
                     * @reasoncode  SSX_GENERIC_FAILURE
                     * @userdata1   Applet start status
                     * @userdata4   ERC_MMU_UNMAP_APPLET_WRITE_FAILURE
                     * @devdesc     Failure unmapping applet rwdata section permissions
                     */
                    l_reasonCode = SSX_GENERIC_FAILURE;
                    l_extReasonCode = ERC_MMU_UNMAP_APPLET_WRITE_FAILURE;

                    // bail out
                    break;
                }
            }

            //1c. set SRAM applet region to writable for copying image and calculating checksum

            l_ssxrc = ppc405_mmu_map(
                        l_apltSramAddress,              //
                        l_apltSramAddress,              //
                        l_applet_section_size,          // set whole applet including writable section
                        0,                              //
                        TLBLO_WR,
                        &l_info->mmuMapWritePermission
                        );
            if(l_ssxrc != SSX_OK)
            {
                TRAC_ERR("Error mapping SRAM with execute permissions rc=[%08X]",l_ssxrc);

                /*
                 * @errortype
                 * @moduleid    APLT_MID_START_APPLET
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   Applet start status
                 * @userdata4   ERC_MMU_MAP_APPLET_OVERWRITE_FAILURE
                 * @devdesc     Failure mapping applet memory region permissions
                 */
                l_reasonCode = SSX_GENERIC_FAILURE;
                l_extReasonCode = ERC_MMU_MAP_APPLET_OVERWRITE_FAILURE;
                break;
            }

#endif /* PPC405_MMU_SUPPORT */

            // 2. read applet header info
            // 3. copy applet image to SRAM using DMA

            BceRequest pba_copy;
            l_ssxrc = bce_request_create(&pba_copy,                // block copy object
                                         &G_pba_bcde_queue,        // mainstore to sram copy engine
                                         G_ApltAddressTable[l_info->appId].iv_aplt_address,   // mainstore applet address
                                         l_apltSramAddress,        // sram applet starting address
                                         G_ApltAddressTable[l_info->appId].iv_size, // size of applet
                                         SSX_WAIT_FOREVER,         // no timeout
                                         0,                        // no call back
                                         0,                        // no call back arguments
                                         ASYNC_REQUEST_BLOCKING);  // blocking request

            if(l_ssxrc != SSX_OK)
            {
                TRAC_ERR("PBA request create failure rc=[%08X]",l_ssxrc);

                /*
                 * @errortype
                 * @moduleid    APLT_MID_START_APPLET
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   Applet start status
                 * @userdata4   ERC_BCE_REQUEST_CREATE_FAILURE
                 * @devdesc     SSX BCE related failure
                 */
                l_reasonCode = SSX_GENERIC_FAILURE;
                l_extReasonCode = ERC_BCE_REQUEST_CREATE_FAILURE;
                break;
            }

            // actual copying
            l_ssxrc = bce_request_schedule(&pba_copy);

            if(l_ssxrc != SSX_OK)
            {
                TRAC_ERR("PBA request schedule failure rc=[%08X]",l_ssxrc);

                /*
                 * @errortype
                 * @moduleid    APLT_MID_START_APPLET
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   Applet start status
                 * @userdata4   ERC_BCE_REQUEST_SCHEDULE_FAILURE
                 * @devdesc     Failure to copy applet image using DMA
                 */
                l_reasonCode = SSX_GENERIC_FAILURE;
                l_extReasonCode = ERC_BCE_REQUEST_SCHEDULE_FAILURE;
                break;
            }

            // --------------------------------------------------
            // Verify the applet version matches the main version
            // --------------------------------------------------
            if(l_mainAppHeader->version != l_apltHeader->version)
            {
                   TRAC_ERR("Applet not loaded b/c of version mismatch Main[%08x]:Applet[%08x]",
                            l_mainAppHeader->version, l_apltHeader->version);
                /*
                 * @errortype
                 * @moduleid    APLT_MID_START_APPLET
                 * @reasoncode  INTERNAL_FAILURE
                 * @userdata1   Applet start status
                 * @userdata4   ERC_APLT_START_VERSION_MISMATCH
                 * @devdesc     Internal failure in energy management
                 */
                l_reasonCode = INTERNAL_FAILURE;
                l_extReasonCode = ERC_APLT_START_VERSION_MISMATCH;
                break;

            }

            // 3b. invalidate the data cache
            //     ie: clean up data cache in case stuff is left
            //         from previous applet
            dcache_invalidate((void *) l_apltHeader, l_applet_section_size);

            // 4. Verify checksum of applet
            uint32_t    l_counter = 0;
            uint8_t *   l_srcPtr = (uint8_t *) (l_apltHeader);
            uint32_t    l_checksum = 0;
            uint32_t    l_headerChecksum = l_apltHeader->checksum;

            while (l_counter < l_apltHeader->image_size )
            {
                l_checksum += (*(l_srcPtr + l_counter));
                l_counter = l_counter + 1;
                if( l_counter == (uint32_t)(offsetof(imageHdr_t,checksum)))
                {
                    l_counter =  ((uint32_t)(offsetof(imageHdr_t,checksum)) +
                    sizeof(l_apltHeader->checksum));
                }
            }//end while loop

            if ( l_checksum != l_headerChecksum )
            {
                //error
                TRAC_ERR("Checksum verification failure header chksum=[%08X] calculated chksum =[%08X] ",
                            l_headerChecksum, l_checksum);

                /*
                 * @errortype
                 * @moduleid    APLT_MID_START_APPLET
                 * @reasoncode  INTERNAL_FAILURE
                 * @userdata1   Applet start status
                 * @userdata4   ERC_APLT_START_CHECKSUM_MISMATCH
                 * @devdesc     Internal failure in energy management
                 */
                l_reasonCode = INTERNAL_FAILURE;
                l_extReasonCode = ERC_APLT_START_CHECKSUM_MISMATCH;
                break;
            }

            // 5. flush complete I cache
            //    ie: ensure execution of current applet is clean
            icache_invalidate_all();


            // 5a. flush part of data cache
            //     ie: clean up data cache in case stuff is left
            //         from previous applet
            // TODO: need to ensure this will work correctly on HW
            //       may require tweaking, based on results
            dcache_invalidate((void *)l_apltHeader->zero_data_addr,
                              l_apltHeader->zero_data_size );

            // size of readonly section is located in applet memory section
            // get it for next step to protect text and data section
            l_applet_readonly_size = l_apltHeader->readonly_size;

#if PPC405_MMU_SUPPORT

            // 5b. unmap (remove permissions) on SRAM applet after copying applet image.
            //     permissions should be set correctly in SRAM rodata/rwdata section in next step.
            if ( l_info->mmuMapWritePermission != 0 )
            {
                l_ssxrc = ppc405_mmu_unmap(&(l_info->mmuMapWritePermission));

                if(l_ssxrc)
                {
                    TRAC_ERR("Error unmapping/changing permissions on SRAM applet section rc=[%08X]", -l_ssxrc);

                    /*
                     * @errortype
                     * @moduleid    APLT_MID_START_APPLET
                     * @reasoncode  SSX_GENERIC_FAILURE
                     * @userdata1   Applet start status
                     * @userdata4   ERC_MMU_UNMAP_APPLET_OVERWRITE_FAILURE
                     * @devdesc     Failure unmapping applet memory region permissions
                     */
                    l_reasonCode = SSX_GENERIC_FAILURE;
                    l_extReasonCode = ERC_MMU_UNMAP_APPLET_OVERWRITE_FAILURE;

                    // bail out
                    break;
                }
            }

            // 6 mapping mmu tlb permission (read-only section and writable section)
            // 6a. protect applet text in SRAM (tlbie)
            l_ssxrc = ppc405_mmu_map(
                        l_apltSramAddress,
                        l_apltSramAddress,
                        l_applet_readonly_size,         // protect the text and data section
                        0,
                        TLBLO_EX,
                        &(l_info->mmuMapReadPermission)
                        );
            if(l_ssxrc != SSX_OK)
            {
                TRAC_ERR("Error mapping SRAM with execute permissions rc=[%08X]",l_ssxrc);

                /*
                 * @errortype
                 * @moduleid    APLT_MID_START_APPLET
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   Applet start status
                 * @userdata4   ERC_MMU_MAP_APPLET_READ_FAILURE
                 * @devdesc     Failure mapping applet read-only/executable permissions
                 */
                l_reasonCode = SSX_GENERIC_FAILURE;
                l_extReasonCode = ERC_MMU_MAP_APPLET_READ_FAILURE;
                break;
            }

            // 6b. map applet rwdata section in SRAM as writable (tlbie)
            if(0 != l_apltHeader->boot_writeable_size)
            {
                l_ssxrc = ppc405_mmu_map(
                            l_apltHeader->boot_writeable_addr,
                            l_apltHeader->boot_writeable_addr,
                            l_apltHeader->boot_writeable_size,
                            0,
                            TLBLO_WR,                            // set rwdata section to write permission
                            &(l_info->mmuMapWritePermission)
                            );
                if(l_ssxrc != SSX_OK)
                {
                    TRAC_ERR("Error mapping SRAM with execute permissions rc=[%08X]",l_ssxrc);

                    /*
                    * @errortype
                    * @moduleid    APLT_MID_START_APPLET
                    * @reasoncode  SSX_GENERIC_FAILURE
                    * @userdata1   Applet start status
                    * @userdata4   ERC_MMU_MAP_APPLET_WRITE_FAILURE
                    * @devdesc     Failure mapping applet rwdata section permissions
                    */
                    l_reasonCode = SSX_GENERIC_FAILURE;
                    l_extReasonCode = ERC_MMU_MAP_APPLET_WRITE_FAILURE;
                    break;
                }
            }

#endif /* PPC405_MMU_SUPPORT */

            // save off current applet being run (already copied) into previous
            // ie: cache applet id
            // NOTE: This is done here, to ensure the full set up is always completed
            //       before saving off the current applet id into previous applet id var
            l_info->previousAppId = l_info->appId;

        }//end if

        // 7. Copy entry point address from applet header
        //    and start executing from that address
        errlHndl_t (*execute_app)(void *) =(void *) l_apltHeader->ep_addr;
        errlHndl_t l_applog = (*execute_app)(l_info->param);
        // check if applet return an error log
        if ( l_applog )
        {
            TRAC_ERR("Error found from applet function see error @%p",l_applog);

            if ( l_info->isBlocking )
            {
                // if applet caller wanted us to block
                // save off the error and status
                l_info->errorHandle = l_applog;
                l_info->status = OCC_APLT_EXECUTE_FAILURE;
            }
            else
            {
                // non blocker, so commit the error
                commitErrl(&l_applog);
            }
        }

    }while(0); // end of do while loop

    // All the error at this point setting reason code is before executing
    // applet so set the status to pre start failure.
    if( l_reasonCode != 0)
    {
        l_status = OCC_APLT_PRE_START_FAILURE;
        // TODO use correct trace
        tracDesc_t  l_trace = NULL;

        l_rc = createErrl(APLT_MID_START_APPLET,        //modId
                          l_reasonCode,                 //reasoncode
                          l_extReasonCode,              //Extended reason code
                          ERRL_SEV_UNRECOVERABLE,       //Severity
                          l_trace,                      //Trace Buf
                          DEFAULT_TRACE_SIZE,           //Trace Size
                          l_status,                     //userdata1
                          0);                           //userdata2

        // Callout to firmware
        addCalloutToErrl(l_rc,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        if ((l_info->isBlocking ) && (l_info->errorHandle == NULL))
        {
            // Return error to caller for blocking call and
            // there is no error executing applet
            l_info->errorHandle = l_rc;
            l_rc = NULL;
        }
        else
        {
            // commit log for the non-blocking call OR there is
            // error executing applet
            commitErrl(&l_rc);
        }
    }

    if( (l_info->isBlocking) && (l_info->status == OCC_APLT_SUCCESS))
    {
        // Get status only if call is blocking AND no error executing applet
        l_info->status = l_status;
    }
    // Moved posting of semaphores outside of while loop so that
    // in the error paths semaphores gets posted and not blocked forever.
    // 8. post on Finished Semaphore if caller wanted us to block them
    if ( l_info->isBlocking )
    {
        l_ssxrc = ssx_semaphore_post( &l_sem->Finished );

        if ( l_ssxrc != SSX_OK )
        {
            /*
            * @errortype
            * @moduleid    APLT_MID_START_APPLET
            * @reasoncode  SSX_GENERIC_FAILURE
            * @userdata4   ERC_FINISHED_SEM_POSTING_FAILURE
            * @devdesc     SSX semaphore related failure
            */
            TRAC_ERR("Error posting the Applet finished semaphore rc=[%08X]",l_ssxrc);
            l_reasonCode = SSX_GENERIC_FAILURE;
            l_extReasonCode = ERC_FINISHED_SEM_POSTING_FAILURE;
        }
    }
    // If caller passed in semaphore post it as caller might be blocked on
    // this semaphore.
    else if( l_info->callerSem != NULL)
    {
        l_ssxrc = ssx_semaphore_post( l_info->callerSem);

        if ( l_ssxrc != SSX_OK )
        {
            /*
            * @errortype
            * @moduleid    APLT_MID_START_APPLET
            * @reasoncode  SSX_GENERIC_FAILURE
            * @userdata4   ERC_CALLER_SEM_POSTING_FAILURE
            * @devdesc     SSX semaphore related failure
            */
            TRAC_ERR("Error posting the Applet complete semaphore rc=[%08X]",l_ssxrc);
            l_reasonCode = SSX_GENERIC_FAILURE;
            l_extReasonCode = ERC_CALLER_SEM_POSTING_FAILURE;
        }
    }

    // 8a. post on Running Semaphore
    l_ssxrc = ssx_semaphore_post( &l_sem->Running );
    if ( l_ssxrc != SSX_OK )
    {
        /*
         * @errortype
         * @moduleid    APLT_MID_START_APPLET
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata4   ERC_RUNNING_SEM_POSTING_FAILURE
         * @devdesc     SSX semaphore related failure
         */
        TRAC_ERR("Error posting the Applet running semaphore rc=[%08X]",l_ssxrc);
        l_reasonCode = SSX_GENERIC_FAILURE;
        l_extReasonCode = ERC_RUNNING_SEM_POSTING_FAILURE;
    }

    if( l_reasonCode != 0)
    {
        // TODO use correct trace
        tracDesc_t  l_trace = NULL;

        l_rc = createErrl(APLT_MID_START_APPLET,        //modId
                          l_reasonCode,                 //reasoncode
                          l_extReasonCode,              //Extended reason code
                          ERRL_SEV_UNRECOVERABLE,       //Severity
                          l_trace,                      //Trace Buf
                          DEFAULT_TRACE_SIZE,           //Trace Size
                          0,                            //userdata1
                          0);                           //userdata2

        // Callout to firmware
        addCalloutToErrl(l_rc,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        // Error posting semaphores so commit it.
        commitErrl(&l_rc);
    }
}


// Function Specification
//
// Name:  App_thread_routine
//
// Description: Applet Thread
//
// End Function Specification
void App_thread_routine(void *i_arg)
{
    // This happens too often, we cannot trace it
    //TRAC_INFO("Started ...");

    errlHndl_t l_errl = NULL;
    OCC_APLT_TYPE l_type =  APLT_TYPE_INVALID;
    // TODO use correct trace
    tracDesc_t  l_trace = NULL;

    if( i_arg == NULL)
    {
        TRAC_ERR("Invalid parameter");
        /*
         * @errortype
         * @moduleid    APLT_MNGR_THREAD
         * @reasoncode  INTERNAL_FAILURE
         * @userdata1   APLT_TYPE_INVALID
         * @userdata4   ERC_INVALID_INPUT_DATA
         * @devdesc     Internal failure in energy management
         */
        l_errl = createErrl(APLT_MNGR_THREAD,           //modId
                            INTERNAL_FAILURE,           //reasoncode
                            ERC_INVALID_INPUT_DATA,     //Extended reason code
                            ERRL_SEV_UNRECOVERABLE,     //Severity
                            l_trace,                    //Trace Buf
                            DEFAULT_TRACE_SIZE,         //Trace Size
                            l_type,                     //userdata1
                            0);                         //userdata2

        // Callout to firmware
        addCalloutToErrl(l_errl,
                        ERRL_CALLOUT_TYPE_COMPONENT_ID,
                        ERRL_COMPONENT_ID_FIRMWARE,
                        ERRL_CALLOUT_PRIORITY_HIGH);
        // commit log
        commitErrl( &l_errl );
        //TODO: should we reset OCC?
    }
    else
    {
        l_type = *((OCC_APLT_TYPE*)i_arg);

        while( 1 )
        {
            // This happens too often, we cannot trace it
            //TRAC_INFO("ApltType: [0x%x] Waiting ...", l_type);

            int l_ssxrc = SSX_OK;

            if( APLT_TYPE_PRODUCT == l_type)
            {
                l_ssxrc = ssx_semaphore_pend(&G_ApltSemaphore.Wakeup,
                                             SSX_WAIT_FOREVER);
            }
            else
            {
                l_ssxrc = ssx_semaphore_pend(&G_TestApltSemaphore.Wakeup,
                                             SSX_WAIT_FOREVER);

            }

            if ( l_ssxrc != SSX_OK )
            {
                TRAC_ERR("WakeUp Semaphore Failure SsxRc[0x%08X]", -l_ssxrc );

                /*
                 * @errortype
                 * @moduleid    APLT_MNGR_THREAD
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   Semaphore pend return code
                 * @userdata2   Applet type input from caller
                 * @userdata4   ERC_WAKEUP_SEM_PENDING_FAILURE
                 * @devdesc     SSX semaphore related failure
                 */
                l_errl = createErrl(APLT_MNGR_THREAD,               //modId
                                    SSX_GENERIC_FAILURE,            //reasoncode
                                    ERC_WAKEUP_SEM_PENDING_FAILURE, //Extended reason code
                                    ERRL_SEV_UNRECOVERABLE,         //Severity
                                    l_trace,                        //Trace Buf
                                    DEFAULT_TRACE_SIZE,             //Trace Size
                                    -l_ssxrc,                       //userdata1
                                    l_type);                        //userdata2

                // commit log
                REQUEST_RESET(l_errl);
                break;
            }

            startApplet(l_type);

        } // end while loop
    }

    TRAC_IMP("Applet Thread Routine [%d] Ended Unexpectedly.", l_type);
}
