/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/pgas_ppc.h $                                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2016                        */
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
#ifndef __PGAS_PPC_H__
#define __PGAS_PPC_H__

// ** WARNING : This file is maintained as part of the OCC firmware.  Do **
// ** not edit this file in the PMX area, the hardware procedure area,   **
// ** or the PoreVe area as any changes will be lost.                    **

/// \file pgas_ppc.h
/// \brief Legacy PGAS assembler implemented as PowerPC assembler macros.
///
/// PGAS is documented in a seperate standalone document entitled <em> PGAS :
/// PORE GAS (GNU Assembler) User's and Reference Manual </em>.
///
/// This file contains the legacy PGAS assembler, which was first implemented
/// as this set of assembler macros for the PowerPC assembler.  This file is
/// included into pgas.h if the compile switch PGAS_PPC is defined in the
/// compile environment.

#ifdef __ASSEMBLER__

////////////////////////////////////////////////////////////////////////////
// PGAS Base Assembler
////////////////////////////////////////////////////////////////////////////    


        //////////////////////////////////////////////////////////////////////
        // Symbolic Register Mnemonics
        //////////////////////////////////////////////////////////////////////
        //
        // PGAS uses gas symbols for register mnemonics so that they will
        // appear as-is in assembler listings, but we can still do arithmetic
        // on the mnemonics in the PGAS macros.

        .set    P0,    PORE_REGISTER_PRV_BASE_ADDR0 
        .set    P1,    PORE_REGISTER_PRV_BASE_ADDR1 
        .set    A0,    PORE_REGISTER_OCI_BASE_ADDR0 
        .set    A1,    PORE_REGISTER_OCI_BASE_ADDR1 
        .set    CTR,   PORE_REGISTER_SCRATCH0  
        .set    D0,    PORE_REGISTER_SCRATCH1       
        .set    D1,    PORE_REGISTER_SCRATCH2
        .set    EMR,   PORE_REGISTER_ERROR_MASK
        .set    ETR,   PORE_REGISTER_EXE_TRIGGER
        .set    SPRG0, PORE_REGISTER_DATA0
        .set    PC,    PORE_REGISTER_PC
        .set    IFR,   PORE_REGISTER_IBUF_ID

      
        //////////////////////////////////////////////////////////////////////
        // Core Instruction Set
        //////////////////////////////////////////////////////////////////////

        // The final construction of an instruction word.  The opcode is a
        // 7-bit value and the operand is always a 24-bit value. Note that the
        // parity bit is always 0.

        .macro  ..instruction, opcode, operand
        .long   (\opcode << 25) | (\operand)
        .endm

        // NOP, TRAP, RET

        .macro  nop
        ..instruction PGAS_OPCODE_NOP, 0
        .endm

        .macro  trap
        ..instruction PGAS_OPCODE_TRAP, 0
        .endm

        .macro  ret
        ..instruction PGAS_OPCODE_RET, 0
        .endm

        // WAITS, HALT, HOOKI

        .macro  waits, u24:req
        ..check_u24 (\u24)
        .if     ((\u24) == 0)
        .error "PGAS does not allow WAITS 0; Use HALT if the intention is to halt"
        .endif
        ..instruction PGAS_OPCODE_WAITS, (\u24)
        .endm

        .macro  halt
        ..instruction PGAS_OPCODE_WAITS, 0
        .endm

        .macro  hooki, u24:req, imm:req
        ..check_u24     (\u24)
        ..instruction PGAS_OPCODE_HOOKI, (\u24) 
        .quad   (\imm)  
        .endm

        .macro  wait, args:vararg
        .error "PGAS implements the 'waits' mnemonic instead of PORE 'wait'"
        .endm

        .macro  hook, args:vararg
        .error "PGAS implements the 'hooki' mnemonic instead of PORE 'hook'"
        .endm

        // BRA, LOOP
        //
        // Note that all branch offsets in PORE are WORD offsets, so the byte
        // offsets computed by the underlying assembler need to be divided by
        // 4. Unfortunately PGAS is not able to check whether the offsets fit
        // in the allowed space.

        .macro  ..bra, opcode, target
        ..instruction \opcode, ((((\target) - $) / 4) & 0xffffff)
        .endm

        .macro  bra, target:req
        ..bra   PGAS_OPCODE_BRA, (\target)
        .endm

        .macro  loop, target:req
        ..bra   PGAS_OPCODE_LOOP, (\target)
        .endm

        // BRAZ, BRANZ

        .macro  ..brac, opcode, src, target
        ..branch_compare_data (\src)
        ..instruction \opcode, ((\src << 20) | ((((\target) - $) / 4) & 0xfffff))
        .endm

        .macro  braz, src:req, target:req
        ..brac PGAS_OPCODE_BRAZ, (\src), (\target)
        .endm

        .macro  branz, src:req, target:req
        ..brac PGAS_OPCODE_BRANZ, (\src), (\target)
        .endm

        // CMPIBRAEQ, CMPIBRANE

        .macro  ..cmpibra, opcode, src, target, imm
        ..d0    (\src)
        ..instruction \opcode, ((((\target) - $) / 4) & 0xffffff)
        .quad   (\imm)
        .endm

        .macro  cmpibraeq, src:req, target:req, imm:req
        ..cmpibra PGAS_OPCODE_CMPIBRAEQ, (\src), (\target), (\imm)
        .endm

        .macro  cmpibrane, src:req, target:req, imm:req
        ..cmpibra  PGAS_OPCODE_CMPIBRANE, (\src), (\target), (\imm)
        .endm

        .macro  cmpbra, args:vararg
        .error "PGAS implements the 'cmpibraeq' mnemonic instead of PORE 'cmpbra'"
        .endm

        .macro  cmpnbra, args:vararg
        .error "PGAS implements the 'cmpibrane' mnemonic instead of PORE 'cmpnbra'"
        .endm

        // BRAD, BSRD

        .macro  ..brad, opcode, src
        ..data  (\src)
        ..instruction \opcode, ((\src) << 20)
        .endm   

        .macro  brad, src:req
        ..brad  PGAS_OPCODE_BRAD, (\src)
        .endm

        .macro  bsrd, src:req
        ..brad  PGAS_OPCODE_BSRD, (\src)
        .endm

        // ANDI, ORI, XORI
        
        .macro  ..ilogic, opcode, dest, src, imm
        ..data  (\dest)
        ..data  (\src)
        ..instruction \opcode, (((\dest) << 20) | ((\src) << 16))
        .quad   \imm
        .endm

        .macro  andi, dest:req, src:req, imm:req
        ..ilogic PGAS_OPCODE_ANDI, (\dest), (\src), (\imm)
        .endm

        .macro  ori, dest:req, src:req, imm:req
        ..ilogic PGAS_OPCODE_ORI, (\dest), (\src), (\imm)
        .endm

        .macro  xori, dest:req, src:req, imm:req
        ..ilogic PGAS_OPCODE_XORI, (\dest), (\src), (\imm)
        .endm

        // AND, OR, XOR, ADD, SUB

        .macro  ..alurr, opcode, dest, src1, src2
        ..data  (\dest)
        ..d0d1  (\src1), (\src2)
        ..instruction \opcode, ((\dest) << 20)
        .endm

        .macro  and, dest:req, src1:req, src2:req
        ..alurr PGAS_OPCODE_AND, (\dest), (\src1), (\src2)
        .endm

        .macro  or, dest:req, src1:req, src2:req
        ..alurr PGAS_OPCODE_OR, (\dest), (\src1), (\src2)
        .endm

        .macro  xor, dest:req, src1:req, src2:req
        ..alurr PGAS_OPCODE_XOR, (\dest), (\src1), (\src2)
        .endm

        .macro  add, dest:req, src1:req, src2:req
        ..alurr PGAS_OPCODE_ADD, (\dest), (\src1), (\src2)
        .endm

        .macro  sub, dest:req, src1:req, src2:req
        ..alurr PGAS_OPCODE_SUB, (\dest), (\src1), (\src2)
        .endm

        // ADDS, SUBS

        .macro  ..inc, opcode, dest, src, short
        ..check_s16 (\short)
        ..ls_destination (\dest)
        ..same  (\dest), (\src)
        ..instruction (\opcode), (((\dest) << 20) | ((\short) & 0xffff))
        .endm
        
        .macro  adds, dest:req, src:req, short:req
        ..inc   PGAS_OPCODE_ADDS, (\dest), (\src), (\short)
        .endm

        .macro  subs, dest:req, src:req, short:req
        ..inc   PGAS_OPCODE_SUBS, (\dest), (\src), (\short)
        .endm

        .macro  addi, args:vararg
        .error "PGAS implements the 'adds' mnemonic instead of PORE 'addi'"
        .endm

        .macro  subi, args:vararg
        .error "PGAS implements the 'subs' mnemonic instead of PORE 'subi'"
        .endm

        // NEG
        
        .macro  neg, dest:req, src:req
        ..data  (\dest)
        ..data  (\src)
        ..instruction PGAS_OPCODE_NEG, (((\dest) << 20) | ((\src) << 16))
        .endm           
             
        // MR

        .macro  mr, dest:req, src:req
        ..mr_destination (\dest)
        ..mr_source (\src)
        ..instruction PGAS_OPCODE_MR, (((\dest) << 20) | ((\src) << 16))
        .endm

        .macro  copy, args:vararg
        .error  "PGAS implents the 'mr' mnemonic instead of PORE 'copy'"
        .endm

        // ROLS

        .macro  rols, dest:req, src:req, short:req
        ..data  (\dest)
        ..data  (\src)
        .if     ((\short) != 1)
        .if     ((\short) != 4)
        .if     ((\short) != 8)
        .if     ((\short) != 16)
        .if     ((\short) != 32)
        .error  "The legal ROLS shift amounts are 1, 4, 8, 16 and 32"
        .endif
        .endif
        .endif
        .endif
        .endif
        ..instruction PGAS_OPCODE_ROLS, (((\dest) << 20) | ((\src) << 16) | (\short))
        .endm

        .macro  rol, args:vararg
        .error "PGAS implements the 'rols' mnemonic instead of PORE 'rol'"
        .endm

        // LS
        
        .macro  ls, dest:req, short:req
        ..ls_destination (\dest)
        ..check_s20 (\short)
        ..instruction PGAS_OPCODE_LS, (((\dest) << 20) | ((\short) & 0xfffff))
        .endm

        .macro  load20, args:vararg
        .error  "PGAS implements the 'ls' mnemonic instead of PORE 'load20'"
        .endm
        
        // LI, LIA

        .macro  ..li, dest:req
        ..li_destination (\dest)
        ..instruction PGAS_OPCODE_LI, ((\dest) << 20)
        .endm
                
        .macro  li, dest:req, imm:req
        ..li    (\dest)
        .quad   (\imm)
        .endm

        .macro  lia, dest:req, space:req, offset:req
        ..lia_destination (\dest)
        ..li    (\dest)
        .quadia (\space), (\offset)
        .endm

        .macro  load64, args:vararg
        .error  "PGAS implements the 'li' mnemonic instead of PORE 'load64'"
        .endm

        // LD, LDANDI, STD, STI, STIA, BSI, BCI
        //
        // For LD, LDANDI, and STD, PGAS does not expose the underlying
        // register-specific opcodes but only provides the general form.  
        //
        // The base register is used to determine if this is a load/store from
        // the pervasive or memory address spaces.  For memory space accesses
        // the offset is a 22-bit unsigned value, and the final ima24 is
        //
        //     1<reg 0/1><offset>
        //
        // PGAS will not assemble relocatable offsets, and checks that offsets
        // fit in 24 bits.
        //
        // For pervasive accesses, it is assumed that the offset provided is a
        // 32-bit SCOM address.  Here the final ima24 is
        // 
        //     0<reg 0/1>00<port><local_address>
        //
        // PGAS checks that the 32-bit SCOM address looks like a SCOM address
        // in that SCOM adresses are required to have bits 0 and 8:11 == 0.
        //
        // Note that memory and pervasive base registers use a 0/1 encoding
        // here, not the 4-bit encoding used elsewhere in the ISA. The bit
        // appearing in the instruction is the low-order bit of the register
        // encoding.
        
        .macro  ..pervasive_ima24, opcode, offset, base
        ..check_scom (\offset)
        ..instruction (\opcode), ((((\base) % 2) << 22) | ((\offset) & 0x3fffff))
        .endm

        .macro  ..memory_ima24, opcode, offset, base
        ..check_u24 (\offset)
        .if     ((\offset) % 8)
        .error  "The memory space offset is not a multiple of 8 - assumed alignment error"
        .endif
        ..instruction (\opcode), (0x800000 | (((\base) % 2) << 22) | ((\offset) & 0x3fffff))
        .endm

        .macro  ..ima24, opcode, offset, base
        .if     ((\base == P0) || ((\base == P1)))
        ..pervasive_ima24 (\opcode), (\offset), (\base)
        .elseif ((\base == A0) || ((\base == A1)))
        ..memory_ima24 (\opcode), (\offset), (\base)
        .else
        .error  "Expecting either a 'Pervasive Chiplet ID' or an 'Address' register"
        .endif
        .endm

        .macro  ..ima24_select, opcode0, opcode1, dest, offset, base
        ..data  (\dest)
        .if     ((\dest) == D0)
        ..ima24 (\opcode0), (\offset), (\base)
        .else
        ..ima24 (\opcode1), (\offset), (\base)
        .endif
        .endm

        .macro  ld, dest:req, offset:req, base:req
        ..ima24_select PGAS_OPCODE_LD0, PGAS_OPCODE_LD1, (\dest), (\offset), (\base)
        .endm
        
        .macro  ldandi, dest:req, offset:req, base:req, imm:req
        ..ima24_select PGAS_OPCODE_LD0ANDI, PGAS_OPCODE_LD1ANDI, (\dest), (\offset), (\base)
        .quad   (\imm)
        .endm

        .macro  std, dest:req, offset:req, base:req
        ..ima24_select PGAS_OPCODE_STD0, PGAS_OPCODE_STD1, (\dest), (\offset), (\base)
        .endm

        .macro  sti, offset:req, base:req, imm:req
        ..ima24 PGAS_OPCODE_STI, (\offset), (\base)
        .quad   (\imm)
        .endm

        .macro  stia, offset:req, base:req, space:req, addr:req
        ..ima24 PGAS_OPCODE_STI, (\offset), (\base)
        .quadia (\space), (\addr)
        .endm

        .macro  ..bsi, opcode, dest, offset, base, imm
        ..d0    (\dest)
        ..ima24 (\opcode), (\offset), (\base)
        .quad   (\imm)
        .endm

#ifdef IGNORE_HW274735

        // BSI and BCI are normally redacted due to HW274735. See also pgas.h
        
        .macro  bsi, dest:req, offset:req, base:req, imm:req
        ..bsi   PGAS_OPCODE_BSI, (\dest), (\offset), (\base), (\imm)
        .endm
        
        .macro  bci, dest:req, offset:req, base:req, imm:req
        ..bsi   PGAS_OPCODE_BCI, (\dest), (\offset), (\base), (\imm)
        .endm

#endif // IGNORE_HW274735

        .macro  scr1rd, args:vararg
        .error  "PGAS implements the 'ld' mnemonic instead of the PORE 'scr1rd'"
        .endm

        .macro  scr2rd, args:vararg
        .error  "PGAS implements the 'ld' mnemonic instead of the PORE 'scr2rd'"
        .endm

        .macro  scr1rda, args:vararg
        .error  "PGAS implements the 'ldandi' mnemonic instead of the PORE 'scr1rda'"
        .endm

        .macro  scr2rda, args:vararg
        .error  "PGAS implements the 'ldandi' mnemonic instead of the PORE 'scr2rda'"
        .endm

        .macro  scr1wr, args:vararg
        .error  "PGAS implements the 'std' mnemonic instead of the PORE 'scr1wr'"
        .endm

        .macro  scr2wr, args:vararg
        .error  "PGAS implements the 'std' mnemonic instead of the PORE 'scr2wr'"
        .endm

        .macro  wri, args:vararg
        .error  "PGAS implements the 'sti' mnemonic instead of the PORE 'wri'"
        .endm

        .macro  bs, args:vararg
        .error  "PGAS implements the 'bsi' mnemonic instead of the PORE 'bs'"
        .endm

        .macro  bc, args:vararg
        .error  "PGAS implements the 'bci' mnemonic instead of the PORE 'bc'"
        .endm

        // SCAND
        //
        // The 24-bit operand here is
        //
        //     <update><capture>000000<length>

        .macro  scand, update:req, capture:req, length:req, select:req, offset:req
        .if     (((\update) != 0) && ((\update) != 1))
        .error  "SCAND requires a binary value for 'update'"
        .endif
        .if     (((\capture) != 0) && ((\capture) != 1))
        .error  "SCAND requires a binary value for 'capture'"
        .endif
        ..check_u16 (\length)
        ..instruction PGAS_OPCODE_SCAND, ((\update << 23) | (\capture << 22) | (\length))
        .long   (\select)
        .long   (\offset)
        .endm

        // BRAIA, BSR, CMPIBSREQ
        //
        // In order to support separate compilation in PGAS programs being
        // linked with the PowerPC linker it is necessary to implement BSR and
        // CMPIBSREQ in terms of BRAIA.  These instructions require that the
        // default address space have been defined.  The BSR instructions
        // first take a short local subroutine branch to create a stack frame,
        // then use BRAIA to branch to the (relocatable) target address. The
        // return from the subroutine then branches around the BRAIA to
        // complete the sequence.

        .macro  braia, space:req, offset:req
        ..instruction PGAS_OPCODE_BRAI, 0
        .quadia (\space), (\offset)
        .endm

        .macro  ..bsr, target
        ..bra   PGAS_OPCODE_BSR, (\target)
        .endm
        
        .macro  bsr, target:req
        ..check_default_space
        ..bsr   (. + 8)
        bra     (. + 16)
        braia   _PGAS_DEFAULT_SPACE, (\target)
        .endm

        .macro  cmpibsreq, src:req, target:req, imm:req
        ..d0    (\src)
        ..check_default_space
        cmpibrane (\src), (. + 32), (\imm)
        ..bsr   (. + 8)
        bra     (. + 16)
        braia   _PGAS_DEFAULT_SPACE, (\target)
        .endm

#endif  // __ASSEMBLER__
                    
#endif // __PGAS_PPC_H__
