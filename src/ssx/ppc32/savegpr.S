// $Id: savegpr.S,v 1.1.1.1 2013/12/11 21:03:25 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ppc32/savegpr.S,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

///
/// \file savegpr.S
/// \brief Push registers onto or load registers from the stack 
///
/// This code was developed due to missing references to _savegpr_{X}
/// and _restgpr_{X}_x that surfaced when moving to GCC 4.5.1 and 
/// using the -Os optimization level.
/// 
/// The open-source code was viewed to get the spec, but no 
/// code was copied.
///


        .macro SAVE reg
                .global _savegpr_\reg
        _savegpr_\reg\(): 
                stw \reg,(((31-\reg)*4 +4)*-1)(11)
        .endm

        SAVE 14
        SAVE 15
        SAVE 16
        SAVE 17
        SAVE 18
        SAVE 19
        SAVE 20
        SAVE 21
        SAVE 22
        SAVE 23
        SAVE 24
        SAVE 25
        SAVE 26
        SAVE 27
        SAVE 28
        SAVE 29
        SAVE 30
        SAVE 31
        blr
        
        .macro RESTX reg
                .global _restgpr_\reg\()_x
        _restgpr_\reg\()_x: 
                .ifeq (31-\reg)
                        lwz 0,4(11)
                .endif
                lwz \reg,(((31-\reg)*4 +4)*-1)(11)
                        
        .endm
        
        RESTX 14
        RESTX 15
        RESTX 16
        RESTX 17
        RESTX 18
        RESTX 19
        RESTX 20
        RESTX 21
        RESTX 22
        RESTX 23
        RESTX 24
        RESTX 25
        RESTX 26
        RESTX 27
        RESTX 28
        RESTX 29
        RESTX 30
        RESTX 31
        mtlr 0
        mr 1,11
        blr
