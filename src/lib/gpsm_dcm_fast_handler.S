// $Id: gpsm_dcm_fast_handler.S,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $

/// \file gpsm_dcm_fast_handler.S
/// \brief Assembler support for GPSM_DCM procedures
/// \cond
        .nolist
#include "ssx.h"
#include "pmc_dcm.h"
#include "gpsm_dcm.h"
        .list

#define GPSM_IC_CONTINUOUS_DATA (PMC_IC_MSG_CC << 4 | GPSM_IC_DATA)

        // The gpsm_dcm_write fast handler.  

        // Register use:        
        //
        // R3 = Entry: void *arg;    *arg -> GpsmDcmFast
        // R4 = Entry: SsxIrqId irq; irq : constant
        // R5 = Entry: int priority; priority -> data 
        // R6 = data_pointer
        // R7 = remaining_size
        // CR = <condition>
        // LR = <return address>
        // No other registers can be used, other than SPRG
         
        .global_function gpsm_dcm_fast_write

gpsm_dcm_fast_write:
        
        _ssx_irq_status_clear %r4, %r6, %r7 //clear status first

        lwz    %r6, 0(%r3)                  //load data pointer
        lwz    %r7, 4(%r3)                  //load remaining size

        cmpwi  %r7, 0                       //if size == 0
        beq    exit_write_handler           //exit fast write
        cmpwi  %r7, 1                       //if size == 1
        beq    load_one_byte                //load only one byte 

        lhz    %r5, 0(%r6)                  //otherwise load two bytes 
        addi   %r6, %r6, 2                  //data pointer+2
        addi   %r7, %r7,-2                  //remaining size-2
        b      form_and_send_packet         //then form and send packet

load_one_byte:

        lbz    %r5, 0(%r6)                  //load one byte from buffer
        addi   %r6, %r6, 1                  //data pointer+1
        addi   %r7, %r7,-1                  //remaining size-1
        
form_and_send_packet:

        stw    %r6, 0(%r3)                  //store updated data pointer
        stw    %r7, 4(%r3)                  //store updated remaining size
                                            
        lis    %r6, GPSM_IC_CONTINUOUS_DATA //load cmd code to upper bits 
        or     %r5, %r5, %r6                //load data into lower bits
        slwi   %r5, %r5, 8                  //packet<<8 to give ecc field
        _stwi  %r5, %r7, PMC_INTCHP_MSG_WDATA               //send packet 
        blr                                                 //return

exit_write_handler:       

        bl __ssx_irq_fast2full              //convert fast to full irq handler
        _ssx_irq_disable %r4, %r6, %r7      //disable this irq
        addi %r3, %r3, 8                    //argument pointer to semaphore
        bl ssx_semaphore_post               //post to that semaphore
        b  __ssx_irq_full_mode_exit         //exit

        .epilogue gpsm_dcm_fast_write



        // The gpsm_dcm_read fast handler.  

        // Register use:        
        //
        // R3 = Entry: void *arg;    *arg -> GpsmDcmFast(data,size,semaphore)
        // R4 = Entry: SsxIrqId irq; irq : constant
        // R5 = Entry: int priority; priority -> data 
        // R6 = data_pointer
        // R7 = remaining_size
        // CR = <condition>
        // LR = <return address>
        // No other registers can be used, other than SPRG
         
        .global_function gpsm_dcm_fast_read

gpsm_dcm_fast_read:
        
        _ssx_irq_status_clear %r4, %r6, %r7 //clear status first

        lwz    %r7, 4(%r3)                  //load remaining size
        cmpwi  %r7, 0                       //if size == 0
        beq    exit_read_handler            //then exit fast read      

        _lwzi  %r6, %r7, PMC_INTCHP_MSG_RDATA    //receive packet     
        
        extrwi %r5, %r6, 16, 8              //extract data to r5
        extrwi %r6, %r6, 8,  0              //extract cmd code to r6
        li     %r7, GPSM_IC_CONTINUOUS_DATA //designated cmd code to r7

        cmpw   %r6, %r7                     //compare and check cmd code
        bne    panic_read_packet            //panic on wrong cmd code 

        lwz    %r6, 0(%r3)                  //load data pointer
        lwz    %r7, 4(%r3)                  //load remaining size

        cmpwi  %r7, 1                       //if only one byte left
        beq    store_one_byte               //then store only one byte

        sth    %r5, 0(%r6)                  //otherwise store two bytes to buf
        addi   %r6, %r6, 2                  //data pointer+2
        addi   %r7, %r7,-2                  //remaining size-2 
        b      check_read_status            //check read status

store_one_byte:

        stb    %r5, 0(%r6)                  //store one byte to buf
        addi   %r6, %r6, 1                  //data pointer+1
        addi   %r7, %r7,-1                  //remaining size-1
 
check_read_status:

        stw    %r6, 0(%r3)                  //store updated data pointer
        stw    %r7, 4(%r3)                  //store updated remaining size

        cmpwi  %r7, 0                       //if size == 0
        beq    exit_read_handler            //then terminate fast read
        blr                                 //return if still data left

exit_read_handler:

        bl __ssx_irq_fast2full              //convert fast to full irq handler
        _ssx_irq_disable %r4, %r6, %r7      //disable this irq 
        addi %r3, %r3, 8                    //argument pointer to semaphore
        bl ssx_semaphore_post               //post to that semaphore
        b  __ssx_irq_full_mode_exit         //exit

panic_read_packet:

        SSX_PANIC(GPSM_DCM_READ_NOT_WRITE_DATA) //PANIC if cmd code is wrong

        .epilogue gpsm_dcm_fast_read
/// \endcond

