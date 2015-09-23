/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/tools/scripts/src/fapi2PlatAttributeService.H $       */
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
/**
 *  @file fapiPlatAttributeService.H
 *
 *  @brief Defines the PLAT attribute access macros and defines which macro
 *         handles each attribute.
 *
 */


#ifndef FAPI2PLATATTRIBUTESERVICE_H_
#define FAPI2PLATATTRIBUTESERVICE_H_

#include <stdint.h>
#include <stddef.h>
#include <fapi2AttributeIds.H>
#include <plat_includes.H>
#include "proc_sbe_fixed.H"
#include "plat_target_parms.H"

#define PLAT_GET_CHIP_EC_FEATURE_OVERRIDE(ID, PTARGET, VAL) \
      fapi2::_getEcFeatureOverride<fapi2::ID##_Type>(fapi2::ID, PTARGET, VAL)

/* INSERT NEW ATTRIBUTES HERE */

#define ATTR_TARGET_SCOMABLE_GETMACRO PLAT_ATTR_GET_GLOBAL_INT
#define ATTR_TARGET_SCOMABLE_SETMACRO PLAT_ATTR_SET_GLOBAL_INT



/******************************************************************************/
/*                          * Global macros *                                 */
/* These macros are called by the macros above to invoke the appropriate API. */
/* These macros do not need to change when a new attribute is introduced.     */
/******************************************************************************/

/* global get uint8_t 1D array macro */
#define PLAT_ATTR_GET_UINT8_1D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeArrayShort<fapi2::ID##_Type, static_cast<TargetType>(fapi2::ID##_TargetTypes), fapi2::ID> \
            (fapi2::ID, PTARGET, VAL)    
     
/* global set uint8_t 1D array macro */
#define PLAT_ATTR_SET_UINT8_1D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_setAttributeArrayShort<fapi2::ID##_Type, static_cast<TargetType>(fapi2::ID##_TargetTypes), fapi2::ID> \
            (fapi2::ID, PTARGET, VAL)   

/* global get uint8_t 2D array macro */
#define PLAT_ATTR_GET_UINT8_2D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeArrayShort(fapi2::ID, PTARGET, VAL[0])
/* global set uint8_t 2D array macro */
#define PLAT_ATTR_SET_UINT8_2D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_setAttributeArrayShort(fapi2::ID, PTARGET, VAL[0])

/* global get uint8_t 3D array macro */
#define PLAT_ATTR_GET_UINT8_3D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeArrayShort(fapi2::ID, PTARGET, VAL[0][0])
/* global set uint8_t 3D array macro */
#define PLAT_ATTR_SET_UINT8_3D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_setAttributeArrayShort(fapi2::ID, PTARGET, VAL[0][0])

/* global get uint8_t 4D array macro */
#define PLAT_ATTR_GET_UINT8_4D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeArrayShort(fapi2::ID, PTARGET, VAL[0][0][0])
/* global set uint8_t 4D array macro */
#define PLAT_ATTR_SET_UINT8_4D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_setAttributeArrayShort(fapi2::ID, PTARGET, VAL[0][0][0])

/* global get uint32_t 1D array macro */
#define PLAT_ATTR_GET_UINT32_1D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeArrayWord(fapi2::ID, PTARGET, VAL)
/* global set uint32_t 1D array macro */
#define PLAT_ATTR_SET_UINT32_1D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_setAttributeArrayWord(fapi2::ID, PTARGET, VAL)

/* global get uint32_t 2D array macro */
#define PLAT_ATTR_GET_UINT32_2D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeArrayWord(fapi2::ID, PTARGET, VAL[0])
/* global set uint32_t 2D array macro */
#define PLAT_ATTR_SET_UINT32_2D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_setAttributeArrayWord(fapi2::ID, PTARGET, VAL[0])

/* global get uint32_t 3D array macro */
#define PLAT_ATTR_GET_UINT32_3D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeArrayWord(fapi2::ID, PTARGET, VAL[0][0])
/* global set uint32_t 3D array macro */
#define PLAT_ATTR_SET_UINT32_3D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_setAttributeArrayWord(fapi2::ID, PTARGET, VAL[0][0])

/* global get uint32_t 4D array macro */
#define PLAT_ATTR_GET_UINT32_4D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeArrayWord(fapi2::ID, PTARGET, VAL[0][0][0])
/* global set uint32_t 4D array macro */
#define PLAT_ATTR_SET_UINT32_4D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_setAttributeArrayWord(fapi2::ID, PTARGET, VAL[0][0][0])


/* global get uint64_t 1D array macro */
#define PLAT_ATTR_GET_UINT64_1D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeArrayDoubleWord(fapi2::ID, PTARGET, VAL)
/* global set uint64_t 1D array macro */
#define PLAT_ATTR_SET_UINT64_1D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_setAttributeArrayDoubleWord(fapi2::ID, PTARGET, VAL)

/* global get uint64_t 2D array macro */
#define PLAT_ATTR_GET_UINT64_2D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeArrayDoubleWord(fapi2::ID, PTARGET, VAL[0])
/* global set uint64_t 2D array macro */
#define PLAT_ATTR_SET_UINT64_2D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_setAttributeArrayDoubleWord(fapi2::ID, PTARGET, VAL[0])

/* global get uint64_t 3D array macro */
#define PLAT_ATTR_GET_UINT64_3D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeArrayDoubleWord(fapi2::ID, PTARGET, VAL[0][0])
/* global set uint64_t 3D array macro */
#define PLAT_ATTR_SET_UINT64_3D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_setAttributeArrayDoubleWord(fapi2::ID, PTARGET, VAL[0][0])

/* global get uint64_t 4D array macro */
#define PLAT_ATTR_GET_UINT64_4D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeArrayDoubleWord(fapi2::ID, PTARGET, VAL[0][0][0])
/* global set uint64_t 4D array macro */
#define PLAT_ATTR_SET_UINT64_4D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_setAttributeArrayDoubleWord(fapi2::ID, PTARGET, VAL[0][0][0])

/* global get int macro (uint8_t, 32 and 64) */
#define PLAT_ATTR_GET_GLOBAL_INT(ID, PTARGET, VAL) \
     fapi2::_get<fapi2::ID##_Type, static_cast<TargetType>(fapi2::ID##_TargetTypes), fapi2::ID> \
                (fapi2::ID, PTARGET, VAL)

/* global set int macro (uint8_t, 32 and 64) */
#define PLAT_ATTR_SET_GLOBAL_INT(ID, PTARGET, VAL) \
     fapi2::_set<fapi2::ID##_Type, static_cast<TargetType>(fapi2::ID##_TargetTypes), fapi2::ID> \
                (fapi2::ID, PTARGET, VAL)


//here

/******************************************************************************/
// Get Override Macros
/******************************************************************************/
/* global get override uint8_t 1D array macro */
#define PLAT_ATTR_GET_OVERRIDE_UINT8_1D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeOverrideArrayShort(fapi2::ID, PTARGET, VAL)
/* global get override uint8_t 2D array macro */
#define PLAT_ATTR_GET_OVERRIDE_UINT8_2D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeOverrideArrayShort(fapi2::ID, PTARGET, VAL[0])
/* global get override uint8_t 3D array macro */
#define PLAT_ATTR_GET_OVERRIDE_UINT8_3D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeOverrideArrayShort(fapi2::ID, PTARGET, VAL[0][0])
/* global get override uint8_t 4D array macro */
#define PLAT_ATTR_GET_OVERRIDE_UINT8_4D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeOverrideArrayShort(fapi2::ID, PTARGET, VAL[0][0][0])


/* global get override uint32_t 1D array macro */
#define PLAT_ATTR_GET_OVERRIDE_UINT32_1D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeOverrideArrayWord(fapi2::ID, PTARGET, VAL)
/* global get override uint32_t 2D array macro */
#define PLAT_ATTR_GET_OVERRIDE_UINT32_2D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeOverrideArrayWord(fapi2::ID, PTARGET, VAL[0])
/* global get override uint32_t 3D array macro */
#define PLAT_ATTR_GET_OVERRIDE_UINT32_3D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeOverrideArrayWord(fapi2::ID, PTARGET, VAL[0][0])
/* global get override uint32_t 4D array macro */
#define PLAT_ATTR_GET_OVERRIDE_UINT32_4D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeOverrideArrayWord(fapi2::ID, PTARGET, VAL[0][0][0])


/* global get override uint64_t 1D array macro */
#define PLAT_ATTR_GET_OVERRIDE_UINT64_1D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeOverrideArrayDoubleWord(fapi2::ID, PTARGET, VAL)
/* global get override uint64_t 2D array macro */
#define PLAT_ATTR_GET_OVERRIDE_UINT64_2D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeOverrideArrayDoubleWord(fapi2::ID, PTARGET, VAL[0])
/* global get override uint64_t 3D array macro */
#define PLAT_ATTR_GET_OVERRIDE_UINT64_3D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeOverrideArrayDoubleWord(fapi2::ID, PTARGET, VAL[0][0])
/* global get override uint64_t 4D array macro */
#define PLAT_ATTR_GET_OVERRIDE_UINT64_4D_ARRAY(ID, PTARGET, VAL) \
     fapi2::_getAttributeOverrideArrayDoubleWord(fapi2::ID, PTARGET, VAL[0][0][0])

/* global get override int macro (uint8_t, 32 and 64) */
#define PLAT_ATTR_GET_OVERRIDE_GLOBAL_INT(ID, PTARGET, VAL) \
     fapi2::_getOverride<fapi2::ID##_Type>(fapi2::ID, PTARGET, VAL)

/******************************************************************************/
// Get string 
/******************************************************************************/

extern "C" 
{ 
    extern fapi2attr::ProcChipAttributes_t* G_proc_chip_attributes asm("G_proc_chip_attributes")  __attribute__ ((section (".fixed")));
    extern fapi2attr::PervAttributes_t*     G_perv_attributes      asm("G_perv_attributes")  __attribute__ ((section (".fixed")));
    extern fapi2attr::CoreAttributes_t*     G_core_attributes      asm("G_core_attributes")        __attribute__ ((section (".fixed")));
    extern fapi2attr::EQAttributes_t*       G_eq_attributes        asm("G_eq_attributes")          __attribute__ ((section (".fixed")));
    extern fapi2attr::EXAttributes_t*       G_ex_attributes        asm("G_ex_attributes")          __attribute__ ((section (".fixed")));

}

namespace fapi2 
{


// Parameters are done as pointers (vs references) to allow the attribute 
// storage to be relocated
template<fapi2::TargetType K, typename TAttrStruct, typename TValue, fapi2::AttributeId AId>
void __set( const fapi2::Target<K>* i_ptarget, TAttrStruct* object, const fapi2::AttributeId attrid, TValue* value );

template<fapi2::TargetType K, typename TAttrStruct, typename TValue, fapi2::AttributeId AId>
void __get( const fapi2::Target<K>* i_ptarget, TAttrStruct* object, const fapi2::AttributeId attrid, TValue* value );


/* INSERT NEW GETTER AND SETTER FUNCTIONS HERE */



//******************************************************************************
// Get base template
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _get(const AttributeId i_Id,
                const Target<K> * const i_pTarget,
                T& o_value)
{
    return FAPI2_RC_SUCCESS;    
}

//******************************************************************************
// Get uint8_t 
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _get(const fapi2::AttributeId i_id,
                const Target<K> * const i_pTarget,
                uint8_t& o_value)
{
    static_assert(std::is_same<T, uint8_t>::value, "Attribute type mismatch");

    if(K & TARGET_TYPE_PROC_CHIP)
    {
        __get<K, fapi2attr::ProcChipAttributes_t, T, A>( i_pTarget, G_proc_chip_attributes, i_id, &o_value );  
    }

    if(K & TARGET_TYPE_PERV)
    {
         __get<K, fapi2attr::PervAttributes_t, T, A>( i_pTarget, G_perv_attributes, i_id, &o_value );  
    }

    if(K & TARGET_TYPE_CORE)
    {
        __get<K, fapi2attr::CoreAttributes_t, T, A>( i_pTarget, G_core_attributes, i_id, &o_value );  
    }

    if(K & TARGET_TYPE_EQ)
    {
        __get<K, fapi2attr::EQAttributes_t, T, A>( i_pTarget, G_eq_attributes, i_id, &o_value );  
    }
    
    if(K & TARGET_TYPE_EX)
    {                                           
        __get<K, fapi2attr::EXAttributes_t, T, A>( i_pTarget, G_ex_attributes, i_id, &o_value );                     
    }        
 
    return FAPI2_RC_SUCCESS;
}


//******************************************************************************
// Get uint32_t 
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _get(const AttributeId i_id,
                const Target<K> * const i_pTarget,
                uint32_t& o_value)
{
    static_assert(std::is_same<T, uint32_t>::value, "Attribute type mismatch");
    
    if(K & TARGET_TYPE_PROC_CHIP)
    {
        __get<K, fapi2attr::ProcChipAttributes_t, T, A>( i_pTarget, G_proc_chip_attributes, i_id, &o_value );  
    }

    if(K & TARGET_TYPE_PERV)
    {
         __get<K, fapi2attr::PervAttributes_t, T, A>( i_pTarget, G_perv_attributes, i_id, &o_value );  
    }

    if(K & TARGET_TYPE_CORE)
    {
        __get<K, fapi2attr::CoreAttributes_t, T, A>( i_pTarget, G_core_attributes, i_id, &o_value );  
    }

    if(K & TARGET_TYPE_EQ)
    {
        __get<K, fapi2attr::EQAttributes_t, T, A>( i_pTarget, G_eq_attributes, i_id, &o_value );  
    }
    
    if(K & TARGET_TYPE_EX)
    {                                           
        __get<K, fapi2attr::EXAttributes_t, T, A>( i_pTarget, G_ex_attributes, i_id, &o_value );                     
    }        
 
    return FAPI2_RC_SUCCESS;
}


//******************************************************************************
// Get uint64_t 
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _get(const AttributeId i_id,
                const Target<K> * const i_pTarget,
                uint64_t& o_value)
{
    static_assert(std::is_same<T, uint64_t>::value, "Attribute type mismatch");
    

    if(K & TARGET_TYPE_PROC_CHIP)
    {
        __get<K, fapi2attr::ProcChipAttributes_t, T, A>( i_pTarget, G_proc_chip_attributes, i_id, &o_value );  
    }

    if(K & TARGET_TYPE_PERV)
    {
         __get<K, fapi2attr::PervAttributes_t, T, A>( i_pTarget, G_perv_attributes, i_id, &o_value );  
    }

    if(K & TARGET_TYPE_CORE)
    {
        __get<K, fapi2attr::CoreAttributes_t, T, A>( i_pTarget, G_core_attributes, i_id, &o_value );  
    }

    if(K & TARGET_TYPE_EQ)
    {
        __get<K, fapi2attr::EQAttributes_t, T, A>( i_pTarget, G_eq_attributes, i_id, &o_value );  
    }
    
    if(K & TARGET_TYPE_EX)
    {                                           
        __get<K, fapi2attr::EXAttributes_t, T, A>( i_pTarget, G_ex_attributes, i_id, &o_value );                     
    }        

    return FAPI2_RC_SUCCESS;    
}


//******************************************************************************
// Get Override uint8_t 
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _getOverride(const AttributeId i_id,
                        const Target<K> * const i_pTarget,
                        uint8_t& o_value)
{
    static_assert(std::is_same<T, uint8_t>::value, "Attribute type mismatch");

    return FAPI2_RC_SUCCESS;    
}


//******************************************************************************
// Get Override uint32_t 
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _getOverride(const AttributeId i_id,
                        const Target<K> * const i_pTarget,
                        uint32_t& o_value)
{
    static_assert(std::is_same<T, uint32_t>::value, "Attribute type mismatch");

    return FAPI2_RC_SUCCESS;   
}


//******************************************************************************
// Get Override uint64_t 
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _getOverride(const AttributeId i_id,
                        const Target<K> * const i_pTarget,
                        uint64_t& o_value)
{
    static_assert(std::is_same<T, uint64_t>::value, "Attribute type mismatch");

    return FAPI2_RC_SUCCESS;    
}


//******************************************************************************
// Get override EC Feature (uint8_t)
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _getEcFeatureOverride(const AttributeId i_id,
                                 const Target<K> * const i_pTarget,
                                 uint8_t& o_value)
{
    static_assert(std::is_same<T, uint8_t>::value, "Attribute type mismatch");

    // The way this is implemented, we want to return a non-zero return code if we found an override.  
    // Return 0 if there was an error.
    // This is how it's implemented:
    // PLAT_GET_CHIP_EC_FEATURE_OVERRIDE(ID, PTARGET, VAL) ? fapi::FAPI_RC_SUCCESS : fapi::fapiQueryChipEcFeature(fapi::ID, PTARGET, VAL)

    return FAPI2_RC_SUCCESS;    
}


//******************************************************************************
// Get uint8_t array
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _getAttributeArrayShort(const AttributeId i_id,
                                   const Target<K> * const i_pTarget,
                                   uint8_t * o_pValues)
{
//       fapi2::Attributeta o_data;
//     fapi2::ReturnCode l_fapi_rc(FAPI2_RC_SUCCESS); 
//     uint32_t l_ecmd_rc = ECMD_SUCCESS;
// 
//     ecmdChipTarget l_ecmd_target;
//     fapiTargetPointerToEcmdTarget(i_pTarget, l_ecmd_target);
// 
//     o_data.faValidMask = FAPI_ATTRIBUTE_TYPE_UINT8ARY;
//     o_data.faUint8ary = o_pValues;
// 
//     l_ecmd_rc = fapi2GetAttribute(l_ecmd_target, i_id, o_data); 
//     if (l_ecmd_rc)
//     {
//         l_fapi_rc = (ReturnCodes) l_ecmd_rc;
//     }
//     return l_fapi_rc;
    return FAPI2_RC_SUCCESS;
}

//******************************************************************************
// Set uint8_t array
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _setAttributeArrayShort(const AttributeId i_id,
                                   const Target<K> * const i_pTarget,
                                   uint8_t * i_pValues)
{
    if(K & TARGET_TYPE_PROC_CHIP)
    {
        __set<K, fapi2attr::CoreAttributes_t, T, A>( *i_pTarget, G_proc_chip_attributes, *i_pValues );  
    }

    if(K & TARGET_TYPE_PERV)
    {
        __set<K, fapi2attr::PervAttributes_t, T, A>( *i_pTarget, G_perv_attributes, i_pValues );  
    }

    if(K & TARGET_TYPE_CORE)
    {
        __set<K, fapi2attr::CoreAttributes_t, T, A>( *i_pTarget, G_core_attributes, *i_pValues );  
    }

    if(K & TARGET_TYPE_EQ)
    {
        __set<K, fapi2attr::EQAttributes_t, T, A>( *i_pTarget, G_eq_attributes, *i_pValues );  
    }
    
    if(K & TARGET_TYPE_EX)
    {                                           
        __set<K, fapi2attr::EXAttributes_t, T, A>( *i_pTarget, G_ex_attributes, *i_pValues );                         
    }
    
    return FAPI2_RC_SUCCESS;
}

//******************************************************************************
// Get uint32_t array
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _getAttributeArrayWord(const AttributeId i_id,
                                  const Target<K> * const i_pTarget,
                                  uint32_t * o_pValues)
{
    return FAPI2_RC_SUCCESS;
}

//******************************************************************************
// Set uint32_t array
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _setAttributeArrayWord(const AttributeId i_id,
                                  const Target<K> * const i_pTarget,
                                  uint32_t * i_pValues)
{
    return FAPI2_RC_SUCCESS;
}

//******************************************************************************
// Get uint64_t array
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _getAttributeArrayDoubleWord(const AttributeId i_id,
                                        const Target<K> * const i_pTarget,
                                        uint64_t * o_pValues)
{
    return FAPI2_RC_SUCCESS;
}

//******************************************************************************
// Set uint64_t array
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _setAttributeArrayDoubleWord(const AttributeId i_id,
                                        const Target<K> * const i_pTarget,
                                        uint64_t * i_pValues)
{
    return FAPI2_RC_SUCCESS;
}

//******************************************************************************
// Get Override uint8_t array
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _getAttributeOverrideArrayShort(const AttributeId i_id,
                                           const Target<K> * const i_pTarget,
                                           uint8_t * o_pValues)
{
    return FAPI2_RC_SUCCESS;
}

//******************************************************************************
// Get Override uint32_t array
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _getAttributeOverrideArrayWord(const AttributeId i_id,
                                          const Target<K> * const i_pTarget,
                                          uint32_t * o_pValues)
{
    return FAPI2_RC_SUCCESS;
}

//******************************************************************************
// Get Override uint64_t array
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _getAttributeOverrideArrayDoubleWord(const AttributeId i_id,
                                                const Target<K> * const i_pTarget,
                                                uint64_t * o_pValues)
{
    return FAPI2_RC_SUCCESS;
}


//******************************************************************************
// Set base template
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _set(const AttributeId i_Id,
                const Target<K> * const i_pTarget,
                T& i_value)
{
    return FAPI2_RC_SUCCESS;    
}



//******************************************************************************
// Set uint8_t 
//******************************************************************************
template<typename T, TargetType K, typename A>
ReturnCode _set(const AttributeId i_Id,
                const Target<K> * const i_pTarget,
                uint8_t& i_value)
{
    static_assert(std::is_same<T, uint8_t>::value, "Attribute type mismatch"); // May need to remove

    if(K & TARGET_TYPE_PROC_CHIP)
    {
        __set<K, fapi2attr::CoreAttributes_t, T, A>( *i_pTarget, G_proc_chip_attributes, i_value );  
    }

    if(K & TARGET_TYPE_PERV)
    {
        __set<K, fapi2attr::PervAttributes_t, T, A>( *i_pTarget, G_perv_attributes, i_value );  
    }

    if(K & TARGET_TYPE_CORE)
    {
        __set<K, fapi2attr::CoreAttributes_t, T, A>( *i_pTarget, G_core_attributes, i_value );  
    }

    if(K & TARGET_TYPE_EQ)
    {
        __set<K, fapi2attr::EQAttributes_t, T, A>( *i_pTarget, G_eq_attributes, i_value );  
    }
    
    if(K & TARGET_TYPE_EX)
    {                                           
        __set<K, fapi2attr::EXAttributes_t, T, A>( *i_pTarget, G_ex_attributes, i_value );                         
    }
    
    return FAPI2_RC_SUCCESS;
}


//******************************************************************************
// Set uint32_t 
//******************************************************************************
template<typename T, TargetType K, typename A>
ReturnCode _set(
                const Target<K> * const i_pTarget,
                uint32_t& i_value)
{
    static_assert(std::is_same<T, uint32_t>::value, "Attribute type mismatch"); // May need to remove

    if(K & TARGET_TYPE_PROC_CHIP)
    {
        __set<K, fapi2attr::CoreAttributes_t, T, A>( *i_pTarget, G_proc_chip_attributes, i_value );  
    }

    if(K & TARGET_TYPE_PERV)
    {
        __set<K, fapi2attr::PervAttributes_t, T, A>( *i_pTarget, G_perv_attributes, i_value );  
    }

    if(K & TARGET_TYPE_CORE)
    {
        __set<K, fapi2attr::CoreAttributes_t, T, A>( *i_pTarget, G_core_attributes, i_value );  
    }

    if(K & TARGET_TYPE_EQ)
    {
        __set<K, fapi2attr::EQAttributes_t, T, A>( *i_pTarget, G_eq_attributes, i_value );  
    }
    
    if(K & TARGET_TYPE_EX)
    {                                           
        __set<K, fapi2attr::EXAttributes_t, T, A>( *i_pTarget, G_ex_attributes, i_value );                         
    }
    
    return FAPI2_RC_SUCCESS;
}


//******************************************************************************
// Set uint64_t 
//******************************************************************************

template<typename T, TargetType K, AttributeId A>
ReturnCode _set(const AttributeId i_Id,
                const Target<K> * const i_pTarget,
                const uint64_t & i_value)
{
    static_assert(std::is_same<T, uint64_t>::value, "Attribute type mismatch"); // May need to remove

    if(K & TARGET_TYPE_PROC_CHIP)
    {
        __set<K, fapi2attr::CoreAttributes_t, T, A>( *i_pTarget, G_proc_chip_attributes, i_value );  
    }

    if(K & TARGET_TYPE_PERV)
    {
        __set<K, fapi2attr::PervAttributes_t, T, A>( *i_pTarget, G_perv_attributes, i_value );  
    }

    if(K & TARGET_TYPE_CORE)
    {
        __set<K, fapi2attr::CoreAttributes_t, T, A>( *i_pTarget, G_core_attributes, i_value );  
    }

    if(K & TARGET_TYPE_EQ)
    {
        __set<K, fapi2attr::EQAttributes_t, T, A>( *i_pTarget, G_eq_attributes, i_value );  
    }
    
    if(K & TARGET_TYPE_EX)
    {                                           
        __set<K, fapi2attr::EXAttributes_t, T, A>( *i_pTarget, G_ex_attributes, i_value );                         
    }
    
    return FAPI2_RC_SUCCESS;
}


//******************************************************************************
// Get int8_t 
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _get(const AttributeId i_id,
                const Target<K> * const i_pTarget,
                int8_t& o_value)
{
    static_assert(std::is_same<T, int8_t>::value, "Attribute type mismatch");

    if(K & TARGET_TYPE_PROC_CHIP)
    {
        __get<K, fapi2attr::ProcChipAttributes_t, T, A>( i_pTarget, G_proc_chip_attributes, i_id, &o_value );  
    }

    if(K & TARGET_TYPE_PERV)
    {
         __get<K, fapi2attr::PervAttributes_t, T, A>( i_pTarget, G_perv_attributes, i_id, &o_value );  
    }

    if(K & TARGET_TYPE_CORE)
    {
        __get<K, fapi2attr::CoreAttributes_t, T, A>( i_pTarget, G_core_attributes, i_id, &o_value );  
    }

    if(K & TARGET_TYPE_EQ)
    {
        __get<K, fapi2attr::EQAttributes_t, T, A>( i_pTarget, G_eq_attributes, i_id, &o_value );  
    }
    
    if(K & TARGET_TYPE_EX)
    {                                           
        __get<K, fapi2attr::EXAttributes_t, T, A>( i_pTarget, G_ex_attributes, i_id, &o_value );                     
    }        

    return FAPI2_RC_SUCCESS;    
}


//******************************************************************************
// Get int32_t 
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _get(const AttributeId i_id,
                const Target<K> * const i_pTarget,
                int32_t& o_value)
{
    static_assert(std::is_same<T, int32_t>::value, "Attribute type mismatch");

    if(K & TARGET_TYPE_PROC_CHIP)
    {
        __get<K, fapi2attr::ProcChipAttributes_t, T, A>( i_pTarget, G_proc_chip_attributes, i_id, &o_value );  
    }

    if(K & TARGET_TYPE_PERV)
    {
         __get<K, fapi2attr::PervAttributes_t, T, A>( i_pTarget, G_perv_attributes, i_id, &o_value );  
    }

    if(K & TARGET_TYPE_CORE)
    {
        __get<K, fapi2attr::CoreAttributes_t, T, A>( i_pTarget, G_core_attributes, i_id, &o_value );  
    }

    if(K & TARGET_TYPE_EQ)
    {
        __get<K, fapi2attr::EQAttributes_t, T, A>( i_pTarget, G_eq_attributes, i_id, &o_value );  
    }
    
    if(K & TARGET_TYPE_EX)
    {                                           
        __get<K, fapi2attr::EXAttributes_t, T, A>( i_pTarget, G_ex_attributes, i_id, &o_value );                     
    }        

    return FAPI2_RC_SUCCESS;    
}


//******************************************************************************
// Get int64_t 
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _get(const AttributeId i_id,
                const Target<K> * const i_pTarget,
                int64_t& o_value)
{
    static_assert(std::is_same<T, int64_t>::value, "Attribute type mismatch");

    if(K & TARGET_TYPE_PROC_CHIP)
    {
        __get<K, fapi2attr::ProcChipAttributes_t, T, A>( i_pTarget, G_proc_chip_attributes, i_id, &o_value );  
    }

    if(K & TARGET_TYPE_PERV)
    {
         __get<K, fapi2attr::PervAttributes_t, T, A>( i_pTarget, G_perv_attributes, i_id, &o_value );  
    }

    if(K & TARGET_TYPE_CORE)
    {
        __get<K, fapi2attr::CoreAttributes_t, T, A>( i_pTarget, G_core_attributes, i_id, &o_value );  
    }

    if(K & TARGET_TYPE_EQ)
    {
        __get<K, fapi2attr::EQAttributes_t, T, A>( i_pTarget, G_eq_attributes, i_id, &o_value );  
    }
    
    if(K & TARGET_TYPE_EX)
    {                                           
        __get<K, fapi2attr::EXAttributes_t, T, A>( i_pTarget, G_ex_attributes, i_id, &o_value );                     
    }        

    return FAPI2_RC_SUCCESS;    
}


//******************************************************************************
// Get Override int8_t 
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _getOverride(const AttributeId i_id,
                        const Target<K> * const i_pTarget,
                        int8_t& o_value)
{
    static_assert(std::is_same<T, int8_t>::value, "Attribute type mismatch");

    return FAPI2_RC_SUCCESS;
}


//******************************************************************************
// Get Override int32_t 
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _getOverride(const AttributeId i_id,
                        const Target<K> * const i_pTarget,
                        int32_t& o_value)
{
    static_assert(std::is_same<T, int32_t>::value, "Attribute type mismatch");

    return FAPI2_RC_SUCCESS;
}


//******************************************************************************
// Get Override int64_t 
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _getOverride(const AttributeId i_id,
                        const Target<K> * const i_pTarget,
                        int64_t& o_value)
{
    static_assert(std::is_same<T, int64_t>::value, "Attribute type mismatch");
    
    return FAPI2_RC_SUCCESS;
}


//******************************************************************************
// Get int8_t array
//******************************************************************************
template<TargetType K>
ReturnCode _getAttributeArraySignedShort(const AttributeId i_id,
                                         const Target<K> * const i_pTarget,
                                         int8_t * o_pValues)
{
    return FAPI2_RC_SUCCESS;
}


//******************************************************************************
// Set int8_t array
//******************************************************************************
template<TargetType K>
ReturnCode _setAttributeArraySignedShort(const AttributeId i_id,
                                         const Target<K> * const i_pTarget,
                                         int8_t * i_pValues)
{
    return FAPI2_RC_SUCCESS;
}


//******************************************************************************
// Get int32_t array
//******************************************************************************
template<TargetType K>
ReturnCode _getAttributeArraySignedWord(const AttributeId i_id,
                                        const Target<K> * const i_pTarget,
                                        int32_t * o_pValues)
{
    return FAPI2_RC_SUCCESS;
}


//******************************************************************************
// Set int32_t array
//******************************************************************************
template<TargetType K>
ReturnCode _setAttributeArraySignedWord(const AttributeId i_id,
                                        const Target<K> * const i_pTarget,
                                        int32_t * i_pValues)
{
    return FAPI2_RC_SUCCESS;
}


//******************************************************************************
// Get int64_t array
//******************************************************************************
template<TargetType K>
ReturnCode _getAttributeArraySignedDoubleWord(const AttributeId i_id,
                                              const Target<K> * const i_pTarget,
                                              int64_t * o_pValues)
{
    return FAPI2_RC_SUCCESS;
}

//******************************************************************************
// Set int64_t array
//******************************************************************************
template<TargetType K>
ReturnCode _setAttributeArraySignedDoubleWord(const AttributeId i_id,
                                              const Target<K> * const i_pTarget,
                                              int64_t * i_pValues)
{
    return FAPI2_RC_SUCCESS;
}

//******************************************************************************
// Get Override int8_t array
//******************************************************************************
template<TargetType K>
ReturnCode _getAttributeOverrideArraySignedShort(const AttributeId i_id,
                                                 const Target<K> * const i_pTarget,
                                                 int8_t * o_pValues)
{
    return FAPI2_RC_SUCCESS;
}


//******************************************************************************
// Get Override int32_t array
//******************************************************************************
template<TargetType K>
ReturnCode _getAttributeOverrideArraySignedWord(const AttributeId i_id,
                                                const Target<K> * const i_pTarget,
                                                int32_t * o_pValues)
{
    return FAPI2_RC_SUCCESS;
}


//******************************************************************************
// Get Override int64_t array
//******************************************************************************
template<TargetType K>
ReturnCode _getAttributeOverrideArraySignedDoubleWord(const AttributeId i_id,
                                                      const Target<K> * const i_pTarget,
                                                      int64_t * o_pValues)
{
    return FAPI2_RC_SUCCESS;
}


//******************************************************************************
// Set int8_t 
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _set(const AttributeId i_id,
                const Target<K> * const i_pTarget,
                int8_t& i_value)
{
    static_assert(std::is_same<T, int8_t>::value, "Attribute type mismatch"); // May need to remove

    if(K & TARGET_TYPE_PROC_CHIP)
    {
        __set<K, fapi2attr::CoreAttributes_t, T, A>( *i_pTarget, G_proc_chip_attributes, i_value );  
    }

    if(K & TARGET_TYPE_PERV)
    {
        __set<K, fapi2attr::PervAttributes_t, T, A>( *i_pTarget, G_perv_attributes, i_value );  
    }

    if(K & TARGET_TYPE_CORE)
    {
        __set<K, fapi2attr::CoreAttributes_t, T, A>( *i_pTarget, G_core_attributes, i_value );  
    }

    if(K & TARGET_TYPE_EQ)
    {
        __set<K, fapi2attr::EQAttributes_t, T, A>( *i_pTarget, G_eq_attributes, i_value );  
    }
    
    if(K & TARGET_TYPE_EX)
    {                                           
        __set<K, fapi2attr::EXAttributes_t, T, A>( *i_pTarget, G_ex_attributes, i_value );                         
    }
    
    return FAPI2_RC_SUCCESS;
}


//******************************************************************************
// Set int32_t 
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _set(const AttributeId i_id,
                const Target<K> * const i_pTarget,
                int32_t& i_value)
{
    static_assert(std::is_same<T, int32_t>::value, "Attribute type mismatch"); // May need to remove

    if(K & TARGET_TYPE_PROC_CHIP)
    {
        __set<K, fapi2attr::CoreAttributes_t, T, A>( *i_pTarget, G_proc_chip_attributes, i_value );  
    }

    if(K & TARGET_TYPE_PERV)
    {
        __set<K, fapi2attr::PervAttributes_t, T, A>( *i_pTarget, G_perv_attributes, i_value );  
    }

    if(K & TARGET_TYPE_CORE)
    {
        __set<K, fapi2attr::CoreAttributes_t, T, A>( *i_pTarget, G_core_attributes, i_value );  
    }

    if(K & TARGET_TYPE_EQ)
    {
        __set<K, fapi2attr::EQAttributes_t, T, A>( *i_pTarget, G_eq_attributes, i_value );  
    }
    
    if(K & TARGET_TYPE_EX)
    {                                           
        __set<K, fapi2attr::EXAttributes_t, T, A>( *i_pTarget, G_ex_attributes, i_value );                         
    }
    
    return FAPI2_RC_SUCCESS;
}


//******************************************************************************
// Set int64_t 
//******************************************************************************
template<typename T, TargetType K, AttributeId A>
ReturnCode _set(const AttributeId i_id,
                const Target<K> * const i_pTarget,
                const int64_t & i_value)
{
    static_assert(std::is_same<T, int64_t>::value, "Attribute type mismatch"); // May need to remove

    if(K & TARGET_TYPE_PROC_CHIP)
    {
        __set<K, fapi2attr::CoreAttributes_t, T, A>( *i_pTarget, G_proc_chip_attributes, i_value );  
    }

    if(K & TARGET_TYPE_PERV)
    {
        __set<K, fapi2attr::PervAttributes_t, T, A>( *i_pTarget, G_perv_attributes, i_value );  
    }

    if(K & TARGET_TYPE_CORE)
    {
        __set<K, fapi2attr::CoreAttributes_t, T, A>( *i_pTarget, G_core_attributes, i_value );  
    }

    if(K & TARGET_TYPE_EQ)
    {
        __set<K, fapi2attr::EQAttributes_t, T, A>( *i_pTarget, G_eq_attributes, i_value );  
    }
    
    if(K & TARGET_TYPE_EX)
    {                                           
        __set<K, fapi2attr::EXAttributes_t, T, A>( *i_pTarget, G_ex_attributes, i_value );                         
    }
    
    return FAPI2_RC_SUCCESS;
}


} // namespace fapi2
#endif // FAPIPLATATTRIBUTESERVICE_H_
