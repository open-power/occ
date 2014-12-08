/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occBootLoader/imageHdrScript.c $                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* COPYRIGHT International Business Machines Corp. 2011,2014              */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
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

//*************************************************************************
// Includes
//*************************************************************************
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <common_types.h>
#include <stddef.h>
#include <string.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define CHECKSUM_FIELD_OFFSET   offsetof(imageHdr_t, checksum)
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
#define ID_STR_LEN              IMAGE_ID_STR_SZ
#define ADDRESS_MASK            0x03FFFFFC
#define BRANCH_MASK             0x48000002
#define DUMP_HDR_STR            "dumpHdr"
#define COMBINE_IMAGE_STR       "combineImage"
#define FILE_TO_WRITE_ODE       "/obj/ppc/occc/405/image.bin"
#define FILE_TO_WRITE_GNU       "image.bin"
#define DISPLAY_SIZE            "displaySize"
#define READELF_CMD             "readelf -S "
#define PIPE_CMD                " > elfdata "
#define ELF_FILE                "elfdata"
#define ELF_FILE_REMOVE_CMD     "rm elfdata"

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

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
                    sscanf(l_str,"%s %s %s %x %x %x ",l_str1,l_sec,
                           l_str1,&l_addr,&l_offset,&l_size);
                    printf("%-25.25s : 0x%08x : %d\t(HEX: %x ) \n",l_sec,
                           l_addr,(int)l_size,l_size);
                    l_totalSz += l_size;
                }
            }
        } // end while loop
        printf("===========================================================\n");
        printf("%-25.25s :            : %d\t(HEX: %x )  \n","Total",
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
int combineImage(char * i_file1)
{
    FILE * l_file1 = NULL;
    FILE * l_file2 = NULL;
    FILE * l_file = NULL;
    int l_rc = SUCCESS_RC;
    unsigned long int l_size = 0;
    bool l_odeBuild = TRUE;

    do
    {
        char * l_sbPath = getenv("SANDBOXBASE");
        if( l_sbPath != NULL)
        {
            l_size = strlen(l_sbPath);
            l_size += strlen(FILE_TO_WRITE_ODE);
        }
        else
        {
            l_sbPath = getenv("OCCROOT");
            if(l_sbPath != NULL)
            {
                l_size = strlen(l_sbPath);
                l_size += strlen(FILE_TO_WRITE_GNU);
                l_odeBuild = FALSE;
            }
            else
            {
                printf("Failed to get either SANDBOXBASE or OCCROOT environment variables\n");
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
        printf("l_fileToWrite: %s\t\tl_sbPath: %s\n", l_fileToWrite, l_sbPath);
        // Open the file1
        l_file1 = fopen(i_file1, "r");

        if( NULL == l_file1)
        {
            printf("Failed to open file %s for reading\n",i_file1);
            l_rc = FAILURE_RC;
            break;
        }

        // Open the destination file
        l_file = fopen(l_fileToWrite, "a");

        if( NULL == l_file)
        {
            printf("Failed to open file %s for reading\n",l_fileToWrite);
            l_rc = FAILURE_RC;
            break;
        }

        // get size of file1
        l_rc = fseek(l_file1, 0, SEEK_END);

        if( l_rc != 0)
        {
            printf("Failed to seek end of the file: %s,rc: %d\n",
                   i_file1,l_rc);
            l_rc = FAILURE_RC;
            break;
        }

        unsigned long int l_sz1 = ftell(l_file1);

        //Now seek back to the beginning
        l_rc = fseek(l_file1, 0, SEEK_SET);

        if( l_rc != 0)
        {
            printf("Failed to seek start after getting size of the file: %s, "
                   "rc: %d\n",i_file1,l_rc);
            l_rc = FAILURE_RC;
            break;
        }

        // Read full file into buffer
        unsigned int l_data[l_sz1];
        size_t l_readSz = fread(&l_data[0], 1, l_sz1, l_file1);

        if( l_readSz != l_sz1)
        {
            printf("Failed to read file: %s,readSz: 0x%x,l_sz: 0x%x\n",
                   i_file1,l_readSz,l_sz1);
            l_rc = -1;
            break;
        }

        // Write file1 data to destination file
        size_t l_writeSz = fwrite( l_data,1,l_sz1,l_file);

        if( l_writeSz != l_sz1)
        {
            printf("Error writing data. Written Sz :0x%x,Expected Sz:0x%x\n",
                   l_writeSz,l_sz1);
            l_rc = FAILURE_RC;
        }

    }while(0);

    // Close file1 if it was open
    if( l_file1 != NULL)
    {
        int l_rc2 = fclose( l_file1);

        if( l_rc2 != 0)
        {
            printf("Failed to close file: %s,rc: %d\n", i_file1,l_rc2);
        }
    }

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
            printf("Failed to read file: %s,readSz: 0x%x,l_sz: 0x%x\n",
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
        printf("ep_addr:\t\t 0x%08x\n", htonl( l_hdrPtr[idx++]));
        printf("checksum:\t\t 0x%08x\n", htonl( l_hdrPtr[idx++]));
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
// End Function Specification
unsigned long int calImageChecksum(FILE * i_filePtr)
{
    unsigned long int l_checksum = 0;

    unsigned long int l_counter = 0;
    int l_val = fgetc(i_filePtr);

    while( l_val != EOF)
    {
        l_checksum += l_val;
        l_counter += 1;
        // Skip checksum field
        if( l_counter == CHECKSUM_FIELD_OFFSET)
        {
            while( l_counter != (CHECKSUM_FIELD_OFFSET + CHECKSUM_FIELD_LEN))
            {
                l_counter++;
                l_val = fgetc(i_filePtr);
            }
        }
        l_val = fgetc(i_filePtr);
    }

    fprintf(stdout,"Checksum: 0x%08X\t\tSize: 0x%08X\n", l_checksum, l_counter);
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
        printf("Failed to seek offset: [0x%x] while writing to file,rc: %d\n",
               i_dataOffset,l_rc);
        l_rc = FAILURE_RC;
    }
    else
    {
        size_t l_writeSz = fwrite( i_dataPtr,1,i_dataSz,i_filePtr);

        if( l_writeSz != i_dataSz)
        {
            printf("Error writing data. Written Sz :0x%x,Expected Sz:0x%x\n",
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
    FILE * l_filePtr = NULL;
    int l_rc = SUCCESS_RC;

    do
    {
        if( argc <= 2)
        {
            printHelp();
            l_rc = FAILURE_RC;
            break;
        }

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

        if( (argc > 2) && (strcmp(argv[2],COMBINE_IMAGE_STR)== 0))
        {
            printf("Combining image from file: %s\n",argv[1]);
            l_rc = combineImage(argv[1]);

            if( l_rc != 0)
            {
                printf("Problem combining image: rc: %d\n",l_rc);
                l_rc = FAILURE_RC;
            }
            break;
        }

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

        // Open the file
        l_filePtr = fopen(argv[1], "r+");

        if( NULL == l_filePtr)
        {
            printf("Failed to open file %s for reading and writing\n",argv[1]);
            l_rc = FAILURE_RC;
            break;
        }
        // file is open now
        // get the file size
        l_rc = fseek(l_filePtr, 0, SEEK_END);

        if( l_rc != 0)
        {
            printf("Failed to seek end of the file: %s, rc: %d\n",argv[1],l_rc);
            l_rc = FAILURE_RC;
            break;
        }

        unsigned long int l_sz = ftell(l_filePtr);

        if( l_sz % 128 != 0)
        {
            printf("Image size:[%d] in file: [%s] is not 128-byte aligned\n",
                   l_sz,argv[1]);
            l_rc = FAILURE_RC;
            break;
        }

        //Now seek back to the beginning
        l_rc = fseek(l_filePtr, 0, SEEK_SET);

        if( l_rc != 0)
        {
            printf("Failed to seek start after getting size of the file: %s, "
                   "rc: %d\n",argv[1],l_rc);
            l_rc = FAILURE_RC;
            break;
        }

        l_sz = htonl(l_sz);

        // Write image size to the image header
        l_rc = write(l_filePtr, &l_sz, IMAGE_SZ_FIELD_LEN,IMAGE_SZ_FIELD_OFFSET);

        if( l_rc != 0)
        {
            printf("Failed to write image size in the file: %s, "
                   "rc: %d,l_sz: 0x%x,IMAGE_SZ_FIELD_LEN: %d, "
                   "IMAGE_SZ_FIELD_OFFSET: %d\n",argv[1],l_rc,
                   IMAGE_SZ_FIELD_LEN,IMAGE_SZ_FIELD_OFFSET);
            l_rc = FAILURE_RC;
            break;

        }

        // Write image version
        unsigned long int l_version = 0;
        sprintf((char*)&l_version, "%s",argv[2]);
        l_rc = write(l_filePtr, &l_version, VERSION_LEN, VERSION_OFFSET);

        if( l_rc != 0)
        {
            printf("Failed to write version in the file: %s, "
                   "rc: %d,l_sz: 0x%x,: VERSION_LEN: %d, "
                   "VERSION_OFFSET: %d\n",VERSION_LEN,l_rc,
                   VERSION_LEN,VERSION_OFFSET);
            l_rc = FAILURE_RC;
            break;

        }

        // If user has passed in image id string, write it to the image
        if( argc > 3)
        {
            l_rc = write(l_filePtr, argv[3], ID_STR_LEN, ID_STR_OFFSET);

            if( l_rc != 0)
            {
                printf("Failed to write id_str in the file: %s, "
                       "rc: %d,l_sz: 0x%x,: ID_STR_LEN: %d, "
                       "ID_STR_OFFSET: %d\n",argv[1],l_rc,
                       ID_STR_LEN,ID_STR_OFFSET);
                l_rc = FAILURE_RC;
                break;

            }
        }

        //Now seek to the ep_address field
        l_rc = fseek(l_filePtr, ADDRESS_OFFSET, SEEK_SET);

        if( l_rc != 0)
        {
            printf("Failed to seek ep_address offset: 0x%x of the file: %s, "
                   "rc: %d\n",ADDRESS_OFFSET,argv[1],l_rc);
            l_rc = FAILURE_RC;
            break;
        }
        unsigned long int l_addr = 0;
        // read ep_addr field
        size_t l_readSz = fread(&l_addr, 1, ADDRESS_LEN, l_filePtr);

        if( l_readSz != ADDRESS_LEN)
        {
            printf("Failed to read address for ep_branch calculation.file: %s, "
                   "readSz: 0x%x,ADDRESS_LEN: 0x%x\n",argv[1],l_readSz,
                   ADDRESS_LEN);
            l_rc = FAILURE_RC;
            break;
        }

        // calculate branch instruction to that address and write to
        // ep_branch_inst field in the image header
        l_addr = ntohl(l_addr);
        l_addr &= ADDRESS_MASK;
        l_addr |= BRANCH_MASK;

        l_addr = htonl(l_addr);

        l_rc = write(l_filePtr, &l_addr, EP_BRANCH_INST_LEN,
                     EP_BRANCH_INST_OFFSET);

        if( l_rc != 0)
        {
            printf("Failed to write ep_branch_inst in the file: %s, "
                   "rc: %d,l_sz: 0x%x,: EP_BRANCH_INST_LEN: %d, "
                   "EP_BRANCH_INST_OFFSET: %d\n",argv[1],l_rc,
                   EP_BRANCH_INST_LEN,EP_BRANCH_INST_OFFSET);
            l_rc = FAILURE_RC;
            break;

        }

        //Now seek back to the beginning for calculating checksum
        l_rc = fseek(l_filePtr, 0, SEEK_SET);

        if( l_rc != 0)
        {
            printf("Failed to seek start before checksum calculation.file: %s,"
                   "rc: %d\n",argv[1],l_rc);
            l_rc = FAILURE_RC;
            break;
        }

        // calculate checksum
        unsigned long int l_checksum = calImageChecksum(l_filePtr);

        l_checksum = htonl(l_checksum);

        // Write checksum
        l_rc = write(l_filePtr, &l_checksum,CHECKSUM_FIELD_LEN,
                     CHECKSUM_FIELD_OFFSET);

        if( l_rc != 0)
        {
            printf("Failed to write image size in the file: %s, "
                   "rc: %d,l_sz: 0x%x,IMAGE_SZ_FIELD_LEN: %d, "
                   "IMAGE_SZ_FIELD_OFFSET: %d\n",argv[1],l_rc,
                   CHECKSUM_FIELD_LEN,CHECKSUM_FIELD_OFFSET);
            l_rc = FAILURE_RC;
            break;
        }

    }while(0);

    // Close file if open
    if( l_filePtr != NULL)
     {
         int l_rc2 = fclose( l_filePtr);

         if( l_rc2 != 0)
         {
             printf("Failed to close file: %s,rc: %d\n", argv[1],l_rc2);
         }
     }


    return l_rc;
}
