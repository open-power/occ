/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occBootLoader/imageHdrScript.c $                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2020                        */
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

//*************************************************************************/
// Includes
//*************************************************************************/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <common_types.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>

//*************************************************************************/
// Externs
//*************************************************************************/

//*************************************************************************/
// Macros
//*************************************************************************/

//*************************************************************************/
// Defines/Enums
//*************************************************************************/
#define CHECKSUM_FIELD_OFFSET   offsetof(imageHdr_t, checksum)
#define CHECKSUM_GPE0_FIELD_OFFSET   offsetof(imageHdr_t, gpe0_checksum)
#define CHECKSUM_GPE1_FIELD_OFFSET   offsetof(imageHdr_t, gpe1_checksum)
#define CHECKSUM_FIELD_LEN      4
#define IMAGE_SZ_FIELD_OFFSET   offsetof(imageHdr_t, image_size)
#define IMAGE_SZ_FIELD_LEN      4
#define FAILURE_RC              -1
#define SUCCESS_RC              0
#define EP_BRANCH_INST_LEN      4
#define EP_BRANCH_INST_OFFSET   offsetof(imageHdr_t, ep_branch_inst)
#define ADDRESS_OFFSET          offsetof(imageHdr_t, ep_addr)
#define ADDRESS_LEN             4
#define VERSION_OFFSET          offsetof(imageHdr_t, version)
#define VERSION_LEN             4
#define ID_STR_OFFSET           offsetof(imageHdr_t, image_id_str)
#define GPE0_SZ_FIELD_OFFSET    offsetof(imageHdr_t, gpe0_size)
#define GPE1_SZ_FIELD_OFFSET    offsetof(imageHdr_t, gpe1_size)
#define ID_STR_LEN              IMAGE_ID_STR_SZ
#define ADDRESS_MASK            0x03FFFFFC
#define ABS_BRANCH_MASK         0x48000002
#define REL_BRANCH_MASK         0x48000000
#define BRANCH_INSTR_OFFSET     0x40
#define DUMP_HDR_STR            "dumpHdr"
#define COMBINE_IMAGE_STR       "combineImage"
#define FILE_TO_WRITE_ODE       "/obj/ppc/occc/405/image.bin"
#define FILE_TO_WRITE_GNU       "/image.bin"
#define DISPLAY_SIZE            "displaySize"
#define READELF_CMD             "readelf -S "
#define PIPE_CMD                " > elfdata "
#define ELF_FILE                "elfdata"
#define ELF_FILE_REMOVE_CMD     "rm elfdata"

//*************************************************************************/
// Structures
//*************************************************************************/

//*************************************************************************/
// Globals
//*************************************************************************/

//*************************************************************************/
// Function Prototypes
//*************************************************************************/

//*************************************************************************/
// Functions
//*************************************************************************/

// Function Specification
//
// Name: displaySize
//
// Description: Display size of the object file
//
// End Function Specification
int displaySize(char * i_file)
{
    int l_rc = SUCCESS_RC;
    unsigned long int l_size = 0;
    FILE * l_file = NULL;
    int l_delfile = 0;
    do
    {
        // create command for the system call to display size using
        // readelf tool and copy output into different file for parsing later.
        if( i_file != NULL)
        {
            l_size = strlen(i_file);
        }
        l_size += strlen(READELF_CMD);
        l_size += strlen(PIPE_CMD);
        char l_cmd[l_size+1];
        unsigned long int l_cnt = strlen(READELF_CMD);
        strncpy(l_cmd,READELF_CMD,l_cnt);
        strncpy(&l_cmd[l_cnt],i_file,strlen(i_file));
        l_cnt += strlen(i_file);
        strncpy(&l_cmd[l_cnt],PIPE_CMD,strlen(PIPE_CMD));
        l_cnt += strlen(PIPE_CMD);
        l_cmd[l_cnt] = '\0';
        // do system call
        system (l_cmd );
        // set to indicate delete temporary file
        l_delfile = 1;
        // Open the file that was written with the system call for
        // reading
        l_file = fopen(ELF_FILE, "r");

        if( NULL == l_file)
        {
            printf("Failed to open file %s for reading\n",ELF_FILE);
            l_rc = FAILURE_RC;
            break;
        }
        // seek to end to get size of file
        l_rc = fseek(l_file, 0, SEEK_END);

        if( l_rc != 0)
        {
            printf("Failed to seek end of the file: %s,rc: %d\n",
                   i_file,l_rc);
            l_rc = FAILURE_RC;
            break;
        }
        // get size
        l_size = ftell(l_file);

        //Now seek back to the beginning to start parsing
        l_rc = fseek(l_file, 0, SEEK_SET);

        if( l_rc != 0)
        {
            printf("Failed to seek start after getting size of the file: %s, "
                   "rc: %d\n",i_file,l_rc);
            l_rc = FAILURE_RC;
            break;
        }

        // parse data and display size
        char l_data[l_size];
        unsigned long int l_totalSz = 0;
        char * l_str = NULL;
        printf("===========================================================\n");
        printf("Size display for %s:\n", i_file);
        printf("%-25.25s : Address    : Size   \n","Section");
        printf("===========================================================\n");
        while (fgets(l_data,l_size,l_file) != NULL)
        {
            // Ignore lines that does not have BITS in them
            if(strstr(l_data,"BITS") != NULL)
            {
                // If there is word debug on the line then we are done parsing
                if(strstr(l_data,"debug") != NULL)
                {
                    break;
                }
                l_str = NULL;
                // We need to parse 2 different options:
                // 1) [ X] and 2) [XX] Where X is number.
                // Example of data:
                // Num   Section    Type      Addr     offset  Size
                // [ 9] .linear_wr  PROGBITS  ffff6000 046000 001000 00 WA 0 0 1
                // [10] .linear_rd  PROGBITS  ffff7000 047000 001000 00 WA 0 0 1
                // We are interested in Section, Address, offset and size.
                // Everything else we can ignore. So for ease of parsing,
                // ignore everything until first "]".
                l_str = strstr(l_data,"]");
                if( NULL != l_str )
                {
                    char l_str1[l_size];
                    char l_sec[l_size];
                    unsigned long int l_addr = 0;
                    unsigned long int l_offset = 0;
                    unsigned long int l_size = 0;
                    sscanf(l_str,"%s %s %s %lx %lx %lx ",l_str1,l_sec,
                           l_str1,&l_addr,&l_offset,&l_size);
                    printf("%-25.25s : 0x%08lx : %d\t(HEX: %lx ) \n",l_sec,
                           l_addr,(int)l_size,l_size);
                    l_totalSz += l_size;
                }
            }
        } // end while loop
        printf("===========================================================\n");
        printf("%-25.25s :            : %d\t(HEX: %lx )  \n","Total",
               (int)l_totalSz,l_totalSz);
        printf("===========================================================\n");

    }while(0);

    // Close file if it was open
    if( l_file != NULL)
    {
        int l_rc2 = fclose( l_file);

        if( l_rc2 != 0)
        {
            printf("Failed to close destination file: rc: %d\n",l_rc2);
        }
    }

    if( l_delfile == 1)
    {
        // remove temporary file
        system(ELF_FILE_REMOVE_CMD);
    }

    return l_rc;
}


// Function Specification
//
// Name: combineImage
//
// Description: Append input image to image.bin
//
// End Function Specification
int combineImage(FILE * i_file1)
{
    FILE * l_file2 = NULL;
    FILE * l_file = NULL;
    int l_rc = SUCCESS_RC;
    unsigned long int l_size = 0;
    bool l_odeBuild = TRUE;

    do
    {
        char * l_sbPath = getenv("BASE_OBJDIR");
        if( l_sbPath != NULL)
        {
            l_size = strlen(l_sbPath);
            l_size += strlen(FILE_TO_WRITE_GNU);
            l_odeBuild = FALSE;
        }
        else
        {
            l_sbPath = getenv("SANDBOXBASE");
            if(l_sbPath != NULL)
            {
                l_size = strlen(l_sbPath);
                l_size += strlen(FILE_TO_WRITE_ODE);
            }
            else
            {
                printf("Failed to get either SANDBOXBASE or BASE_OBJDIR environment variables\n");
                l_rc = FAILURE_RC;
                break;
            }
        }
        char l_fileToWrite[l_size+1];
        strncpy(l_fileToWrite,l_sbPath,strlen(l_sbPath));

        if ( TRUE == l_odeBuild )
        {
            strncpy(&l_fileToWrite[strlen(l_sbPath)],FILE_TO_WRITE_ODE,strlen(FILE_TO_WRITE_ODE));
        }
        else
        {
            strncpy(&l_fileToWrite[strlen(l_sbPath)],FILE_TO_WRITE_GNU,strlen(FILE_TO_WRITE_GNU));
        }
        l_fileToWrite[l_size] = '\0';
        printf("Writing to file: %s\n", l_fileToWrite);

        // Open the destination file
        l_file = fopen(l_fileToWrite, "a");

        if( NULL == l_file)
        {
            printf("Failed to open file %s for writing\n",l_fileToWrite);
            l_rc = FAILURE_RC;
            break;
        }

        // get size of file1
        l_rc = fseek(i_file1, 0, SEEK_END);

        if( l_rc != 0)
        {
            printf("Failed to seek end of the input file, rc: %d\n",l_rc);
            l_rc = FAILURE_RC;
            break;
        }

        unsigned long int l_sz1 = ftell(i_file1);

        //Now seek back to the beginning
        l_rc = fseek(i_file1, 0, SEEK_SET);

        if( l_rc != 0)
        {
            printf("Failed to seek start after getting size of the file, rc: %d\n",l_rc);
            l_rc = FAILURE_RC;
            break;
        }

        // Read full file into buffer
        unsigned int l_data[l_sz1];
        size_t l_readSz = fread(&l_data[0], 1, l_sz1, i_file1);

        if( l_readSz != l_sz1)
        {
            printf("Failed to read input file, readSz: 0x%zx,l_sz: 0x%lx\n",
                   l_readSz,l_sz1);
            l_rc = -1;
            break;
        }

        // Write file1 data to destination file
        size_t l_writeSz = fwrite( l_data,1,l_sz1,l_file);

        if( l_writeSz != l_sz1)
        {
            printf("Error writing data. Written Sz :0x%zx,Expected Sz:0x%lx\n",
                   l_writeSz,l_sz1);
            l_rc = FAILURE_RC;
        }

    }while(0);

    // Close destination file if it was open
    if( l_file != NULL)
    {
        int l_rc2 = fclose( l_file);

        if( l_rc2 != 0)
        {
            printf("Failed to close destination file: rc: %d\n",l_rc2);
        }
    }

    return l_rc;
}


// Function Specification
//
// Name: dumpHdr
//
// Description: Dump image header
//
// End Function Specification
int dumpHdr(char * i_fileStr)
{
    FILE * l_filePtr = NULL;
    int l_rc = SUCCESS_RC;
    do
    {
        // Open the file
        l_filePtr = fopen(i_fileStr, "rb");

        if( NULL == l_filePtr)
        {
            printf("Failed to open file %s for reading\n",i_fileStr);
            l_rc = FAILURE_RC;
            break;
        }
        // file is open now
        // get the file size
        l_rc = fseek(l_filePtr, 0, SEEK_END);

        if( l_rc != 0)
        {
            printf("Failed to seek end of the file: %s,rc: %d\n",
                   i_fileStr,l_rc);
            l_rc = FAILURE_RC;
            break;
        }

        unsigned long int l_sz = ftell(l_filePtr);

        //Now seek back to the beginning
        l_rc = fseek(l_filePtr, 0, SEEK_SET);

        if( l_rc != 0)
        {
            printf("Failed to seek start after getting size of the file: %s, "
                   "rc: %d\n",i_fileStr,l_rc);
            l_rc = FAILURE_RC;
            break;
        }

        l_sz = sizeof(imageHdr_t);

        // Read header from the file
        unsigned int l_hdrPtr[l_sz];
        size_t l_readSz = fread(&l_hdrPtr[0], 1, l_sz, l_filePtr);

        if( l_readSz != l_sz)
        {
            printf("Failed to read file: %s,readSz: 0x%zx,l_sz: 0x%lx\n",
                   i_fileStr,l_readSz,l_sz);
            l_rc = -1;
            break;
        }

        printf("==========================================================\n");
        printf("SRAM Repair Reserved:\t ");
        unsigned int idx = 0;
            for(idx = 0; idx < SRAM_REPAIR_RESERVE_SZ/4; idx++)
            {
                printf("0x%08X  ",htonl(l_hdrPtr[idx]) );
            }
        printf("\n");
        printf("ep_branch_inst:\t\t 0x%08x\n",htonl( l_hdrPtr[idx++]));
        printf("halt_inst:\t\t 0x%x\n",htonl( l_hdrPtr[idx++]));
        printf("image_size:\t\t 0x%08x \n", htonl( l_hdrPtr[idx++]));
        printf("start_addr:\t\t 0x%08x\n",htonl( l_hdrPtr[idx++]));
        printf("readonly_size:\t\t 0x%08x \n",htonl( l_hdrPtr[idx++]));
        printf("boot_writeable_addr:\t 0x%08x\n",htonl( l_hdrPtr[idx++]));
        printf("boot_writeable_size:\t 0x%08x \n",htonl( l_hdrPtr[idx++]));
        printf("zero_data_addr:\t\t 0x%08x\n", htonl( l_hdrPtr[idx++]));
        printf("zero_data_size:\t\t 0x%08x \n", htonl( l_hdrPtr[idx++]));
        printf("gpe0_size:\t\t 0x%08x \n", htonl( l_hdrPtr[idx++]));
        printf("gpe1_size:\t\t 0x%08x \n", htonl( l_hdrPtr[idx++]));
        printf("ep_addr:\t\t 0x%08x\n", htonl( l_hdrPtr[idx++]));
        printf("checksum:\t\t 0x%08x\n", htonl( l_hdrPtr[idx++]));
        printf("gpe0_checksum:\t\t 0x%08x\n", htonl( l_hdrPtr[idx++]));
        printf("gpe1_checksum:\t\t 0x%08x\n", htonl( l_hdrPtr[idx++]));
        printf("version:\t\t %.*s\n", VERSION_LEN,(char*)&l_hdrPtr[idx++]);
        printf("image_id_str:\t\t %s\n",(char*)(&l_hdrPtr[idx]));
        idx += (IMAGE_ID_STR_SZ/4);
        unsigned int i = 0;
        printf("Reserved:\t\t ");
        for(i = 0; i < RESERVED_SZ/4; i++)
        {
            printf("0x%08X  ",htonl(l_hdrPtr[idx++]) );
        }
        printf("\n");
        printf("==========================================================\n");

    }while(0);

    // Close file if open
    if( l_filePtr != NULL)
    {
        int l_rc2 = fclose( l_filePtr);

        if( l_rc2 != 0)
        {
            printf("Failed to close file: %s,rc: %d\n", i_fileStr,l_rc2);
        }
    }

    return l_rc;
}


// Function Specification
//
// Name: calImageChecksum
//
// Description: calculate image checksum
//
// Linux command to calculate summation checksum:
//   od -t u1 -An -w1 -v ../obj/occ_gpe0/occ_gpe0.bin | awk '{s+=$1; if(s > 4294967295) s = and(4294967295, s) } END {printf "0x%08X\n", s}'
//
// End Function Specification
unsigned long int calImageChecksum(FILE * i_filePtr, bool i_gpeFile)
{
    unsigned long int l_checksum = 0;
    unsigned long int l_counter = 0;
    int l_val = 0;

    while(1)
    {
        // Skip checksum field
        if( (l_counter == CHECKSUM_FIELD_OFFSET) && !i_gpeFile )
        {
            while( l_counter != (CHECKSUM_FIELD_OFFSET + CHECKSUM_FIELD_LEN))
            {
                l_counter++;
                l_val = fgetc(i_filePtr);
            }
        }
        else if( (l_counter == CHECKSUM_GPE0_FIELD_OFFSET) && !i_gpeFile )
        {
            while( l_counter != (CHECKSUM_GPE0_FIELD_OFFSET + CHECKSUM_FIELD_LEN))
            {
                l_counter++;
                l_val = fgetc(i_filePtr);
            }
        }
        else if( (l_counter == CHECKSUM_GPE1_FIELD_OFFSET) && !i_gpeFile )
        {
            while( l_counter != (CHECKSUM_GPE1_FIELD_OFFSET + CHECKSUM_FIELD_LEN))
            {
                l_counter++;
                l_val = fgetc(i_filePtr);
            }
        }
        else
        {
            l_val = fgetc(i_filePtr);
            if (l_val == EOF) break;

            l_checksum += l_val;
            l_counter ++;
        }
    }

    fprintf(stdout,"Checksum: 0x%08lX\t\tSize: 0x%08lX\n", l_checksum, l_counter);
    return l_checksum;
}


// Function Specification
//
// Name: write
//
// Description: Write given data to file at given offset
//
// End Function Specification
int write(FILE * i_filePtr,
          const void * i_dataPtr,
          const unsigned long int i_dataSz,
          const unsigned long int i_dataOffset)
{
    int l_rc = SUCCESS_RC;

    l_rc = fseek(i_filePtr, i_dataOffset, SEEK_SET);

    if( l_rc != 0)
    {
        printf("Failed to seek offset: [0x%lx] while writing to file,rc: %d\n",
               i_dataOffset,l_rc);
        l_rc = FAILURE_RC;
    }
    else
    {
        size_t l_writeSz = fwrite( i_dataPtr,1,i_dataSz,i_filePtr);

        if( l_writeSz != i_dataSz)
        {
            printf("Error writing data. Written Sz :0x%zx,Expected Sz:0x%lx\n",
                   l_writeSz,i_dataSz);
            l_rc = FAILURE_RC;
        }
    }

    return l_rc;
}


// Function Specification
//
// Name: printHelp
//
// Description: script usage
//
// End Function Specification
void printHelp()
{
    printf("Script Usage: imageHdrScript [FILE] [OPTIONS]\n\n");
    printf("This OCC Image Header Script is used for handling different image header\n");
    printf("fields and combining different OCC images into single image.\n");
    if ( NULL != getenv("SANDBOXBASE") )
    {
        printf("The path to target image is $SANDBOXBASE%s\n\n", FILE_TO_WRITE_ODE);
    }
    else
    {
        printf("The path to target image is $OCCROOT%s\n\n", FILE_TO_WRITE_GNU);
    }
    printf("Option for ELF executable file (file type: *.out):\n");
    printf("    displaySize     check section sizes in input file\n");
    printf("Options for binary image file (file type: *.bin):\n");
    printf("    combineImage    append input image to the target image\n");
    printf("    dumpHdr         dump values of each header field in input image\n\n");

    printf("If the option string is not equal to \"combineImage\", \"displaySize\", or\n");
    printf("\"dumpHdr\", the script will use that string as image version, and start to\n");
    printf("check/update image header fields.\n");
    printf("Example:\n");
    printf("    imageHdrScript IMAGE_FILE VERSION_STRING\n");
    printf("    imageHdrScript IMAGE_FILE VERSION_STRING ID_STRING\n");
}


// Function Specification
//
// Name: main
//
// Description: main for the script
//
// End Function Specification
int main(int argc, char* argv[])
{
    // NOTE: In order for this to work properly, the arguments must be in this order:
    // argv[0] = (implicitly) name of this executable
    // argv[1] = bootloader binary file
    // argv[2] = 405 binary file
    // argv[3] = gpe0 binary file
    // argv[4] = gpe1 binary file
    // argv[5] = bootloader version
    // argv[6] = 405 version
    // argv[7] = (optional) bootloader ID
    // argv[8] = (optional) 405 ID

#define ARG_BOOTLOADER_BIN  argv[1]
#define ARG_405_BIN         argv[2]
#define ARG_GPE0_BIN        argv[3]
#define ARG_GPE1_BIN        argv[4]
#define ARG_BOOTLOADER_VERS argv[5]
#define ARG_405_VERS        argv[6]
#define ARG_BOOTLOADER_ID   argv[7]
#define ARG_405_ID          argv[8]

    FILE * l_bootLdrPtr = NULL;
    FILE * l_file405Ptr = NULL;
    FILE * l_fileGPE0Ptr = NULL;
    FILE * l_fileGPE1Ptr = NULL;
    int l_rc = SUCCESS_RC;

    uint32_t l_405_sz  = 0;
    uint32_t l_gpe0_sz = 0;
    uint32_t l_gpe1_sz = 0;
    uint32_t l_bootLdr_sz = 0;

    do
    {
        if( argc <= 2)
        {
            printHelp();
            l_rc = FAILURE_RC;
            break;
        }

        //=============================================
        // Dump the image header for the specified file
        //=============================================
        if( (strcmp(argv[2],DUMP_HDR_STR)== 0))
        {
            printf("Dump Image Header\n");
            l_rc = dumpHdr(argv[1]);

            if( l_rc != 0)
            {
                printf("Problem dumping header for file: %s,rc: %d\n",
                       argv[1],l_rc);
                l_rc = FAILURE_RC;
            }
            break;
        }

        //=======================================================
        // Dump the imagesize of the image for the specified file
        //=======================================================
        if( (argc > 2) && (strcmp(argv[2],DISPLAY_SIZE)== 0))
        {
            l_rc = displaySize(argv[1]);

            if( l_rc != 0)
            {
                printf("Problem displaying size: rc: %d\n",l_rc);
                l_rc = FAILURE_RC;
            }
            break;
        }

        // At this point we know there is at least 1 argument to the program
        // but we need 6: path to bootloader, 405, gpe0, gpe1, and the versions.
        if( argc < 7 )
        {
            printf("Need the path to the bootloader, 405, gpe0, gpe1, and versions of 405 and bootloader\n");
            l_rc = FAILURE_RC;
            break;
        }

        //===============
        // Open the files
        //===============
        l_bootLdrPtr = fopen(ARG_BOOTLOADER_BIN, "r+");
        l_file405Ptr = fopen(ARG_405_BIN, "r+");
        l_fileGPE0Ptr = fopen(ARG_GPE0_BIN, "r+");
        l_fileGPE1Ptr = fopen(ARG_GPE1_BIN, "r+");

        // Verify all were successfully opened
        if( NULL == l_bootLdrPtr )
        {
            printf("Failed to open file %s for reading and writing\n", ARG_BOOTLOADER_BIN);
            l_rc = FAILURE_RC;
            break;
        }
        else if( NULL == l_file405Ptr)
        {
            printf("Failed to open file %s for reading and writing\n", ARG_405_BIN);
            l_rc = FAILURE_RC;
            break;
        }
        else if( NULL == l_fileGPE0Ptr)
        {
            printf("Failed to open file %s for reading and writing\n", ARG_GPE0_BIN);
            l_rc = FAILURE_RC;
            break;
        }
        else if( NULL == l_fileGPE1Ptr)
        {
            printf("Failed to open file %s for reading and writing\n", ARG_GPE1_BIN);
            l_rc = FAILURE_RC;
            break;
        }

        //===============================
        // Files are open, get file sizes
        //===============================

        // Bootloader
        l_rc = fseek(l_bootLdrPtr, 0, SEEK_END);
        if( l_rc != 0)
        {
            printf("Failed to seek end of the file: %s, rc: %d\n",ARG_BOOTLOADER_BIN,l_rc);
            l_rc = FAILURE_RC;
            break;
        }
        l_bootLdr_sz = ftell(l_bootLdrPtr);

        // 405
        l_rc = fseek(l_file405Ptr, 0, SEEK_END);
        if( l_rc != 0)
        {
            printf("Failed to seek end of the file: %s, rc: %d\n",ARG_405_BIN,l_rc);
            l_rc = FAILURE_RC;
            break;
        }
        l_405_sz = ftell(l_file405Ptr);

        // GPE 0
        l_rc = fseek(l_fileGPE0Ptr, 0, SEEK_END);
        if( l_rc != 0)
        {
            printf("Failed to seek end of the file: %s, rc: %d\n",ARG_GPE0_BIN,l_rc);
            l_rc = FAILURE_RC;
            break;
        }
        l_gpe0_sz = ftell(l_fileGPE0Ptr);

        // GPE 1
        l_rc = fseek(l_fileGPE1Ptr, 0, SEEK_END);
        if( l_rc != 0)
        {
            printf("Failed to seek end of the file: %s, rc: %d\n",ARG_GPE1_BIN,l_rc);
            l_rc = FAILURE_RC;
            break;
        }
        l_gpe1_sz = ftell(l_fileGPE1Ptr);

        //================================================
        // Ensure that all file sizes are 128-byte aligned
        //================================================
        if( (l_405_sz % 128 != 0) || (l_gpe0_sz % 128 != 0) ||
            (l_gpe1_sz % 128 != 0) || (l_bootLdr_sz % 128 != 0) )
        {
            printf("An Image size is not 128-byte aligned: BootLdr [%d] 405[%d] GPE0[%d] GPE1[%d]\n",
                   l_bootLdr_sz, l_405_sz, l_gpe0_sz, l_gpe1_sz);
            l_rc = FAILURE_RC;
            break;
        }

        //=================================================
        // Now seek back to the beginning of the bootloader
        // and 405 since we are writing image sizes to them
        //=================================================

        // 405
        l_rc = fseek(l_file405Ptr, 0, SEEK_SET);
        if( l_rc != 0)
        {
            printf("Failed to seek start after getting size of the file: %s, "
                   "rc: %d\n",ARG_405_BIN,l_rc);
            l_rc = FAILURE_RC;
            break;
        }

        // Bootloader
        l_rc = fseek(l_bootLdrPtr, 0, SEEK_SET);
        if( l_rc != 0)
        {
            printf("Failed to seek start after getting size of the file: %s, "
                   "rc: %d\n",ARG_BOOTLOADER_BIN,l_rc);
            l_rc = FAILURE_RC;
            break;
        }

        //==========================================================
        // Convert from host (typically little endian) to big endian
        //==========================================================
        l_bootLdr_sz = htonl(l_bootLdr_sz);
        l_405_sz = htonl(l_405_sz);
        l_gpe0_sz = htonl(l_gpe0_sz);
        l_gpe1_sz = htonl(l_gpe1_sz);

        //=======================================
        // Write image sizes to the image headers
        //=======================================

        // Write bootloader size to bootloader image header
        l_rc = write(l_bootLdrPtr, &l_bootLdr_sz, IMAGE_SZ_FIELD_LEN,IMAGE_SZ_FIELD_OFFSET);
        if( l_rc != 0)
        {
            printf("Failed to write bootloader image size in the file: %s, "
                   "rc: %d, IMAGE_SZ_FIELD_LEN: %d, "
                   "IMAGE_SZ_FIELD_OFFSET: %zd\n",ARG_BOOTLOADER_BIN,l_rc,
                   IMAGE_SZ_FIELD_LEN,IMAGE_SZ_FIELD_OFFSET);
            l_rc = FAILURE_RC;
            break;

        }

        // Write 405 size to 405 image header
        l_rc = write(l_file405Ptr, &l_405_sz, IMAGE_SZ_FIELD_LEN,IMAGE_SZ_FIELD_OFFSET);
        if( l_rc != 0)
        {
            printf("Failed to write 405 image size in the file: %s, "
                   "rc: %d, IMAGE_SZ_FIELD_LEN: %d, "
                   "IMAGE_SZ_FIELD_OFFSET: %zd\n",ARG_405_BIN,l_rc,
                   IMAGE_SZ_FIELD_LEN,IMAGE_SZ_FIELD_OFFSET);
            l_rc = FAILURE_RC;
            break;

        }

        // Write GPE0 size to 405 image header
        l_rc = write(l_file405Ptr, &l_gpe0_sz, IMAGE_SZ_FIELD_LEN,GPE0_SZ_FIELD_OFFSET);
        if( l_rc != 0)
        {
            printf("Failed to write GPE0 image size in the file: %s, "
                   "rc: %d, IMAGE_SZ_FIELD_LEN: %d, "
                   "GPE0_SZ_FIELD_OFFSET: %zd\n",ARG_405_BIN,l_rc,
                   IMAGE_SZ_FIELD_LEN,GPE0_SZ_FIELD_OFFSET);
            l_rc = FAILURE_RC;
            break;
        }

        // Write GPE1 size to 405 image header
        l_rc = write(l_file405Ptr, &l_gpe1_sz, IMAGE_SZ_FIELD_LEN,GPE1_SZ_FIELD_OFFSET);
        if( l_rc != 0)
        {
            printf("Failed to write GPE1 image size in the file: %s, "
                   "rc: %d, IMAGE_SZ_FIELD_LEN: %d, "
                   "GPE1_SZ_FIELD_OFFSET: %zd\n",ARG_405_BIN,l_rc,
                   IMAGE_SZ_FIELD_LEN,GPE1_SZ_FIELD_OFFSET);
            l_rc = FAILURE_RC;
            break;
        }

        //=====================
        // Write image versions
        //=====================
        char l_version[VERSION_LEN];

        // Bootloader
        strncpy(l_version, argv[5], VERSION_LEN);
        l_rc = write(l_bootLdrPtr, &l_version, VERSION_LEN, VERSION_OFFSET);
        if( l_rc != 0)
        {
            printf("Failed to write version in the file: %s, "
                   "rc: %d, VERSION_LEN: %d, VERSION_OFFSET: %zd\n",
                   ARG_BOOTLOADER_BIN, l_rc, VERSION_LEN, VERSION_OFFSET);
            l_rc = FAILURE_RC;
            break;
        }

        // 405
        strncpy(l_version, argv[6], VERSION_LEN);
        l_rc = write(l_file405Ptr, &l_version, VERSION_LEN, VERSION_OFFSET);
        if( l_rc != 0)
        {
            printf("Failed to write version in the file: %s, "
                   "rc: %d, VERSION_LEN: %d, VERSION_OFFSET: %zd\n",
                   ARG_405_BIN, l_rc, VERSION_LEN, VERSION_OFFSET);
            l_rc = FAILURE_RC;
            break;
        }

        //===================================
        // If user has passed in image id
        // string(s), write it to the image(s)
        //====================================

        // Bootloader
        if( argc > 7 )
        {
            l_rc = write(l_bootLdrPtr, ARG_BOOTLOADER_ID, ID_STR_LEN, ID_STR_OFFSET);

            if( l_rc != 0)
            {
                printf("Failed to write id_str in the file: %s, "
                       "rc: %d ID_STR_LEN: %d, ID_STR_OFFSET: %zd\n",
                       ARG_BOOTLOADER_BIN, l_rc, ID_STR_LEN,ID_STR_OFFSET);
                l_rc = FAILURE_RC;
                break;
            }
        }

        // 405
        if( argc > 8 )
        {
            l_rc = write(l_file405Ptr, ARG_405_ID, ID_STR_LEN, ID_STR_OFFSET);

            if( l_rc != 0)
            {
                printf("Failed to write id_str in the file: %s, "
                       "rc: %d ID_STR_LEN: %d, ID_STR_OFFSET: %zd\n",
                       ARG_405_BIN, l_rc, ID_STR_LEN,ID_STR_OFFSET);
                l_rc = FAILURE_RC;
                break;
            }
        }

        //====================================================
        // Seek to bootloader/405's entry point address fields
        //====================================================

        // Bootloader
        l_rc = fseek(l_bootLdrPtr, ADDRESS_OFFSET, SEEK_SET);
        if( l_rc != 0)
        {
            printf("Failed to seek ep_address offset: 0x%zx of the file: %s, "
                   "rc: %d\n",ADDRESS_OFFSET,ARG_BOOTLOADER_BIN,l_rc);
            l_rc = FAILURE_RC;
            break;
        }

        // 405
        l_rc = fseek(l_file405Ptr, ADDRESS_OFFSET, SEEK_SET);
        if( l_rc != 0)
        {
            printf("Failed to seek ep_address offset: 0x%zx of the file: %s, "
                   "rc: %d\n",ADDRESS_OFFSET,ARG_405_BIN,l_rc);
            l_rc = FAILURE_RC;
            break;
        }

        uint32_t l_405_addr = 0, l_btldr_addr = 0;

        // Read ep_addr fields

        // 405
        size_t l_readSz = fread(&l_405_addr, 1, ADDRESS_LEN, l_file405Ptr);
        if( l_readSz != ADDRESS_LEN)
        {
            printf("Failed to read address for ep_branch calculation. File: %s, "
                   "readSz: 0x%zx, ADDRESS_LEN: 0x%x\n",ARG_405_BIN,l_readSz,
                   ADDRESS_LEN);
            l_rc = FAILURE_RC;
            break;
        }

        // Bootloader
        l_readSz = fread(&l_btldr_addr, 1, ADDRESS_LEN, l_bootLdrPtr);
        if( l_readSz != ADDRESS_LEN)
        {
            printf("Failed to read address for ep_branch calculation. File: %s, "
                   "readSz: 0x%zx, ADDRESS_LEN: 0x%x\n",ARG_BOOTLOADER_BIN,l_readSz,
                   ADDRESS_LEN);
            l_rc = FAILURE_RC;
            break;
        }

        //==========================================================
        // Calculate branch instruction to that address and write to
        // ep_branch_inst field in the image header
        //==========================================================

        // 405
        l_405_addr = ntohl(l_405_addr);
        l_405_addr &= ADDRESS_MASK;
        l_405_addr |= ABS_BRANCH_MASK;
        l_405_addr = htonl(l_405_addr);
        l_rc = write(l_file405Ptr, &l_405_addr, EP_BRANCH_INST_LEN,
                     EP_BRANCH_INST_OFFSET);

        if( l_rc != 0)
        {
            printf("Failed to write ep_branch_inst in the file: %s, "
                   "rc: %d, EP_BRANCH_INST_LEN: %d, "
                   "EP_BRANCH_INST_OFFSET: %zd\n",ARG_405_BIN,l_rc,
                   EP_BRANCH_INST_LEN,EP_BRANCH_INST_OFFSET);
            l_rc = FAILURE_RC;
            break;
        }

        // Bootloader
        l_btldr_addr = ntohl(l_btldr_addr);
        l_btldr_addr -= BRANCH_INSTR_OFFSET;
        l_btldr_addr &= ADDRESS_MASK;
        l_btldr_addr |= REL_BRANCH_MASK;
        l_btldr_addr = htonl(l_btldr_addr);
        l_rc = write(l_bootLdrPtr, &l_btldr_addr, EP_BRANCH_INST_LEN,
                     EP_BRANCH_INST_OFFSET);

        if( l_rc != 0)
        {
            printf("Failed to write ep_branch_inst in the file: %s, "
                   "rc: %d, EP_BRANCH_INST_LEN: %d, "
                   "EP_BRANCH_INST_OFFSET: %zd\n",ARG_BOOTLOADER_BIN,l_rc,
                   EP_BRANCH_INST_LEN,EP_BRANCH_INST_OFFSET);
            l_rc = FAILURE_RC;
            break;
        }

        //=======================================================
        //Now seek back to the beginning for calculating checksum
        //=======================================================

        // 405
        l_rc = fseek(l_file405Ptr, 0, SEEK_SET);
        if( l_rc != 0)
        {
            printf("Failed to seek start before checksum calculation file: %s,"
                   "rc: %d\n",ARG_405_BIN,l_rc);
            l_rc = FAILURE_RC;
            break;
        }

        // Bootloader
        l_rc = fseek(l_bootLdrPtr, 0, SEEK_SET);
        if( l_rc != 0)
        {
            printf("Failed to seek start before checksum calculation file: %s,"
                   "rc: %d\n",ARG_BOOTLOADER_BIN,l_rc);
            l_rc = FAILURE_RC;
            break;
        }

        // GPE0
        l_rc = fseek(l_fileGPE0Ptr, 0, SEEK_SET);
        if( l_rc != 0)
        {
            printf("Failed to seek ep_address offset: 0x%zx of the file: %s, "
                   "rc: %d\n",ADDRESS_OFFSET,ARG_GPE0_BIN,l_rc);
            l_rc = FAILURE_RC;
            break;
        }

        // GPE1
        l_rc = fseek(l_fileGPE1Ptr, 0, SEEK_SET);
        if( l_rc != 0)
        {
            printf("Failed to seek ep_address offset: 0x%zx of the file: %s, "
                   "rc: %d\n",ADDRESS_OFFSET,ARG_GPE1_BIN,l_rc);
            l_rc = FAILURE_RC;
            break;
        }

        //====================
        // Calculate checksums
        //====================

        // 405
        uint32_t l_checksum = calImageChecksum(l_file405Ptr, false);
        l_checksum = htonl(l_checksum);
        l_rc = write(l_file405Ptr, &l_checksum,CHECKSUM_FIELD_LEN,
                     CHECKSUM_FIELD_OFFSET);
        if( l_rc != 0)
        {
            printf("Failed to write image checksum in the file: %s, "
                   "rc: %d,IMAGE_SZ_FIELD_LEN: %d, "
                   "CHECKSUM_FIELD_OFFSET: %zd\n",ARG_405_BIN,l_rc,
                   CHECKSUM_FIELD_LEN,CHECKSUM_FIELD_OFFSET);
            l_rc = FAILURE_RC;
            break;
        }

        // Bootloader
        l_checksum = calImageChecksum(l_bootLdrPtr, false);
        l_checksum = htonl(l_checksum);
        l_rc = write(l_bootLdrPtr, &l_checksum,CHECKSUM_FIELD_LEN,
                     CHECKSUM_FIELD_OFFSET);
        if( l_rc != 0)
        {
            printf("Failed to write image checksum in the file: %s, "
                   "rc: %d,IMAGE_SZ_FIELD_LEN: %d, "
                   "CHECKSUM_FIELD_OFFSET: %zd\n",ARG_BOOTLOADER_BIN,l_rc,
                   CHECKSUM_FIELD_LEN,CHECKSUM_FIELD_OFFSET);
            l_rc = FAILURE_RC;
            break;
        }

        // GPE0
        l_checksum = calImageChecksum(l_fileGPE0Ptr, true);
        l_checksum = htonl(l_checksum);
        l_rc = write(l_file405Ptr, &l_checksum,CHECKSUM_FIELD_LEN,
                     CHECKSUM_GPE0_FIELD_OFFSET);
        if( l_rc != 0)
        {
            printf("Failed to write image checksum in the file: %s, "
                   "rc: %d,IMAGE_SZ_FIELD_LEN: %d, "
                   "CHECKSUM_GPE0_FIELD_OFFSET: %zd\n",ARG_GPE0_BIN,l_rc,
                   CHECKSUM_FIELD_LEN,CHECKSUM_GPE0_FIELD_OFFSET);
            l_rc = FAILURE_RC;
            break;
        }

        // GPE1
        l_checksum = calImageChecksum(l_fileGPE1Ptr, true);
        l_checksum = htonl(l_checksum);
        l_rc = write(l_file405Ptr, &l_checksum,CHECKSUM_FIELD_LEN,
                     CHECKSUM_GPE1_FIELD_OFFSET);
        if( l_rc != 0)
        {
            printf("Failed to write image checksum in the file: %s, "
                   "rc: %d,IMAGE_SZ_FIELD_LEN: %d, "
                   "CHECKSUM_GPE1_FIELD_OFFSET: %zd\n",ARG_GPE1_BIN,l_rc,
                   CHECKSUM_FIELD_LEN,CHECKSUM_GPE1_FIELD_OFFSET);
            l_rc = FAILURE_RC;
            break;
        }

        // Combine the images into one binary
        l_rc = combineImage(l_bootLdrPtr);
        if ( l_rc )
        {
            printf("Failed to combine file %s, rc %d\n", ARG_BOOTLOADER_BIN, l_rc);
            l_rc = FAILURE_RC;
            break;
        }
        l_rc = combineImage(l_file405Ptr);
        if ( l_rc )
        {
            printf("Failed to combine file %s, rc %d\n", ARG_405_BIN, l_rc);
            l_rc = FAILURE_RC;
            break;
        }
        l_rc = combineImage(l_fileGPE0Ptr);
        if ( l_rc )
        {
            printf("Failed to combine file %s, rc %d\n", ARG_GPE0_BIN, l_rc);
            l_rc = FAILURE_RC;
            break;
        }
        l_rc = combineImage(l_fileGPE1Ptr);
        if ( l_rc )
        {
            printf("Failed to combine file %s, rc %d", ARG_GPE1_BIN, l_rc);
            l_rc = FAILURE_RC;
            break;
        }
    }while(0);

    // Close files if open
    if( l_bootLdrPtr != NULL )
    {
        int l_rc2 = fclose(l_bootLdrPtr);

        if( l_rc2 != 0)
        {
            printf("Failed to close file: %s,rc: %d\n", ARG_BOOTLOADER_BIN,l_rc2);
        }
    }

    if( l_file405Ptr != NULL )
    {
        int l_rc2 = fclose( l_file405Ptr);

        if( l_rc2 != 0)
        {
            printf("Failed to close file: %s,rc: %d\n", ARG_405_BIN,l_rc2);
        }
    }

    if( l_fileGPE0Ptr != NULL )
    {
        int l_rc2 = fclose( l_fileGPE0Ptr);

        if( l_rc2 != 0)
        {
            printf("Failed to close file: %s,rc: %d\n", ARG_GPE0_BIN,l_rc2);
        }
    }

    if( l_fileGPE1Ptr != NULL )
    {
        int l_rc2 = fclose( l_fileGPE1Ptr);

        if( l_rc2 != 0)
        {
            printf("Failed to close file: %s,rc: %d\n", ARG_GPE1_BIN,l_rc2);
        }
    }


    return l_rc;
}
