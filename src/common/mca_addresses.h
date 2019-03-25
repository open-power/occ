/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/common/mca_addresses.h $                                  */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2019                        */
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

#ifndef _MCA_ADDRESSES_H
#define _MCA_ADDRESSES_H

#define NUM_NIMBUS_MC_PAIRS 2
#define MAX_NUM_MCU_PORTS   4
#define NUM_NIMBUS_MCAS (MAX_NUM_MCU_PORTS * NUM_NIMBUS_MC_PAIRS)

// Base Address of NIMBUS MCA.
#define DIMM_MCA_BASE_ADDRESS           0x07010800

#define MCA_MCPAIR_SPACE                0x01000000
#define MCA_PORT_SPACE                  0x40
#define MC_PORT_SPACE(mc,port)          ((MCA_MCPAIR_SPACE * (mc)) + ( MCA_PORT_SPACE * (port)))


#define POWER_CTRL_REG0_OFFSET          0x0134
#define POWER_CTRL_REG0_ADDRESS         (DIMM_MCA_BASE_ADDRESS + POWER_CTRL_REG0_OFFSET)

#define STR_REG0_OFFSET                 0x0135
#define STR_REG0_ADDRESS                (DIMM_MCA_BASE_ADDRESS + STR_REG0_OFFSET)

#define PERF_MON_COUNTS0_OFFSET         0x0137
#define PERF_MON_COUNTS0_ADDRESS        (DIMM_MCA_BASE_ADDRESS + PERF_MON_COUNTS0_OFFSET)

#define N_M_TCR_OFFSET                  0x0116
#define N_M_TCR_ADDRESS                 (DIMM_MCA_BASE_ADDRESS + N_M_TCR_OFFSET)

#define PERF_MON_COUNTS_IDLE_OFFSET     0x013C
#define PERF_MON_COUNTS_IDLE_ADDRESS    (DIMM_MCA_BASE_ADDRESS + PERF_MON_COUNTS_IDLE_OFFSET)

#define EMERGENCY_THROTTLE_OFFSET       0x0125
#define EMERGENCY_THROTTLE_ADDRESS      (DIMM_MCA_BASE_ADDRESS + EMERGENCY_THROTTLE_OFFSET)
#define ER_THROTTLE_IN_PROGRESS_MASK    0x8000000000000000

#define MCA_CAL_FIR_OFFSET              0x0100
#define MCA_CAL_FIR_ADDRESS             (DIMM_MCA_BASE_ADDRESS + MCA_CAL_FIR_OFFSET)
#define MCA_FIR_THROTTLE_ENGAGED_MASK   0x0200000000000000

// Memory Power Control

//Power Control Register 0:     MC#.PORT#.SRQ.PC.MBARPC0Q
//STR Register 0:               MC#.PORT#.SRQ.PC.MBASTR0Q
//DDR Interface SCOM Control:   MC#.PORT#.SRQ.MBA_FARB5Q
//there are 4 each (1 per MCU port)
//OCC knows present MCU ports from the memory throttle config packet

/*                                  PC / STR / SCtl   Power Ctl reg 0     STR reg 0    DDR IF SCOM CTRL
MC/Port Address MCA Port Address    Control Addr       SCOM Address     SCOM Address     SCOM Address
mc01.port0        0x07010800        + 0x134/135/118    = 0x07010934     = 0x07010935     = 0x07010918
mc01.port1        0x07010840        + 0x134/135/118    = 0x07010974     = 0x07010975     = 0x07010958
mc01.port2        0x07010880        + 0x134/135/118    = 0x070109B4     = 0x070109B5     = 0x07010998
mc01.port3        0x070108C0        + 0x134/135/118    = 0x070109F4     = 0x070109F5     = 0x070109d8
mc23.port0        0x08010800        + 0x134/135/118    = 0x08010934     = 0x08010935     = 0x08010918
mc23.port1        0x08010840        + 0x134/135/118    = 0x08010974     = 0x08010975     = 0x08010958
mc23.port2        0x08010880        + 0x134/135/118    = 0x080109B4     = 0x080109B5     = 0x08010998
mc23.port3        0x080108C0        + 0x134/135/118    = 0x080109F4     = 0x080109F5     = 0x080109d8
 */

#define POWER_CTRL_REG0(mc,port) (POWER_CTRL_REG0_ADDRESS + MC_PORT_SPACE(mc,port))

#define STR_REG0(mc,port) (STR_REG0_ADDRESS + MC_PORT_SPACE(mc,port))
#define STR_REG0_MCA(mca) (STR_REG0_ADDRESS + MC_PORT_SPACE((mca>>2),(mca&3)))

// Required for NVDIMM procedure
#define DDR_IF_SCOM_CTRL(mc,port)    (DIMM_MCA_BASE_ADDRESS + 0x0118 + MC_PORT_SPACE(mc,port))

// DIMM Control
/*
MC/Port Address MCA Port Address    Control Addr        SCOM Address
mc01.port0        0x07010800        + 0x00000116        = 0x07010916
mc01.port1        0x07010840        + 0x00000116        = 0x07010956
mc01.port2        0x07010880        + 0x00000116        = 0x07010996
mc01.port3        0x070108C0        + 0x00000116        = 0x070109D6
mc23.port0        0x08010800        + 0x00000116        = 0x08010916
mc23.port1        0x08010840        + 0x00000116        = 0x08010956
mc23.port2        0x08010880        + 0x00000116        = 0x08010996
mc23.port3        0x080108C0        + 0x00000116        = 0x080109D6
 */

//  N/M DIMM Throttling Control SCOM Register Addresses macro
#define N_M_DIMM_TCR(mc,port) (N_M_TCR_ADDRESS + MC_PORT_SPACE(mc,port))

/*
MC/Port Address MCA Port Address   "Deadman" Offset       SCOM Address
mc01.port0        0x07010800        + 0x0000013C        = 0x0701093C
mc01.port1        0x07010840        + 0x0000013C        = 0x0701097C
mc01.port2        0x07010880        + 0x0000013C        = 0x070109BC
mc01.port3        0x070108C0        + 0x0000013C        = 0x070109FC
mc23.port0        0x08010800        + 0x0000013C        = 0x0801093C
mc23.port1        0x08010840        + 0x0000013C        = 0x0801097C
mc23.port2        0x08010880        + 0x0000013C        = 0x080109BC
mc23.port3        0x080108C0        + 0x0000013C        = 0x080109FC
 */

// NIMBUS DIMM Performance Monitor -- Idle Threshold Counter
// This register effectively acts as a deadman register.
// Reading it will reset the deadman counter.
#define PERF_MON_COUNTS_IDLE_MCA(mca) (PERF_MON_COUNTS_IDLE_ADDRESS + MC_PORT_SPACE((mca>>2),(mca&3)))

/*
MC/Port Address MCA Port Address   "Deadman" Offset       SCOM Address
mc01.port0        0x07010800        + 0x00000137        = 0x07010937
mc01.port1        0x07010840        + 0x00000137        = 0x07010977
mc01.port2        0x07010880        + 0x00000137        = 0x070109B7
mc01.port3        0x070108C0        + 0x00000137        = 0x070109F7
mc23.port0        0x08010800        + 0x00000137        = 0x08010937
mc23.port1        0x08010840        + 0x00000137        = 0x08010977
mc23.port2        0x08010880        + 0x00000137        = 0x080109B7
mc23.port3        0x080108C0        + 0x00000137        = 0x080109F7
 */

// NIMBUS DIMM Performance Monitor -- Read/Write Count
#define PERF_MON_COUNTS0_MCA(mca) (PERF_MON_COUNTS0_ADDRESS + MC_PORT_SPACE((mca>>2),(mca&3)))

// Emergency Mode Throttle Register
/*
MC/Port Address MCA Port Address  Emergency Throt Offset SCOM Address
mc01.port0        0x07010800        + 0x00000125        = 0x07010925
mc01.port1        0x07010840        + 0x00000125        = 0x07010965
mc01.port2        0x07010880        + 0x00000125        = 0x070109A5
mc01.port3        0x070108C0        + 0x00000125        = 0x070109E5
mc23.port0        0x08010800        + 0x00000125        = 0x08010925
mc23.port1        0x08010840        + 0x00000125        = 0x08010965
mc23.port2        0x08010880        + 0x00000125        = 0x080109A5
mc23.port3        0x080108C0        + 0x00000125        = 0x080109E5
 */

//  NIMBUS Emergency Throttle SCOM Register Addresses macro
#define ER_THROTTLE_MCA(mca) (EMERGENCY_THROTTLE_ADDRESS + MC_PORT_SPACE((mca>>2),(mca&3)))

// MCA Calibration Fault Isolation Register
/*
MC/Port Address MCA Port Address   MCA FIR Offset       SCOM Address
mc01.port0        0x07010800        + 0x00000100        = 0x07010900
mc01.port1        0x07010840        + 0x00000100        = 0x07010940
mc01.port2        0x07010880        + 0x00000100        = 0x07010980
mc01.port3        0x070108C0        + 0x00000100        = 0x070109C0
mc23.port0        0x08010800        + 0x00000100        = 0x08010900
mc23.port1        0x08010840        + 0x00000100        = 0x08010940
mc23.port2        0x08010880        + 0x00000100        = 0x08010980
mc23.port3        0x080108C0        + 0x00000100        = 0x080109C0
 */

//  NIMBUS MCA Calibration FIR SCOM Register Addresses macro
#define MCA_CAL_FIR_REG_MCA(mca) (MCA_CAL_FIR_ADDRESS + MC_PORT_SPACE((mca>>2),(mca&3)))

// MCBIST Control Register:      MC#.MCBIST.MBA_SCOMFIR.MCB_CNTLQ
#define MCBIST_BASE_ADDRESS      0x070123DB
#define MCBIST_CTRL_REG(mc)      (MCBIST_BASE_ADDRESS + (MCA_MCPAIR_SPACE * (mc)))

#endif // _MCA_ADDRESSES_H
