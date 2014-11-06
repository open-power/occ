/******************************************************************************
// @file trac_interface.c
// @brief Interface codes for TRAC component.
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section  _trac_interface_c trac_interface.c
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      TEAM      06/16/2010  Port  
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *   @at009  859308     alvinwan  10/15/2012  Added tracepp support
 *   @ai005  860268     ailutsar  11/20/2012  Create trace test applet
 *   @nh004  864941     neilhsu   12/20/2012  Support get/delete errl & added trace info
 *   @rc005  864101     rickylie  12/12/2012  add small circ buffer to handle ISR semaphore conflict
 *   @at011  866759     alvinwan  01/15/2013  Fix OCC trexStringFile and trace
 *   @at012  868019     alvinwan  01/25/2014  TRAC_get_buffer_partial() can result in TLB Miss Exception
 *   @sbte0  916280     sbroyles  02/27/2014  Trace buffer fencing
 *  @endverbatim
 *
 *///*************************************************************************/

//*************************************************************************
// Includes
//*************************************************************************
#include "ssx.h"

#include <trac_interface.h>
#include <trac_service_codes.h>
//@pb00Ec - changed from common.h to occ_common.h for ODE support
#include <occ_common.h>
#include <comp_ids.h>
//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define TRAC_END_BUFFER                 "&"

#define TRAC_INTF_MUTEX_TIMEOUT         SSX_SECONDS(5)

#define     TRACE_BUF_VERSION   0x01;    /*!< Trace buffer version            */
#define     TRACE_FIELDTRACE    0x4654;  /*!< Field Trace - "FT"              */
#define     TRACE_FIELDBIN      0x4644   /*!< Binary Field Trace - "FD"       */

#define TRAC_TIME_REAL   0 // upper 32 = seconds, lower 32 = microseconds
#define TRAC_TIME_50MHZ  1
#define TRAC_TIME_200MHZ 2
#define TRAC_TIME_167MHZ 3 // 166666667Hz

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************
// @at009c - start

/// Instantiate the buffers for the traces.
///
/// There are 
/// It may be beneficial to add the attribute:
///     __attribute__ ((section (".noncacheable")))
/// when debugging on real HW, in case the OCC hangs and we can't access
/// the cache to get coherent data.
uint8_t g_trac_inf_buffer[TRACE_BUFFER_SIZE];
uint8_t g_trac_err_buffer[TRACE_BUFFER_SIZE];
uint8_t g_trac_imp_buffer[TRACE_BUFFER_SIZE];

// Need to modify the addTraceToErrl() function in errl.c when new trace buffer is added/removed
tracDesc_t g_trac_inf = (tracDesc_t) &g_trac_inf_buffer;
tracDesc_t g_trac_err = (tracDesc_t) &g_trac_err_buffer;
tracDesc_t g_trac_imp = (tracDesc_t) &g_trac_imp_buffer;

const trace_descriptor_array_t g_des_array[] =
{
    {&g_trac_inf,"INF"},
    {&g_trac_err,"ERR"},
    {&g_trac_imp,"IMP"}
};
// @at009c - end

SsxSemaphore    g_trac_mutex;

// @rc005a - start
static bool circular_full_flag = FALSE;
circular_buf_header_t g_isr_circular_header;
circular_entire_data_t g_isr_circular_buf[CIRCULAR_BUFFER_SIZE];
// @rc005a - end

//*************************************************************************
// Function Prototypes
//*************************************************************************
/**
 *  @brief  Initialize all header values of a trace buffer
 *
 *  This function will initialize all of the values in the trace buffer
 *  header so that it is ready for tracing.
 *
 *  @param o_buf Pointer to trace buffer which will be initialized.
 *  @param i_comp Component who is getting buffer initialized.
 *
 *  @return Non-zero return code on error.
 */
UINT trac_init_values_buffer(tracDesc_t *o_buf,const CHAR *i_comp);


 /**
 *  @brief  Raw buffer write function
 *
 *  This function assumes i_td has been initialized and it also assume
 *  the critical region of the input trace descriptor has been locked.
 *
 *  @param io_td Intialized trace descriptor point to buffer to trace to.
 *  @param i_ptr Pointer to data to write to trace buffer
 *  @param i_size Size of i_ptr
 *
 *  @return Non-zero return code on error.
 */
UINT trac_write_data(tracDesc_t io_td,
                    const void *i_ptr,
                    const ULONG i_size);

// @rc005a - start
 /**
 *  @brief  Raw buffer write function
 *
 *
 *  @param i_ptr 
 *
 *  @return Non-zero return code on error.
 */
uint16_t trac_write_data_to_circular(circular_entire_data_t *i_ptr);


 /**
 *  @brief  Raw buffer write function
 *
 *
 *  @param o_ptr
 *
 *  @return Non-zero return code on error.
 */
uint16_t get_trac_entry_data_from_circular(circular_entire_data_t *o_ptr);
// @rc005a - end

//*************************************************************************
// Functions
//*************************************************************************

// Function Specification
//
// Name: TRAC_init_buffers
//
// Description: 
//
// Flow:              FN=None
// 
// End Function Specification
UINT TRAC_init_buffers()
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    INT             l_rc = 0;
    UINT            l_num_des = 0;
    UINT            i=0;
    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

 
    // Initialize trace mutex
    l_rc = ssx_semaphore_create(&g_trac_mutex, 1, 1);
    
    if(SSX_OK != l_rc)
    {
        // Badness, don't continue
        FIELD("TRAC_init_buffers: Failed to create mutex");
    }
    else
    {
        // Initialize trace buffers
        l_num_des = sizeof(g_des_array) / sizeof(trace_descriptor_array_t);

        for(i=0;i<l_num_des;i++)
        {
            // Initialize the buffer
            l_rc = trac_init_values_buffer(g_des_array[i].entry,
                                           g_des_array[i].comp);
            if(l_rc)
            {
                FIELD1("TRAC_init_buffers: Failed to initialize buffer: ",
                       (unsigned char)i);
                break;
            }
        }
    }

    // @rc005a
    // Initialize isr circular buffer it to all 0's
    g_isr_circular_header.head = 0;
    g_isr_circular_header.tail = 0;
    g_isr_circular_header.entryCount = 0;

    memset(g_isr_circular_buf, 0 , CIRCULAR_BUFFER_SIZE * sizeof(circular_entire_data_t));

    return(l_rc);
}

// Function Specification
//
// Name: trac_init_values_buffer
//
// Description: 
//
// Flow:              FN=None
// 
// End Function Specification
UINT trac_init_values_buffer(tracDesc_t *o_buf,const CHAR *i_comp)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    UINT16             l_rc = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // Initialize it to all 0's
    memset(*o_buf,0,(size_t)TRACE_BUFFER_SIZE);

    (*o_buf)->ver = TRACE_BUF_VERSION;
    (*o_buf)->hdr_len = sizeof(trace_buf_head_t);
    (*o_buf)->time_flg = TRAC_TIME_REAL;  // @at009c
    (*o_buf)->endian_flg = 'B';  // Big Endian
    memcpy((*o_buf)->comp,i_comp,(size_t)COMP_NAME_SIZE);
    (*o_buf)->size = TRACE_BUFFER_SIZE;
    (*o_buf)->times_wrap = 0;
    (*o_buf)->next_free = sizeof(trace_buf_head_t);

    return(l_rc);
}

// Function Specification
//
// Name: trace_adal_write_all
//
// Description: In order to leverage the tracepp, need to add this function. It will call
//              trac_write_int finally
//
// Flow:              FN=None
//
// End Function Specification
//@at009a
UINT trace_adal_write_all(tracDesc_t io_td,const trace_hash_val i_hash,
                     const char *i_fmt,const ULONG i_line, const ULONG i_type,...)
{
    UINT rc = 0, i = 0;
    UINT l_num_args = 0;
    ULONG l_i_param[TRACE_MAX_ARGS] = {0}; // @ai005c

    // Calculate the number of optional parameters by looking at the i_fmt.
    // i_fmt will store something like "%d,%f,%u"
    if(i_fmt != NULL)
    {
        l_num_args = 1;
        for(i=0;i_fmt[i] != 0;i++)
        {
            if( i_fmt[i] == ',')
            {
                l_num_args++;
            }
        }
    }


    // Get the optional parameters
    va_list l_argptr;     //will hold optional parameters
    va_start(l_argptr,i_type);

    // @ai005a
    // Check the number of optional parameters
    if(TRACE_MAX_ARGS < l_num_args)     
    {
        l_num_args = TRACE_MAX_ARGS;
    }
	
    for (i=0;i<l_num_args;i++)
    {
        l_i_param[i] = va_arg(l_argptr,ULONG);
    }

    va_end(l_argptr);



    rc = trac_write_int(io_td,
                        i_hash, i_line,
                        l_num_args,
                        l_i_param[0],
                        l_i_param[1],
                        l_i_param[2],
                        l_i_param[3],
                        l_i_param[4]   );
    return rc;
}


// Function Specification
//
// Name: trac_write_int
//
// Description: 
//
// Flow:              FN=None
// 
// End Function Specification
UINT trac_write_int(tracDesc_t io_td,const trace_hash_val i_hash,
                     const ULONG i_line,
                     const UINT i_num_args,
                     const ULONG i_1,const ULONG i_2,const ULONG i_3,
                     const ULONG i_4,const ULONG i_5
                     )
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    UINT                 l_rc = 0;
    ULONG                l_entry_size = 0;
    trace_entire_entry_t l_entry;
    SsxMachineContext    l_ctx = 0; // @sbte0

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    if(io_td != NULL)
    {

        // Calculate total space needed
        l_entry_size = sizeof(trace_entry_stamp_t);
        l_entry_size += sizeof(trace_entry_head_t);

        // We always add the size of the entry at the end of the trace entry
        // so the parsing tool can easily walk the trace buffer stack so we
        // need to add that on to total size
        l_entry_size += sizeof(ULONG);

        // Now add on size for acutal number of arguments we're tracing
        l_entry_size += (i_num_args * sizeof(ULONG));

        // Word align the entry
        l_entry_size = (l_entry_size + 3) & ~3;

        // Fill in the entry structure
        //l_entry.stamp.tid = (ULONG)tx_thread_identify();   // What is response to this in AME code?
        l_entry.stamp.tid = (__ssx_kernel_context_thread() ? 1 : 0);  //context thread or ISR

        // Capture the time.  Note the time stamp is split into tbh (upper) and
        // tbl (lower), both of which are 32 bits each.  The ssx_timebase_get
        // call returns a uint64_t

        // @rc005c move
        // @at009c - start
        uint64_t l_time = ssx_timebase_get(); // @at011c
        l_entry.stamp.tbh = l_time / SSX_TIMEBASE_FREQUENCY_HZ;                // seconds     @at011c
        l_entry.stamp.tbl = ((l_time % SSX_TIMEBASE_FREQUENCY_HZ)*1000000000)  // nanoseconds @at011c
                          /SSX_TIMEBASE_FREQUENCY_HZ;
        // @at009c - end

        // Length is equal to size of data
        l_entry.head.length = (i_num_args * sizeof(ULONG));
        l_entry.head.tag = TRACE_FIELDTRACE;
        l_entry.head.hash = i_hash;
        l_entry.head.line = i_line;

        switch (i_num_args)
        {
            case 5: l_entry.args[4] = i_5;  // Intentional Fall Through ignore BEAM warning 
            case 4: l_entry.args[3] = i_4;  // Intentional Fall Through ignore BEAM warning 
            case 3: l_entry.args[2] = i_3;  // Intentional Fall Through ignore BEAM warning 
            case 2: l_entry.args[1] = i_2;  // Intentional Fall Through ignore BEAM warning 
            case 1: l_entry.args[0] = i_1;  // Intentional Fall Through ignore BEAM warning 
            default: ;
        }

        // Now put total size at end of buffer
        l_entry.args[i_num_args] = l_entry_size;

        // >> @sbte0 Disable non-critical interrupts if thread context
        if (__ssx_kernel_context_thread())
            ssx_critical_section_enter(SSX_NONCRITICAL, &l_ctx);
        // << @sbte0

        // @rc005c - start
        // Check if context thread or ISR get semaphore or not
        // If ISR did not get semaphore, will add trace log into circular buffer.
        // Context thread will check circular buffer, and add log back into trac buffer.
        // Prevent ISR did not get semaphore, and lost trace log.
        l_rc = ssx_semaphore_pend(&g_trac_mutex, 
                                  __ssx_kernel_context_thread()? \
                                  TRAC_INTF_MUTEX_TIMEOUT : SSX_NO_WAIT);

        if(l_rc == SSX_OK)
        {
            // @sbte0 Either this is thread context and mutex was locked within
            // timeout or this is interrupt context and mutex was immediately
            // available, regardless, mutex is now locked.
            l_rc = SUCCESS;

            // Update the entry count
            io_td->te_count++;

            l_rc = trac_write_data(io_td, (void *)&l_entry, l_entry_size);

            if(l_rc != SUCCESS)
            {
                // Badness - Not much we can do on trace failure.  Can't log error
                // because of recursion concerns.  Luckily a trace error is not critical.
                FIELD("trac_write_int: Failed in call to trac_write_data()");
            }

            // Always try to release even if error above
            ssx_semaphore_post(&g_trac_mutex);
        }
        else if(!__ssx_kernel_context_thread())
        {
            // @sbte0 Tracing in interrupt context and mutex was locked, SSX
            // returned -SSX_SEMAPHORE_PEND_NO_WAIT

            // Failed to get semaphore in ISR
            // Create trace in ISR circular buffer
            circular_entire_data_t l_cir_data_in;
            l_cir_data_in.len = l_entry_size;
            memcpy(&l_cir_data_in.comp, io_td->comp, (size_t)COMP_NAME_SIZE);
            l_cir_data_in.entry = l_entry;

            if(g_isr_circular_header.entryCount >= CIRCULAR_BUFFER_SIZE)
            {
                FIELD("trac_write_int: Circular Buffer size insufficient!\n");
                circular_full_flag = TRUE;
                l_rc = TRAC_CIRCULAR_BUFF_FULL;
                // Always try to release even if error above
                ssx_semaphore_post(&g_trac_mutex);
                return(l_rc);
            }

            // Save to Circular Buffer
            l_rc = trac_write_data_to_circular(&l_cir_data_in);
            g_isr_circular_header.head = (++g_isr_circular_header.head) % CIRCULAR_BUFFER_SIZE;
            g_isr_circular_header.entryCount++;

            if(l_rc != SUCCESS)
            {
                // Badness - Not much we can do on trace failure.  Can't log error
                // because of recursion concerns.  Luckily a trace error is not critical.
                FIELD("trac_write_int: Failed in call to trac_write_data_to_circular()");
            }
        }
        else
        {
            // Failed to get mutex in thread
            FIELD("trac_write_int: Failed to get mutex");
        }
        // @rc005c - end

        // >> @sbte0 Re-enable non-critical interrupts if thread context
        if (__ssx_kernel_context_thread())
            ssx_critical_section_exit(&l_ctx);
        // << @sbte0


        // @rc005a - start
        //2nd. Check caller from thread?
        if(__ssx_kernel_context_thread() && (g_isr_circular_header.entryCount > 0))
        {
            if(circular_full_flag)
            {
                // If ISR circular buffer is full, create a trace in IMP
                // Use existed trace structure to create new trace
                   
                // re-calculate size of the new trace entry
                l_entry_size = l_entry_size + ((1 - i_num_args)*4);

                // fill trace field
                l_entry.head.hash = trace_adal_hash("IMP: ISR Circular Buffer is full, %d entries losted", -1);
                l_entry.head.line = __LINE__;

                // one argment for this trace
                l_entry.head.length = sizeof(ULONG);
                l_entry.args[0] = circular_full_flag;
                l_entry.args[1] = l_entry_size;

                // >> @sbte0 Disable non-critical interrupts in thread context
                ssx_critical_section_enter(SSX_NONCRITICAL, &l_ctx);
                // << @sbte0

                //Write to IMP trace buffer
                l_rc = ssx_semaphore_pend(&g_trac_mutex,TRAC_INTF_MUTEX_TIMEOUT);
                if(l_rc == SSX_OK)
                {
                    tracDesc_t imp_td = TRAC_get_td("IMP");
                    // Update the entry count
                    imp_td->te_count++;

                    l_rc = trac_write_data(imp_td, (void *)&l_entry, l_entry_size);
                    if(l_rc != SUCCESS)
                    {
                        // Badness - Not much we can do on trace failure.  Can't log error
                        // because of recursion concerns.  Luckily a trace error is not critical.
                        FIELD("trac_write_int: Failed in call to trac_write_data()");
                    }
                    ssx_semaphore_post(&g_trac_mutex);
                }
                else
                {
                    // Failed to get mutex in thread
                    FIELD("trac_write_int: Failed to get mutex");
                }

                // >> @sbte0 Re-enable non-critical interrupts
                ssx_critical_section_exit(&l_ctx);
                // << @sbte0

                // Reset full flag
                circular_full_flag = FALSE;
                l_rc = TRAC_CIRCULAR_BUFF_FULL;
            }

            circular_entire_data_t l_cir_data_out;

            do
            {
                // >> @sbte0 Thread context here, disable non-critical
                // interrupts while unloading circular buffer
                ssx_critical_section_enter(SSX_NONCRITICAL, &l_ctx);
                // << @sbte0

                // Get tail position 
                g_isr_circular_header.tail = g_isr_circular_header.tail % CIRCULAR_BUFFER_SIZE;
                //Copy One trace entity from circular buffer
                get_trac_entry_data_from_circular(&l_cir_data_out);

                //Write to trace buffer
                l_rc = ssx_semaphore_pend(&g_trac_mutex,TRAC_INTF_MUTEX_TIMEOUT);
                if(l_rc == SSX_OK)
                {
                    tracDesc_t i_td = TRAC_get_td((const char *)l_cir_data_out.comp);
                    
                    // Update the entry count
                    i_td->te_count++;

                    l_rc = trac_write_data(i_td,
                                          (const void *)&l_cir_data_out.entry,
                                          (const ULONG)l_cir_data_out.len);

                    if(l_rc == SUCCESS)
                    {
                        if(g_isr_circular_header.tail == g_isr_circular_header.head )
                            g_isr_circular_header.entryCount = 0;
                        else
                        {
                            g_isr_circular_header.tail++;
                            g_isr_circular_header.entryCount--;
                        }
                    }
                    else
                    {
                        // Badness - Not much we can do on trace failure.  Can't log error
                        // because of recursion concerns.  Luckily a trace error is not critical.
                        FIELD("trac_write_int: Failed in call to trac_write_data()");
                    }

                    ssx_semaphore_post(&g_trac_mutex);
                }
                else
                {
                    // Failed to get mutex in thread
                    FIELD("trac_write_int: Failed to get mutex");
                }
                // >> @sbte0 Re-enable non-critical interrupts
                ssx_critical_section_exit(&l_ctx);
                // << @sbte0
            }
            while(g_isr_circular_header.entryCount > 0);
        }
       // @rc005a - end
    }
    else
    {
        l_rc = TRAC_INVALID_PARM;
        FIELD("trac_write_int: User passed invalid parameter");
    }

    return(l_rc);
}


// Function Specification
//
// Name: trac_write_bin
//
// Description: 
//
// Flow:              FN=None
// 
// End Function Specification
UINT trac_write_bin(tracDesc_t io_td,const trace_hash_val i_hash,
                    const ULONG i_line,
                    const void *i_ptr,
                    const ULONG i_size)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    UINT                l_rc = 0;
    ULONG               l_entry_size = 0;
    trace_bin_entry_t   l_entry;
    SsxMachineContext   l_ctx = 0; // @sbte0
    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    if((io_td == NULL) || (i_ptr == NULL) || (i_size == 0))
    {
        l_rc = TRAC_INVALID_PARM;
    }
    else
    {
        // Calculate total space needed
        l_entry_size = sizeof(trace_entry_stamp_t);
        l_entry_size += sizeof(trace_entry_head_t);

        // We always add the size of the entry at the end of the trace entry
        // so the parsing tool can easily walk the trace buffer stack so we
        // need to add that on to total size
        l_entry_size += sizeof(ULONG);

        // Now add on size for acutal size of the binary data
        l_entry_size += i_size;

        // Word align the entry
        l_entry_size = (l_entry_size + 3) & ~3;

        // Fill in the entry structure
        //l_entry.stamp.tid = (ULONG)tx_thread_identify();   // What is response to this in AME code?
        l_entry.stamp.tid = 0;   // What is response to this in AME code?

        // Length is equal to size of data
        l_entry.head.length = i_size;
        l_entry.head.tag = TRACE_FIELDBIN;
        l_entry.head.hash = i_hash;
        l_entry.head.line = i_line;

        // We now have total size and need to reserve a part of the trace
        // buffer for this

        // CRITICAL REGION START
        // >> @sbte0 Disable non-critical interrupts if thread context
        if (__ssx_kernel_context_thread())
            ssx_critical_section_enter(SSX_NONCRITICAL, &l_ctx);
        // << @sbte0

        l_rc = ssx_semaphore_pend(&g_trac_mutex,TRAC_INTF_MUTEX_TIMEOUT);

        if(l_rc != SSX_OK)
        {
            // Badness
            FIELD("trac_write_bin: Failed to get mutex");
        }
        else
        {
            // Capture the time.  Note the time stamp is split into tbh (upper) and
            // tbl (lower), both of which are 32 bits each.  The ssx_timebase_get
            // call returns a uint64_t

            // @at009c - start
            uint64_t l_time = ssx_timebase_get(); // @at011c
            l_entry.stamp.tbh = l_time / SSX_TIMEBASE_FREQUENCY_HZ;                // seconds     @at011c
            l_entry.stamp.tbl = ((l_time % SSX_TIMEBASE_FREQUENCY_HZ)*1000000000)  // nanoseconds @at011c
                              /SSX_TIMEBASE_FREQUENCY_HZ;
            // @at009c - end

            // Increment trace counter
            io_td->te_count++;;

            // First write the header
            l_rc = trac_write_data(io_td,
                                   (void *)&l_entry,
                                   sizeof(l_entry));
            do
            {
                if(l_rc != SUCCESS)
                {
                    // Badness - Not much we can do on trace failure.  Can't log error
                    // because of recursion concerns.  Luckily a trace error is not critical.
                    FIELD("trac_write_bin: Failed in call to trac_write_data - 1()");
                    break;
                }

                // Now write the actual binary data
                l_rc = trac_write_data(io_td,
                                       i_ptr,
                                       i_size);
                if(l_rc != SUCCESS)
                {
                    // Badness - Not much we can do on trace failure.  Can't log error
                    // because of recursion concerns.  Luckily a trace error is not critical.
                    FIELD("trac_write_bin: Failed in call to trac_write_data - 2()");
                    break;
                }

                // Now write the size at the end
                l_rc = trac_write_data(io_td,
                                       (void *)&l_entry_size,
                                       sizeof(l_entry_size));
                if(l_rc != SUCCESS)
                {
                    // Badness - Not much we can do on trace failure.  Can't log error
                    // because of recursion concerns.  Luckily a trace error is not critical.
                    FIELD("trac_write_bin: Failed in call to trac_write_data - 3()");
                    break;
                }
            }
            while(FALSE);

            ssx_semaphore_post(&g_trac_mutex);
            // >> @sbte0 Re-enable non-critical interrupts if thread context
            if (__ssx_kernel_context_thread())
                ssx_critical_section_exit(&l_ctx);
            // << @sbte0
        }
        // CRITICAL REGION END
    }

    return(l_rc);
}

// Function Specification
//
// Name: trac_write_data
//
// Description: 
//
// Flow:              FN=None
// 
// End Function Specification
UINT trac_write_data(tracDesc_t io_td,
                    const void *i_ptr,
                    const ULONG i_size)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    UINT                l_rc = 0;
    ULONG               l_total_size = i_size;
    void                *l_buf_ptr = NULL;
    ULONG               l_offset = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    do
    {

        if(i_size > TRACE_BUFFER_SIZE)
        {
            FIELD("trac_write_data: Input size to large!");
            l_rc = TRAC_DATA_SIZE_TO_LARGE;
            break;
        }

        if((io_td->next_free + l_total_size) > TRACE_BUFFER_SIZE)
        {
            // copy what we can to end
            l_buf_ptr = (char *)io_td + io_td->next_free;
            l_buf_ptr = (void *) ( ((ULONG) l_buf_ptr + 3) & ~3);
            l_offset = TRACE_BUFFER_SIZE-io_td->next_free;
            memcpy(l_buf_ptr,i_ptr,(size_t)l_offset);

            l_total_size -= l_offset;

            // Now adjust the main header of buffer
            io_td->times_wrap++;
            io_td->next_free = io_td->hdr_len;
        }

        l_buf_ptr = (char *)io_td + io_td->next_free;

        // Word align the write - total size includes this allignment
        l_buf_ptr = (void *) ( ((ULONG) l_buf_ptr + 3) & ~3);

        memcpy(l_buf_ptr,(char *)i_ptr + l_offset,l_total_size);

        // Make sure size is correct for word allignment
        // Note that this works with binary trace because only the binary data
        // has the potential to be un-word aligned.  If two parts of the binary
        // trace had this problem then this code would not work.
        l_total_size = (l_total_size + 3) & ~3;
        io_td->next_free += l_total_size;

    }while(FALSE);

    return(l_rc);

}

// Function Specification
//
// Name: TRAC_get_td
//
// Description: 
//
// Flow:              FN=None
// 
// End Function Specification
tracDesc_t TRAC_get_td(const char *i_comp)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    UINT            l_num_des = 0;
    UINT            i=0;
    tracDesc_t      l_td = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    l_num_des = sizeof(g_des_array) / sizeof(trace_descriptor_array_t);

    for(i=0;i<l_num_des;i++)
    {
        if(memcmp(i_comp,(*(g_des_array[i].entry))->comp,(size_t)COMP_NAME_SIZE) == 0)
        {
            // Found the component
            l_td = *g_des_array[i].entry;
            break;
        }
    }

    return(l_td);
}

// Function Specification
//
// Name: TRAC_get_buffer
//
// Description: 
//
// Flow:              FN=None
// 
// End Function Specification
UINT TRAC_get_buffer(const tracDesc_t i_td_ptr,
                    void *o_data)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    UINT            l_rc = 0;
    SsxMachineContext   l_ctx = 0; // @sbte0

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    if((i_td_ptr) && (o_data != NULL))
    {
        // >> @sbte0 Disable non-critical interrupts if thread context
        if (__ssx_kernel_context_thread())
            ssx_critical_section_enter(SSX_NONCRITICAL, &l_ctx);
        // << @sbte0

        // Get the lock
        l_rc = ssx_semaphore_pend(&g_trac_mutex,TRAC_INTF_MUTEX_TIMEOUT);
        if(l_rc != SSX_OK)
        {
            // Badness
            FIELD("TRAC_get_buffer: Failed to get mutex");
        }
        else
        {
            l_rc = SUCCESS;

            // @at009a -start
            // Copy it's buffer into temp one
            memcpy(o_data,i_td_ptr,(size_t)TRACE_BUFFER_SIZE);

            // Always try to release even if error above
            ssx_semaphore_post(&g_trac_mutex);
            // @at009a -end

            // >> @sbte0 Re-enable non-critical interrupts if thread context
            if (__ssx_kernel_context_thread())
                ssx_critical_section_exit(&l_ctx);
            // << @sbte0
        }
    }
    else
    {
        FIELD("TRAC_get_buffer: Invalid parameter passed by caller");
        l_rc = TRAC_INVALID_PARM;
    }

    return(l_rc);
}

// Function Specification
//
// Name: TRAC_get_buffer_partial
//
// Description: 
//
// Flow:              FN=None
// 
// End Function Specification
UINT TRAC_get_buffer_partial(const tracDesc_t i_td_ptr,
                    void *io_data,
                    UINT *io_size)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    UINT            l_rc = 0;
    char            *l_full_buf = NULL;
    tracDesc_t      l_head = NULL;
    UINT            l_part_size = 0;
    bool            l_lock_get = FALSE; // @at009a
    SsxMachineContext   l_ctx = 0; // @sbte0

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    do
    {
        if((i_td_ptr == NULL) || (io_data == NULL) || (io_size == NULL))
        {
            FIELD("TRAC_get_buffer_partial: Invalid parameter passed by caller");
            l_rc = TRAC_INVALID_PARM;
            if(io_size != NULL)
            {
                *io_size = 0;
            }
            break;
        }

        // We can't even fit in first part of buffer
        // Make sure data size is larger than header length
        // Otherwise, we will be accessing beyond memory
        if(*io_size < sizeof(trace_buf_head_t))
        {
            // Need to at least have enough space for the header
            FIELD("TRAC_get_buffer_partial: *io_size to small");
            l_rc = TRAC_DATA_SIZE_LESS_THAN_HEADER_SIZE;    // @ai005c
            *io_size = 0;
            break;
        }

        // CRITICAL REGION START
        // >> @sbte0 Disable non-critical interrupts if thread context
        if (__ssx_kernel_context_thread())
            ssx_critical_section_enter(SSX_NONCRITICAL, &l_ctx);
        // << @sbte0

        // Get the lock
        l_rc = ssx_semaphore_pend(&g_trac_mutex,TRAC_INTF_MUTEX_TIMEOUT); // @at009a
        if(l_rc != SSX_OK) // @at009a
        {
            // Badness
            FIELD("TRAC_get_buffer_partial: Failed to get mutex");
        }
        else
        {
            // Now that we have full buffer, adjust it to be requested size
            memset(io_data,0,(size_t)*io_size);

            l_lock_get = TRUE; // @at009a
            l_full_buf = (char*)i_td_ptr; // @at009a
            if(*io_size >= TRACE_BUFFER_SIZE)
            {
                // It fits
                *io_size = TRACE_BUFFER_SIZE;
                memcpy(io_data,l_full_buf,(size_t)*io_size);
                break;
            }

            // copy the header of the trace buffer to io_data
            l_head = (tracDesc_t)l_full_buf;
            memcpy(io_data,l_full_buf,(size_t)(l_head->hdr_len));

            // Reuse the l_head to point to the io_data and fill in the data
            l_head = (tracDesc_t)io_data;
            // @nh004d

            if((l_head->next_free == l_head->hdr_len) && (l_head->times_wrap == 0))
            {
                // No data in buffer so just return what we have
                *io_size = 0;       // @nh004a
                break;
            }

            if(l_head->next_free > *io_size)
            {
                // @ai005d remove duplcate case
                
                l_part_size = *io_size - l_head->hdr_len;

                memcpy((UCHAR *)io_data+l_head->hdr_len,
                       l_full_buf+l_head->next_free-l_part_size,
                       (size_t)l_part_size);

                // We don't need to update *io_size, all data copied.
                l_head->size = *io_size;    // @nh004a
                
                // Set pointer at beginning because this will be a
                // "just wrapped" buffer.
                l_head->next_free = l_head->hdr_len;

                // Buffer is now wrapped because we copied max data into it.
                if(!l_head->times_wrap)
                {
                    l_head->times_wrap = 1;
                }
            }
            else
            {
                // First part of buffer fits fine
                memcpy((UCHAR *)io_data+l_head->hdr_len,
                       l_full_buf+l_head->hdr_len,
                       (size_t)(l_head->next_free - l_head->hdr_len));


                // If it's wrapped then pick up some more data
                if(l_head->times_wrap)
                {
                    // Figure out how much room we have left
                    l_part_size = *io_size - l_head->next_free;

                    memcpy((UCHAR *)io_data+l_head->next_free,
                           l_full_buf+TRACE_BUFFER_SIZE-l_part_size,
                           (size_t)l_part_size);
                    
                    // We don't need to update *io_size, all data copied.
                    l_head->size = *io_size;    // @nh004a
                }
                else
                {
                    // Update copied lens which is what we have in trace buf
                    l_head->size = l_head->next_free;   // @nh004c
                    *io_size = l_head->next_free;       // @nh004c
                }
            }
        }
        // CRITICAL REGION END
    }
    while(FALSE);

    // @at009a - start
    // Always try to release even if error above
    if(l_lock_get)
    {
        ssx_semaphore_post(&g_trac_mutex);
        // >> @sbte0 Re-enable non-critical interrupts if thread context
        if (__ssx_kernel_context_thread())
            ssx_critical_section_exit(&l_ctx);
        // << @sbte0
    }
    // @at009a - end

    return(l_rc);
}

// Function Specification
//
// Name: TRAC_reset_buf
//
// Description: 
//
// Flow:              FN=None
// 
// End Function Specification
UINT TRAC_reset_buf()
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    UINT            l_rc = 0;
    UINT            l_num_des = 0;
    UINT            i=0;
    SsxMachineContext   l_ctx = 0; // @sbte0
    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // >> @sbte0 Disable non-critical interrupts if thread context
    if (__ssx_kernel_context_thread())
        ssx_critical_section_enter(SSX_NONCRITICAL, &l_ctx);
    // << @sbte0

    // Get mutex so no one traces
    l_rc = ssx_semaphore_pend(&g_trac_mutex,TRAC_INTF_MUTEX_TIMEOUT);    
    if(l_rc != SSX_OK)
    {
        FIELD("TRAC_reset_buf: Failure trying to get mutex");
        // Badness
    }
    else
    {
        l_num_des = sizeof(g_des_array) / sizeof(trace_descriptor_array_t);

        for(i=0;i<l_num_des;i++)
        {
            // Initialize the buffer
            l_rc = trac_init_values_buffer(g_des_array[i].entry,
                                           g_des_array[i].comp);
            if(l_rc)
            {
                FIELD("TRAC_reset_buf: Failure in call to trac_init_values_buffer()");
                break;
            }
        }
    }

    // Always try to release even if fail above
    ssx_semaphore_post(&g_trac_mutex); 

    // >> @sbte0 Re-enable non-critical interrupts if thread context
    if (__ssx_kernel_context_thread())
        ssx_critical_section_exit(&l_ctx);
    // << @sbte0

    return(l_rc);
}


// @rc005a - start

// Function Specification
//
// Name: trac_write_data_to_circular
//
// Description: 
//
// Flow:              FN=None
// 
// End Function Specification
uint16_t trac_write_data_to_circular(circular_entire_data_t *i_ptr)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t             l_rc = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    memcpy((void *)&g_isr_circular_buf[g_isr_circular_header.head], 
           (void *)i_ptr, 
           sizeof(circular_entire_data_t));

    return(l_rc);
}

// Function Specification
//
// Name: get_trac_entry_data_from_circular
//
// Description: 
//
// Flow:              FN=None
// 
// End Function Specification
uint16_t get_trac_entry_data_from_circular(circular_entire_data_t *o_ptr)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t            l_rc = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    memcpy((void *)o_ptr, 
           (void *)&g_isr_circular_buf[g_isr_circular_header.tail], 
           sizeof(circular_entire_data_t));

    return(l_rc);
}

// @rc005a -end
