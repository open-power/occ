/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwpf/plat/include/fapi2Structs.H $                    */
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
#ifndef fapiStructs_h
#define fapiStructs_h
// Copyright **********************************************************
//                                                                      
// File fapiStructs.H                                               
//                                                                      
// IBM Confidential                                                     
// OCO Source Materials                                                 
// 9400 Licensed Internal Code                                          
// (C) COPYRIGHT IBM CORP. 1996                                         
//                                                                      
// The source code for this program is not published or otherwise       
// divested of its trade secrets, irrespective of what has been         
// deposited with the U.S. Copyright Office.                             
//                                                                      
// End Copyright ******************************************************

/**
 * @file fapiStructs.H
 * @brief fapi eCMD Extension Structures

 * Extension Owner : John Farrugia
*/

//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <string>


//--------------------------------------------------------------------
//  Forward References                                                
//--------------------------------------------------------------------

#define ECMD_FAPI_CAPI_VERSION "1.0"          ///< eCMD FAPI Extension version



#ifndef ECMD_PERLAPI

namespace fapi
{

/**
 * @brief Enumeration of fapi file types
 */
typedef enum {
    FAPI_FILE_UNKNOWN,       ///< Default for not initialized
    FAPI_FILE_HWP
} FileType_t;


enum AttributeSource
{
    FAPI_ATTRIBUTE_SOURCE_UNKNOWN = 0x00000000,
    FAPI_ATTRIBUTE_SOURCE_PLAT = 0x00000001,
    FAPI_ATTRIBUTE_SOURCE_HWP = 0x00000002,
};


#define FAPI_ATTRIBUTE_TYPE_STRING      0x80000000
#define FAPI_ATTRIBUTE_TYPE_UINT8       0x40000000
#define FAPI_ATTRIBUTE_TYPE_UINT32      0x20000000
#define FAPI_ATTRIBUTE_TYPE_UINT64      0x10000000
#define FAPI_ATTRIBUTE_TYPE_UINT8ARY    0x04000000
#define FAPI_ATTRIBUTE_TYPE_UINT32ARY   0x02000000
#define FAPI_ATTRIBUTE_TYPE_UINT64ARY   0x01000000

#define FAPI_ATTRIBUTE_MODE_CONST       0x80000000
/**
 @brief Used by the get/set configuration functions to return the data
*/
template<typename T>
class Attribute 
{
public:
    // Constructor
    Attribute();

    // Destructor 
    ~Attribute();
    
    //
    /// @brief Assignment Operator.
    /// @param[in] i_right Reference to Value to assign from.
    /// @return Reference to 'this' Target
    ///
    Attribute<T>& operator=(const T& i_right)
    {
        this->value = i_right->value;
    }

private:
    T   value;
  
};

inline AttributeData::AttributeData() {}

inline AttributeData::~AttributeData() {}

} //namespace
#endif // #ifndef ECMD_PERLAPI
#endif 





