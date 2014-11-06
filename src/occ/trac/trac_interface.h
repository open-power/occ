/******************************************************************************
// @file trac_interface.h
// @brief Interface codes for TRAC component.
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _trac_interface_h trac_interface.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      TEAM      06/16/2010  Port  
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *   @at009  859308     alvinwan  10/15/2012  Added tracepp support
 *   @ai005  860268     ailutsar  11/20/2012  Create trace test applet
 *   @rc005  864101     rickylie  12/12/2012  add small circ buffer to handle ISR semaphore conflict
 *
 *  @endverbatim
 *
 *///*************************************************************************/
 
#ifndef _TRAC_INTERFACE_H
#define _TRAC_INTERFACE_H

/** \defgroup Trace Trace Component
 * Port of the trace code used in the fsp and tpmd.
 */

//*************************************************************************
// Includes
//*************************************************************************
//@pb00Ec - changed from common.h to occ_common.h for ODE support
#include <occ_common.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************
/**
 * \defgroup TracMacros Trace Component Macro Interfaces
 * \ingroup Trace
 * Used to trace 0 - 5 arguments or a binary buffer when using a hash value.
 */
/*@{*/


// @at009c - start
#define TRACE(i_td,i_string,args...)                  \
                trace_adal_write_all(i_td,trace_adal_hash(i_string,-1),__LINE__,0,##args)

#define TRACEBIN(i_td,i_string,i_ptr,i_size)          \
                trac_write_bin(i_td,trace_adal_hash(i_string,0),__LINE__,i_ptr,i_size)

#ifndef NO_TRAC_STRINGS

#define FIELD(a) \
        printf("%s",a)

#define FIELD1(a,b) \
        printf("%s%lx",a,(unsigned long)b)

#else  // NO_TRAC_STRINGS

#define FIELD(a)

#define FIELD1(a,b)

#endif  // NO_TRAC_STRINGS

#define SUCCESS 0
// @at009c - end



/*@}*/

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define     TRACE_MAX_ARGS      5        /*!< Maximum number of args to trace */

typedef uint32_t trace_hash_val;          

// NOTE!  Increment this when new components are added!
#define TRAC_NUM_TRACE_COMPONENTS           1


#define TRACE_BUFFER_SIZE                   8192 // @ai005a

#define CIRCULAR_BUFFER_SIZE                4   // @rc005a

//*************************************************************************
// Structures
//*************************************************************************
/*
 * @brief Structure is put at beginning of all trace buffers
 */
typedef struct trace_buf_head {
    UCHAR ver;         /*!< version of this struct (1)                      */
    UCHAR hdr_len;     /*!< size of this struct in bytes                    */
    UCHAR time_flg;    /*!< meaning of timestamp entry field                */
    UCHAR endian_flg;  /*!< flag for big ('B') or little ('L') endian       */
    CHAR comp[16];     /*!< the buffer name as specified in init call       */
    UINT32 size;       /*!< size of buffer, including this struct           */
    UINT32 times_wrap; /*!< how often the buffer wrapped                    */
    UINT32 next_free;  /*!< offset of the byte behind the latest entry      */
    UINT32 te_count;   /*!< Updated each time a trace is done               */
    UINT32 extracted;  /*!< Not currently used                              */
}trace_buf_head_t;

/*!
 * @brief Timestamp and thread id for each trace entry.
 */
typedef struct trace_entry_stamp {
    UINT32 tbh;        /*!< timestamp upper part                            */
    UINT32 tbl;        /*!< timestamp lower part                            */
    UINT32 tid;        /*!< process/thread id                               */
}trace_entry_stamp_t;

/*
 * @brief Structure is used by adal app. layer to fill in trace info.
 */
typedef struct trace_entry_head {
    UINT16 length;      /*!< size of trace entry                             */
    UINT16 tag;         /*!< type of entry: xTRACE xDUMP, (un)packed         */
    UINT32 hash;       /*!< a value for the (format) string                 */
    UINT32 line;       /*!< source file line number of trace call           */
}trace_entry_head_t;

/*
 * @brief Parameter traces can be all contained in one write.
 */
typedef struct trace_entire_entry {
    trace_entry_stamp_t stamp;
    trace_entry_head_t head;
    UINT32 args[TRACE_MAX_ARGS + 1];
} trace_entire_entry_t;


/*
 * @brief Binary first writes header and time stamp.
 */
typedef struct trace_bin_entry {
    trace_entry_stamp_t stamp;
    trace_entry_head_t head;
} trace_bin_entry_t;

/**
 * @brief Used as input to traces to get to correct buffer.
 */
typedef trace_buf_head_t * tracDesc_t;

/*
 * @brief Structure is used to hold array of all trace descriptors
 */
typedef struct trace_descriptor_array
{
    tracDesc_t  *entry; /*!< Pointer to trace descriptor                   */
    CHAR        *comp;  /*!< Pointer to component name                     */
}trace_descriptor_array_t;

// @rc005a - start
typedef struct circular_buf_head
{
    UINT32 head;       // pointer to head
    UINT32 tail;       // pointer to tail
    UINT32 entryCount; // nums of entry
} circular_buf_header_t;


typedef struct circular_entire_data {
    UINT32 len;
    CHAR comp[4];
    trace_entire_entry_t entry;
} circular_entire_data_t;

// @rc005a - end

//*************************************************************************
// Globals
//*************************************************************************
// All TPMF component trace descriptors.
extern tracDesc_t g_trac_inf;  // @at009a
extern tracDesc_t g_trac_err;  // @at009a
extern tracDesc_t g_trac_imp;  // @at009a

extern const trace_descriptor_array_t g_des_array[];

//*************************************************************************
// Function Prototypes
//*************************************************************************

/**
 * \defgroup TracIntefaces Trace Component Interfaces for External Use
  * \ingroup Trace
 */
/*@{*/


/**
 *  @brief  Allocate and initialize all trace buffers in memory.
 *
 *  This function will allocate memory for each of the pre-defined trace
 *  buffers, initilize the buffers with starting data, and setup the 
 *  trace descriptors which each component will use to trace.
 *
 *  This function must be called first before any components try to trace!
 *
 *  @return Non-zero return code on error.
 */
UINT TRAC_init_buffers(void);

/**
 *  @brief  Retrieve full trace buffer for component i_comp
 *
 *  This function assumes memory has already been allocated for
 *  the full trace buffer in o_data.
 *
 *  @param i_td_ptr Trace descriptor of buffer to retrieve.
 *  @param o_data Pre-allocated pointer to where data will be stored.
 *
 *  @return Non-zero return code on error
 */
UINT TRAC_get_buffer(const tracDesc_t i_td_ptr,
                     void *o_data);

/**
 *  @brief  Retrieve partial trace buffer for component i_comp
 *
 *  This function assumes memory has already been allocated for
 *  the trace buffer (size io_size).  This function will copy
 *  in up to io_size in bytes to the buffer and set io_size
 *  to the exact size that is copied in.
 *
 *  @param i_td_ptr Trace descriptor of buffer to retrieve.
 *  @param o_data Pre-allocated pointer to where data will be stored.
 *  @param io_size Size of trace data to retrieve (input)
 *                 Actual size of trace data stored (output)
 *
 *  @return Non-zero return code on error
 */
UINT TRAC_get_buffer_partial(const tracDesc_t i_td_ptr,
                    void *o_data,
                    UINT *io_size);
                    
/**
 *  @brief  Retrieve trace descriptor for input component name
 *
 *  @param i_comp Component name to retrieve trace descriptor for.
 *
 *  @return Valid trace descriptor on success, NULL on failure.
 */                    
tracDesc_t TRAC_get_td(const char *i_comp);

/**
 *  @brief  Reset all trace buffers
 *
 *  @return Non-zero return code on error
 */                    
UINT TRAC_reset_buf(void);

/*@}*/  // Ending tag for external interface module in doxygen


/**
 *  @brief  Trace input integers to trace buffer.
 *
 *  This function assumes i_td has been initialized.
 *
 *  @param io_td Intialized trace descriptor point to buffer to trace to.
 *  @param i_hash Hash value to be recorded for this trace.
 *  @param i_fmt Output format
 *  @param i_line Line number trace is occurring on.
 *  @param i_type trace type. filed or debug.
 *  @param ... parames that are limited to a size of 4 bytes, i.e. int, uint32_t, nnn*
 *
 *  @return Non-zero return code on error.
 */
UINT trace_adal_write_all(tracDesc_t io_td,const trace_hash_val i_hash,
                     const char *i_fmt,const ULONG i_line, const ULONG i_type,...);


/**
 *  @brief  Trace input integers to trace buffer.
 *
 *  This function assumes i_td has been initialized.
 *
 *  @param io_td Intialized trace descriptor point to buffer to trace to.
 *  @param i_hash Hash value to be recorded for this trace.
 *  @param i_line Line number trace is occurring on.
 *  @param i_num_args Number of arguments to trace.
 *  @param i_1 Input Parameter 1
 *  @param i_2 Input Parameter 2
 *  @param i_3 Input Parameter 3
 *  @param i_4 Input Parameter 4
 *  @param i_5 Input Parameter 5
 *
 *  @return Non-zero return code on error.
 */
UINT trac_write_int(tracDesc_t io_td,const trace_hash_val i_hash,
                    const ULONG i_line,
                    const UINT i_num_args,
                    const ULONG i_1,const ULONG i_2,const ULONG i_3,
                    const ULONG i_4,const ULONG i_5
                   );


 /**
 *  @brief  Trace binary data to buffer.
 *
 *  This function assumes i_td has been initialized.
 *
 *  @param io_td Intialized trace descriptor point to buffer to trace to.
 *  @param i_hash Hash value to be recorded for this trace.
 *  @param i_line Line number trace is occurring on.
 *  @param i_ptr Pointer to binary data to trace.
 *  @param i_size Size of data to copy from i_ptr.
 *
 *  @return Non-zero return code on error.
 */
UINT trac_write_bin(tracDesc_t io_td,const trace_hash_val i_hash,
                    const ULONG i_line,
                    const void *i_ptr,
                    const ULONG i_size);

//*************************************************************************
// Functions
//*************************************************************************

#endif //_TRAC_INTERFACE_H
