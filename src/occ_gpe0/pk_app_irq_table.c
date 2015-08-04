/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/pk_app_irq_table.c $                             */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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
#include "pk.h"

EXTERNAL_IRQ_TABLE_START
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_DEBUGGER                      
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_TRACE_TRIGGER                 
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_OCC_ERROR                     
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PBA_ERROR                     
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_SRT_ERROR                     
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_GPE0_HALT                     
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_GPE1_HALT                     
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_GPE2_HALT                     
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_GPE3_HALT                     
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PPC405_HALT                   
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_OCB_ERROR                     
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_SPIPSS_ERROR                  
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_CHECK_STOP_PPC405             
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_CHECK_STOP_GPE0               
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_CHECK_STOP_GPE1               
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_CHECK_STOP_GPE2               
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_CHECK_STOP_GPE3               
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_OCC_MALF_ALERT                
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_ADU_MALF_ALERT                
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_EXTERNAL_TRAP                 
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_IVRM_PVREF_ERROR              
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_OCC_TIMER0                    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_OCC_TIMER1                    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_AVS_SLAVE0                    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_AVS_SLAVE1                    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_IPI0_HI_PRIORITY              
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_IPI1_HI_PRIORITY              
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_IPI2_HI_PRIORITY              
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_IPI3_HI_PRIORITY              
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_IPI4_HI_PRIORITY              
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_ADCFSM_ONGOING                
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_RESERVED_31                   
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PBAX_OCC_SEND                 
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PBAX_OCC_PUSH0                
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PBAX_OCC_PUSH1                
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PBA_BCDE_ATTN                 
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PBA_BCUE_ATTN                 
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_STRM0_PULL                    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_STRM0_PUSH                    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_STRM1_PULL                    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_STRM1_PUSH                    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_STRM2_PULL                    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_STRM2_PUSH                    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_STRM3_PULL                    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_STRM3_PUSH                    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PMC_PCB_INTR_TYPE0_PENDING    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PMC_PCB_INTR_TYPE1_PENDING    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PMC_PCB_INTR_TYPE2_PENDING    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PMC_PCB_INTR_TYPE3_PENDING    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PMC_PCB_INTR_TYPE4_PENDING    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PMC_PCB_INTR_TYPE5_PENDING    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PMC_PCB_INTR_TYPE6_PENDING    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PMC_PCB_INTR_TYPE7_PENDING    
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PMC_O2S_0A_ONGOING            
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PMC_O2S_0B_ONGOING            
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PMC_O2S_1A_ONGOING            
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PMC_O2S_1B_ONGOING            
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_PSSBRIDGE_ONGOING             
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_IPI0_LO_PRIORITY              
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_IPI1_LO_PRIORITY              
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_IPI2_LO_PRIORITY              
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_IPI3_LO_PRIORITY              
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_IPI4_LO_PRIORITY              
IRQ_HANDLER_DEFAULT            //OCCHW_IRQ_RESERVED_63
EXTERNAL_IRQ_TABLE_END

