/******************************************************************************
// @file parser.c
// @brief OCC ERRL TEST
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section parser.c PARSER.C
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @rc004             rickylie  02/15/2012  Created
 *   @ai009  865968     ailutsar  01/07/2012  Initial version
 *                                            OCC Error log and trace parser web tool enhancement
 *   @jh001  881996     joshych   05/07/2013  Support SRAM error log format
 *   @jh006  897972     joshych   09/17/2013  Problems with OCC elog web parser
 *   @jh008  899096     joshych   09/23/2013  OCC Event Log Parser UserDetail Failure
 *
 *  @endverbatim
 *
 *///*************************************************************************/
 
//*************************************************************************
// Includes
//*************************************************************************
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <memory>
#include <time.h>
#include <errl.h>
#include <sys/types.h>
#include <netinet/in.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define HEX(value,digit_width)  std::uppercase << std::setfill('0') << \
                                std::setw(digit_width) << std::hex << (uint32_t)value
#define DEC(value)              std::dec << (uint32_t)value
#define tracr_buffer_size   1024

// This definition would be changing this to a dynamic variable to be passed 
// into this function
#define SSX_TIMEBASE_FREQUENCY_HZ   600000000   

//*************************************************************************
// Function Prototypes
//*************************************************************************
std::string hex_dump(uint8_t* i_binary_buffer, uint32_t i_buffer_size);

//*************************************************************************
// Global variables
//*************************************************************************
std::string g_severityTbl[3] = { "INFORMATIONAL", "PREDICTIVE", "UNRECOVERABLE"}; // @jh006a
std::string g_calloutTypeTbl[2] = { "HUID", "CompID"}; // @jh006a
std::string g_UsrDtlTypeTbl[3] = { "TRACE", "CallHome", "BINARY"}; // @jh008a

//*************************************************************************
// Structures/Classes
//*************************************************************************
class ErrlUserDetailsEntryContent
{
    public:
        uint8_t*    iv_Content;
        uint32_t    iv_length;
        
        ErrlUserDetailsEntryContent() {
            iv_length = 0;
            iv_Content = NULL;
        }
        
        ErrlUserDetailsEntryContent(uint8_t* i_binary_buffer, uint32_t i_buffer_size) {
            iv_length = i_buffer_size;
            iv_Content = new uint8_t[iv_length];
            memcpy(iv_Content, i_binary_buffer, iv_length);
        }
        
        ErrlUserDetailsEntryContent & operator=(const ErrlUserDetailsEntryContent & i_ErrlUserDetailsEntryContent){
            iv_length = i_ErrlUserDetailsEntryContent.iv_length;
            iv_Content = new uint8_t[iv_length];
            memcpy(iv_Content, i_ErrlUserDetailsEntryContent.iv_Content, iv_length);
        }
        
        ErrlUserDetailsEntryContent(const ErrlUserDetailsEntryContent& i_ErrlUserDetailsEntryContent) {
            iv_length = i_ErrlUserDetailsEntryContent.iv_length;
            iv_Content = new uint8_t[iv_length];
            memcpy(iv_Content, i_ErrlUserDetailsEntryContent.iv_Content, iv_length);
        }
        
        ~ErrlUserDetailsEntryContent() {
            if(iv_Content)
            {
                delete [] iv_Content;
            }
        }
        
        std::string hexdump() {
            return hex_dump(iv_Content, iv_length);
        }
};

class OCC_ERRL
{
    public:
        OCC_ERRL();
        OCC_ERRL(uint8_t*, uint32_t);
        
        std::string dump_header();
        std::string dump_userDetails();
        
    private:
        /// Error Log Header
        ErrlEntry_t                                 iv_header;
        
        /// User Details Entry	
        std::vector<ErrlUserDetailsEntry_t>         iv_UserDetailsHeader;

        /// User Details Entry Content        
        std::vector<ErrlUserDetailsEntryContent>    iv_UserDetailsContent;
};

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************
std::string OCC_ERRL::dump_header() // jh001c
{
    std::stringstream l_stream;
    std::string l_string;
    
    l_stream << "\n OCC Error Log Dump";
    l_stream << "\n-------------------------------------------------------------------------------"; 
    l_stream << "\n Error Log Header"; 
    l_stream << "\n-------------------------------------------------------------------------------"; 
    l_stream << "\n Version        : 0x" << HEX(iv_header.iv_version, 2); 
    l_stream << "\n Entry Id       : 0x" << HEX(iv_header.iv_entryId, 2);
    l_stream << "\n Reason Code    : 0x" << HEX(iv_header.iv_reasonCode,2); 
    l_stream << "\n Severity       : " << g_severityTbl[iv_header.iv_severity]; // @jh006c
    l_stream << "\n Actions        : 0x" << HEX(iv_header.iv_actions.word,2);
    l_stream << "\n Word 4         : 0x" << HEX(ntohl(iv_header.iv_userData4), 8);
    
    // loop for print out Callouts
    // @jh006c - start
    uint8_t l_index = 0;
    uint32_t l_callout = 0;
    for (uint8_t i=0 ; i<iv_header.iv_numCallouts ; i++)
    {
        l_callout = iv_header.iv_callouts[i].iv_calloutValue >> 32;
        if (0 == l_callout)
        {
            break;
        }
        l_index++;
    }
    l_stream << "\n-------------------------------------------------------------------------------";
    l_stream << "\n Number Of Callouts : 0x" << HEX(l_index, 2)
             << " (Dec " << DEC(l_index) << ")";
    l_stream << "\n-------------------------------------------------------------------------------";
    for (uint8_t i=0 ; i<l_index ; i++)
    {
        l_callout = iv_header.iv_callouts[i].iv_calloutValue >> 32;
        l_stream << "\n Type           : " << g_calloutTypeTbl[iv_header.iv_callouts[i].iv_type-1];
        l_stream << "\n Callout Value  : 0x" << HEX(ntohl(l_callout), 8);
        l_stream << "\n Priority       : 0x" << HEX(iv_header.iv_callouts[i].iv_priority, 2);
        l_stream << "\n-------------------------------------------------------------------------------";
    }
    // @jh006c - end
    
    // print out User Details Header
    l_stream << "\n UserDetails Header ";
    l_stream << "\n-------------------------------------------------------------------------------"; 
    l_stream << "\n Version        : 0x" << HEX(iv_header.iv_userDetails.iv_version, 2);
    l_stream << "\n Module Id      : 0x" << HEX(ntohs(iv_header.iv_userDetails.iv_modId), 4);
    l_stream << "\n Firmware Level : 0x" << HEX(ntohl(iv_header.iv_userDetails.iv_fwLevel), 8);
    
	// parsing timestamp to format( seconds: microseconds)
    l_stream << "\n Time           : " << DEC((iv_header.iv_userDetails.iv_timeStamp/SSX_TIMEBASE_FREQUENCY_HZ)/CLOCKS_PER_SEC) << " seconds:" 
             << DEC((iv_header.iv_userDetails.iv_timeStamp/SSX_TIMEBASE_FREQUENCY_HZ)%CLOCKS_PER_SEC ) << " microseconds";
    
    l_stream << "\n OCC ID         : 0x" << HEX(iv_header.iv_userDetails.iv_occId, 2);
    l_stream << "\n OCC Role       : " << ((iv_header.iv_userDetails.iv_occRole != 0) ? "OCC_MASTER":"OCC_SLAVE");
    l_stream << "\n OCC State      : 0x" << HEX(iv_header.iv_userDetails.iv_operatingState, 2);

    // the commited bit is actually stored at reserved1 due to reverse bit order
    uint8_t l_commited = iv_header.iv_userDetails.iv_reserved1 >> 6;
    l_stream << "\n Committed      : " << (l_commited == 1 ? "true":"false"); // @jh006c
    l_stream << "\n Word 1         : 0x" << HEX(ntohl(iv_header.iv_userDetails.iv_userData1), 8);
    l_stream << "\n Word 2         : 0x" << HEX(ntohl(iv_header.iv_userDetails.iv_userData2), 8);
    l_stream << "\n Word 3         : 0x" << HEX(ntohl(iv_header.iv_userDetails.iv_userData3), 8);
    
    l_stream << "\n-------------------------------------------------------------------------------"; 
    l_stream << "\n Error log Entry Size  : " << HEX(ntohs(iv_header.iv_userDetails.iv_entrySize), 4)
             << " (Dec " << DEC(ntohs(iv_header.iv_userDetails.iv_entrySize)) << ")";
    l_stream << "\n-------------------------------------------------------------------------------"; 
    l_stream << "\n User Details Size     : " << HEX(ntohs(iv_header.iv_userDetails.iv_userDetailEntrySize), 4)
             << " (Dec " << DEC(ntohs(iv_header.iv_userDetails.iv_userDetailEntrySize)) << ")";
    l_stream << "\n-------------------------------------------------------------------------------"; 
    
    l_string = l_stream.str();
    
    return l_string;
}

std::string OCC_ERRL::dump_userDetails()
{
    std::stringstream l_stream;
    std::string l_string;
    FILE *fd = NULL;
    uint8_t * l_readBufEntry = NULL;
    
    for(uint32_t l_index = 0; l_index < iv_UserDetailsHeader.size(); l_index++)
    {
        // print out header of User Detail Section
        l_stream << "\n Section        : " << l_index+1; // @jh008c
        l_stream << "\n Version        : 0x" << HEX(iv_UserDetailsHeader[l_index].iv_version, 2); 
        l_stream << "\n Type           : " << g_UsrDtlTypeTbl[iv_UserDetailsHeader[l_index].iv_type-1]; // jh008c
        l_stream << "\n Section Size   : 0x" << HEX(ntohs(iv_UserDetailsHeader[l_index].iv_size), 4)
                 << "  (Dec " << DEC(ntohs(iv_UserDetailsHeader[l_index].iv_size)) << ")"; 
        l_stream << "\n Section Data   : ";
        l_stream << iv_UserDetailsContent[l_index].hexdump();
        
        // parsing User Detail
        switch(iv_UserDetailsHeader[l_index].iv_type)
        {
            case ERRL_USR_DTL_TRACE_DATA:   // Traces
                l_stream << "\n-------------------------------------------------------------------------------"; 
                l_stream << "\n Trace Data\n";
                
                do{
                    // open file to store traces data
                    std::ofstream l_fstr_out("/tmp/tmp_occ_trace", std::ios::binary);
                    if(!l_fstr_out.is_open())
                    {
                        //open fstream failed
                        l_stream << "\nFailed to open trace file";
                        break;
                    }
                    
                    // dump trace data to file, then close it
                    l_fstr_out.write((const char*)iv_UserDetailsContent[l_index].iv_Content,
                                     iv_UserDetailsContent[l_index].iv_length);
                    l_fstr_out.close();
                
                    // call fsp-trace to get traces content
                    l_readBufEntry = new uint8_t[tracr_buffer_size];
                    
                    // use popen to execute fsp-trace
                    fd = popen("./fsp-trace -s ./trexStringFile /tmp/tmp_occ_trace|sed \"/Can't open device 2/d\";rm /tmp/tmp_occ_trace", "r"); 
                    if(NULL == fd)
                    {
                        //popen failed
                        l_stream << "\nFailed to execute fsp-trace command";
                        break;
                    }
                    
                    //read all data into buffer
                    while(fgets((char*)l_readBufEntry, tracr_buffer_size, fd))
                    {
                        l_stream << l_readBufEntry;
                    }
                    
                    // close popen fd
                    pclose(fd);
                    
                    
                }while(FALSE);
                break;
            default:
                break;
        }
        l_stream << "\n-------------------------------------------------------------------------------";
    }
    
    l_string = l_stream.str();
    
    if(l_readBufEntry)
    {
        delete [] l_readBufEntry;
    }
    
    return l_string;
}

OCC_ERRL::OCC_ERRL()
{
    memset(&iv_header, 0, sizeof(iv_header));
}

// @jh008c
OCC_ERRL::OCC_ERRL(uint8_t* i_binary_buffer, uint32_t i_buffer_size)
{
    uint32_t    l_sizeRead = 0;
    ErrlUserDetailsEntry_t  l_UserDetailsEntry;
    //copy Error Log Header into ErrlEntry_t
    if (i_buffer_size >= sizeof(ErrlEntry_t))
    {
        memcpy(&iv_header, i_binary_buffer, sizeof(ErrlEntry_t));
        l_sizeRead = sizeof(ErrlEntry_t);
    }

    while(l_sizeRead < i_buffer_size){
        // make sure we have the enough buffer size
        if (l_sizeRead+sizeof(ErrlUserDetailsEntry_t) <= i_buffer_size)
        {
            // copy User Details Entry structure
            memset(&l_UserDetailsEntry, 0x00, sizeof(l_UserDetailsEntry));
            memcpy(&l_UserDetailsEntry, i_binary_buffer + l_sizeRead, sizeof(ErrlUserDetailsEntry_t));
            l_sizeRead += sizeof(ErrlUserDetailsEntry_t);
            if (l_sizeRead+ntohs(l_UserDetailsEntry.iv_size) <= i_buffer_size)
            {
                //get User Details Entry content
                ErrlUserDetailsEntryContent l_UserDetailsEntryContent(i_binary_buffer + l_sizeRead,
                                                                ntohs(l_UserDetailsEntry.iv_size));
                l_sizeRead += ntohs(l_UserDetailsEntry.iv_size);
                // push user detail structure and content into containers
                iv_UserDetailsHeader.push_back(l_UserDetailsEntry);
                iv_UserDetailsContent.push_back(l_UserDetailsEntryContent);
            }
            else
            {
                break;
            } // check l_UserDetailsEntry.iv_size
        }
        else
        {
            break;
        } // check ErrlUserDetailsEntry_t
    }
    
}

std::string hex_dump(uint8_t* i_binary_buffer, uint32_t i_buffer_size)
{
    std::stringstream l_stream;
    std::string l_string;

    for(uint32_t i = 0; i < i_buffer_size; i++)
    {
        l_stream << HEX((uint32_t)i_binary_buffer[i], 2);
        if(3 == i%4)    // 4 bytes as a group
        {
            l_stream << "  ";
        }
        if(15 == i%16)  // insert new line and space
        {
            l_stream << "\n                  ";
        }
    }
    
    l_string = l_stream.str();
    
    return l_string;
}

// Function Specification
//
// Name: main
//
// Description: main function
//
// Flow:              FN=None
// 
// End Function Specification

int main(int argc, char* argv[])
{
    OCC_ERRL *              l_occ_errl = NULL;      // Error Log Object
    std::vector<uint8_t>    l_readBufEntry;         // Buffer for storing source data
    uint32_t                l_len = 0;              // length of source data
    
    if (argc > 2)
    {
        do {
            std::ifstream l_fstr_in;
            std::ofstream l_fstr_out;
            
            //open file for reading source of Error Log
            l_fstr_in.open(argv[1], std::ios::binary);
            
            // open file for writing parsing result
            l_fstr_out.open(argv[2], std::ios::binary);

            if (!l_fstr_in.is_open() || !l_fstr_out.is_open())
            {
                //open fstream failed
                std::cout << "Failed to open file\n";
                break;
            }
            
            // get length of file:
            l_fstr_in.seekg (0, std::ios::end);
            l_len = l_fstr_in.tellg();
            l_fstr_in.seekg (0, std::ios::beg);
            
            l_readBufEntry.resize((l_len));
            l_fstr_in.read ((char*)&l_readBufEntry[0],l_len);
            l_fstr_in.close();
            
            try{
                // create OCC ERRL object
                l_occ_errl = new OCC_ERRL(&l_readBufEntry[0], l_readBufEntry.size());
                
                if(l_occ_errl)
                {
                    // get ERRL header and dump to file
                    std::string l_string = l_occ_errl->dump_header();
                    l_fstr_out << l_string;
                    
                    // get user details and dump to file
                    l_string = l_occ_errl->dump_userDetails();
                    l_fstr_out << l_string;
                }
            }
            catch(...)
            {
                l_fstr_out << "Parsing error!\n";
            }
            
            l_fstr_out.close();
            
        }while(FALSE);
    }
    else
    {
        std::cout << "Usage: OCCFileParser <input_file> <output_file>\n";
    }

    if(l_occ_errl)
    {
        delete l_occ_errl;
    }
    
    return 0;
}
