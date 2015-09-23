/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/tools/image/sbe_xip_tool.c $                          */
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
// $Id: sbe_xip_tool.c,v 1.13 2014/06/27 20:50:16 maploetz Exp $

/// \file sbe_xip_tool.c
/// \brief SBE-XIP image search/edit tool
///
/// Note: This file was originally stored under .../procedures/ipl/sbe.  It
/// was moved here at version 1.19.

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



#define __PPE__
#include "fapi2.H"
#include "proc_sbe_fixed.H"
#include "sbe_xip_image.h"

#include "sbe_link.H"
#include "p9_image_help_base.H"
#include "p9_ring_identification.H"
#include "p9_scan_compression.H"

// Usage: sbe_xip_tool <image> [-<flag> ...] normalize
//        sbe_xip_tool <image> [-<flag> ...] get <item>
//        sbe_xip_tool <image> [-<flag> ...] getv <item> <index>
//        sbe_xip_tool <image> [-<flag> ...] set <item> <value> [ <item1> <value1> ... ]
//        sbe_xip_tool <image> [-<flag> ...] setv <item> <index> <value> [ <item1> <index1> <value1> ... ]
//        sbe_xip_tool <image> [-<flag> ...] report [<regex>]
//        sbe_xip_tool <image> [-<flag> ...] append <section> <file>
//        sbe_xip_tool <image> [-<flag> ...] extract <section> <file>
//        sbe_xip_tool <image> [-<flag> ...] delete [ <section0> ... <sectionN> ]
//        sbe_xip_tool <image> [-<flag> ...] dis <section or .rings_summary>\n"//
// 
// This simple application uses the SBE-XIP image APIs to normalize, search
// update and edit SBE-XIP images. This program encapsulates several commands
// in a common command framework which requires an image to operate on, a
// command name, and command arguments that vary by command.  Commands that
// modify the image always rewrite the image in-place in the filesystem;
// however the original image is only modified if the command has completed
// without error.
// 
// The program operates on an SBE-XIP format binary image, which must be
// normalized - unless the tool is being called to normalize the image in the
// first place using the 'normalize' command. The tool also validates the
// image prior to operating on the image.
// 
// The 'get' command retrieves a scalar value from the image and prints its
// representation on stdout (followed by a newline).  Scalar integer values
// and image addresses are printed as hex numbers (0x...).  Strings are printed
// verbatim.
// 
// The 'getv' command retrieves a vector element from the image and prints its
// representation on stdout (followed by a newline).  Integer values
// and image addresses are printed as hex numbers (0x...).  Vectors of strings
// are not supported.
// 
// The 'set' command allows setting integer and string values in the image.
// New integer values can be specified in decimal or hex (0x...).  Strings are
// taken verbatim from the command line.  Note that new string values will be
// silently truncated to the length of the current string if the new value is
// longer than the current string. Updating address values is currently not
// supported.  Any number of item/value pairs can be specified with a single
// 'set' command.
//
// The 'setv' command is provided to set individual vector elements of
// integral arrays.
// 
// The 'report' command prints a report including a dump of the header and
// section table, a listing of the types and values of all items that appear
// in the TOC. The TOC listing includes the
// sequence number of the entry in the TOC, the item name, the item type and
// the item value.
//
// The 'append' command either creates or extends the section named by the
// section argument, by appending the contents of the named file verbatim.
// Currently the section must either be the final (highest address) section of
// the image, or must be empty, in which case the append command creates the
// section as the final section of the image. The 'append' command writes the
// relocatable image address where the input file was loaded to stdout.
//
// The 'extract' command extracts a sections from the binary image.
//
// The 'delete' command deletes 0 or more sections, starting with <section0>.
// Each section to be deleted must either be the final (highest address)
// section of the image at the time it is deleted, or must be empty. The
// 'delete' command writes the size of the final modified image to stdout.
//
// The 'dis' command disassembles the section named by the section argument.
// Note that the section name .rings_summary, which is not an actual XIP
// section name, merely indicates to summarize the .rings section.
//
// The following -i<flag> are supported:
// -ifs
//    causes the validation step to ignore image size check against the file
//    size.
// -iv
//    causes all validation checking to be ignored. (Skips validation step.)

const char* g_usage =
"Usage: sbe_xip_tool <image> [-i<flag> ...] normalize\n"
"       sbe_xip_tool <image> [-i<flag> ...] get <item>\n"
"       sbe_xip_tool <image> [-i<flag> ...] getv <item> <index>\n"
"       sbe_xip_tool <image> [-i<flag> ...] set <item> <value> [ <item1> <value1> ... ]\n"
"       sbe_xip_tool <image> [-i<flag> ...] setv <item> <index> <value> [ <item1> <index1> <value1> ... ]\n"
"       sbe_xip_tool <image> [-i<flag> ...] report [<regex>]\n"
"       sbe_xip_tool <image> [-i<flag> ...] append <section> <file>\n"
"       sbe_xip_tool <image> [-i<flag> ...] extract <section> <file>\n"
"       sbe_xip_tool <image> [-i<flag> ...] delete [ <section0> ... <sectionN> ]\n"
"       sbe_xip_tool <image> [-i<flag> ...] dis <section or .rings_summary>\n"//\n"
"\n"
"This simple application uses the SBE-XIP image APIs to normalize, search\n"
"update and edit SBE-XIP images. This program encapsulates several commands\n"
"in a common command framework which requires an image to operate on, a\n"
"command name, and command arguments that vary by command.  Commands that\n"
"modify the image always rewrite the image in-place in the filesystem;\n"
"however the original image is only modified if the command has completed\n"
"without error.\n"
"\n"
"The program operates on an SBE-XIP format binary image, which must be\n"
"normalized - unless the tool is being called to normalize the image in the\n"
"first place using the 'normalize' command. The tool also validates the\n"
"image prior to operating on the image.\n"
"\n"
"The 'get' command retrieves a scalar value from the image and prints its\n"
"representation on stdout (followed by a newline).  Scalar integer values\n"
"and image addresses are printed as hex numbers (0x...).  Strings are printed\n"
"verbatim.\n"
"\n"
"The 'getv' command retrieves a vector element from the image and prints its\n"
"representation on stdout (followed by a newline).  Integer values\n"
"and image addresses are printed as hex numbers (0x...).  Vectors of strings\n"
"are not supported.\n"
"\n"
"The 'set' command allows setting integer and string values in the image.\n"
"New integer values can be specified in decimal or hex (0x...).  Strings are\n"
"taken verbatim from the command line.  Note that new string values will be\n"
"silently truncated to the length of the current string if the new value is\n"
"longer than the current string. Updating address values is currently not\n"
"supported.  Any number of item/value pairs can be specified with a single\n"
"'set' command.\n"
"\n"
"The 'setv' command is provided to set individual vector elements of\n"
"integral arrays.\n"
"\n"
"The 'report' command prints a report including a dump of the header and\n"
"section table, a listing of the types and values of all items that appear\n"
"in the TOC. The TOC listing includes the\n"
"sequence number of the entry in the TOC, the item name, the item type and\n"
"the item value.\n"
"\n"
"The 'append' command either creates or extends the section named by the\n"
"section argument, by appending the contents of the named file verbatim.\n"
"Currently the section must either be the final (highest address) section of\n"
"the image, or must be empty, in which case the append command creates the\n"
"section as the final section of the image. The 'append' command writes the\n"
"relocatable image address where the input file was loaded to stdout.\n"
"\n"
"The 'extract' command extracs a sections from a binary image.\n"
"\n"
"The 'delete' command deletes 0 or more sections, starting with <section0>.\n"
"Each section to be deleted must either be the final (highest address)\n"
"section of the image at the time it is deleted, or must be empty. The\n"
"'delete' command writes the size of the final modified image to stdout.\n"
"\n"
"The 'dis' command disassembles the section named by the section argument.\n"
"Note that the section name .rings_summary, which is not an actual XIP\n"
"section name, merely indicates to summarize the .rings section.\n"
"\n"
"-i<flag>:\n"
"\t-ifs  Causes the validation step to ignore image size check against the\n"
"\tfile size.\n"
"\t-iv   Causes all validation checking to be ignored.\n"
    ;

SBE_XIP_ERROR_STRINGS(g_errorStrings);
SBE_XIP_TYPE_STRINGS(g_typeStrings);
SBE_XIP_TYPE_ABBREVS(g_typeAbbrevs);
SBE_XIP_SECTION_NAMES(g_sectionNames);
// Disassembler error support.
DIS_ERROR_STRINGS(g_errorStringsDis);

#define ERRBUF_SIZE 60

typedef struct {
    int index;
    int regex;
    regex_t preg;
} ReportControl;

off_t g_imageSize;


// Byte-reverse a 32-bit integer if on an LE machine
inline uint32_t 
myRev32(const uint32_t i_x)
{
    uint32_t rx;

#ifdef _BIG_ENDIAN
    rx = i_x;
#else
    uint8_t *pix = (uint8_t*)(&i_x);
    uint8_t *prx = (uint8_t*)(&rx);

    prx[0] = pix[3];
    prx[1] = pix[2];
    prx[2] = pix[1];
    prx[3] = pix[0];
#endif

    return rx;
}

// Byte-reverse a 64-bit integer if on a little-endian machine
inline uint64_t 
myRev64(const uint64_t i_x)
{
    uint64_t rx;

#ifdef _BIG_ENDIAN
    rx = i_x;
#else
    uint8_t *pix = (uint8_t*)(&i_x);
    uint8_t *prx = (uint8_t*)(&rx);

    prx[0] = pix[7];
    prx[1] = pix[6];
    prx[2] = pix[5];
    prx[3] = pix[4];
    prx[4] = pix[3];
    prx[5] = pix[2];
    prx[6] = pix[1];
    prx[7] = pix[0];
#endif

    return rx;
}

// Normalize an SBE-XIP image.  We normalize a copy of the image first so that
// the original image will be available for debugging in case the
// normalization fails, then validate and copy the normalized image back to
// the mmap()-ed file.

int
normalize(void* io_image, const int i_argc, const char** i_argv, uint32_t i_maskIgnores)
{
    int rc;
    void *copy;

    do {

        // The 'normalize' command takes no arguments

        if (i_argc != 0) {
            fprintf(stderr, g_usage);
            exit(1);
        }

        copy = malloc(g_imageSize);
        if (copy == 0) {
            perror("malloc() failed : ");
            exit(1);
        }

        memcpy(copy, io_image, g_imageSize);
    
        rc = sbe_xip_normalize(copy);
        if (rc) break;

        if ( !(i_maskIgnores & SBE_XIP_IGNORE_ALL) )  {
            rc = sbe_xip_validate2(copy, g_imageSize, i_maskIgnores);
        }
        if (rc) break;

        memcpy(io_image, copy, g_imageSize);

    } while (0);

    return rc;
}


// Print a line of a report, listing the index, symbol, type and current
// value. 

int
tocListing(void* io_image,
           const SbeXipItem* i_item, 
           void* arg)
{
    int rc;
    ReportControl *control;
    uint64_t data;
    char* s;

    control = (ReportControl*)arg;

    do {
        rc = 0;

        if (control->regex) {
            if (regexec(&(control->preg), i_item->iv_id, 0, 0, 0)) {
                break;
            }
        }
                        
        printf("0x%04x | %-42s | %s | ",
               control->index, i_item->iv_id, 
               SBE_XIP_TYPE_STRING(g_typeAbbrevs, i_item->iv_type));

        switch (i_item->iv_type) {
        case SBE_XIP_UINT8:
            rc = sbe_xip_get_scalar(io_image, i_item->iv_id, &data);
            if (rc) break;
            printf("0x%02x", (uint8_t)data);
            break;
        case SBE_XIP_INT8:
            rc = sbe_xip_get_scalar(io_image, i_item->iv_id, &data);
            if (rc) break;
            printf("%d", (int8_t)data);
            break;
        case SBE_XIP_UINT16:
            rc = sbe_xip_get_scalar(io_image, i_item->iv_id, &data);
            if (rc) break;
            printf("0x%08x", (uint16_t)data);
            break;
        case SBE_XIP_INT16:
            rc = sbe_xip_get_scalar(io_image, i_item->iv_id, &data);
            if (rc) break;
            printf("%d", (int16_t)data);
            break;
        case SBE_XIP_UINT32:
            rc = sbe_xip_get_scalar(io_image, i_item->iv_id, &data);
            if (rc) break;
            printf("0x%08x", (uint32_t)data);
            break;
        case SBE_XIP_INT32:
            rc = sbe_xip_get_scalar(io_image, i_item->iv_id, &data);
            if (rc) break;
            printf("%d", (int32_t)data);
            break;
        case SBE_XIP_UINT64:
            rc = sbe_xip_get_scalar(io_image, i_item->iv_id, &data);
            if (rc) break;
            printf("0x%016llx", data);
            break;
        case SBE_XIP_INT64:
            rc = sbe_xip_get_scalar(io_image, i_item->iv_id, &data);
            if (rc) break;
            printf("%d", (int64_t)data);
            break;
        case SBE_XIP_STRING:
            rc = sbe_xip_get_string(io_image, i_item->iv_id, &s);
            if (rc) break;
            printf("%s", s);
            break;
        case SBE_XIP_ADDRESS:
            rc = sbe_xip_get_scalar(io_image, i_item->iv_id, &data);
            if (rc) break;
            printf("0x%04x:0x%08x", 
                   (uint16_t)((data >> 32) & 0xffff),
                   (uint32_t)(data & 0xffffffff));
            break;
        default:
            printf("unknown type\n");
            rc = SBE_XIP_BUG;
            break;
        }
        printf("\n");
    } while (0);
    control->index += 1;
    return rc;
}


// Dump the image header, including the section table

int
dumpHeader(void* i_image)
{
    int i;
    SbeXipHeader header;
    SbeXipSection* section;
    char magicString[9];

    SBE_XIP_SECTION_NAMES(section_name);

    // Dump header information. Since the TOC may not exist we need to get
    // the information from the header explicitly.

    sbe_xip_translate_header(&header, (SbeXipHeader*)i_image);

    memcpy(magicString, (char*)(&(((SbeXipHeader*)i_image)->iv_magic)), 8);
    magicString[8] = 0;

    printf("Magic Number   : 0x%016llx \"%s\"\n", 
           header.iv_magic, magicString);
    printf("Header Version : 0x%02x\n", header.iv_headerVersion);
    printf("Link Address   : 0x%016llx\n", header.iv_linkAddress);
    printf("Entry Offset   : 0x%08x\n", (uint32_t)header.iv_entryOffset);
    printf("Image Size     : 0x%08x (%d)\n",
           header.iv_imageSize, header.iv_imageSize);
    printf("Normalized     : %s\n", header.iv_normalized ? "Yes" : "No");
    printf("TOC Sorted     : %s\n", header.iv_tocSorted ? "Yes" : "No");
    printf("Build Date     : %02d/%02d/%04d\n", 
           (header.iv_buildDate / 100) % 100,
           header.iv_buildDate % 100,
           header.iv_buildDate / 10000);
    printf("Build Time     : %02d:%02d\n", 
           header.iv_buildTime / 100,
           header.iv_buildTime % 100);
    printf("Build User     : %s\n", header.iv_buildUser);
    printf("Build Host     : %s\n", header.iv_buildHost);
    printf("\n");

    printf("Section Table  :   Offset      Size\n");
    printf("\n");

    for (i = 0; i < SBE_XIP_SECTIONS; i++) {
        section = &(header.iv_section[i]);
        printf("%-16s 0x%08x 0x%08x (%d)\n",
               section_name[i], 
               section->iv_offset, section->iv_size, section->iv_size);
    }

    printf("\n");

    return 0;
}


// Print a report

int
report(void* io_image, const int i_argc, const char** i_argv)
{
    int rc;
    ReportControl control;
    char errbuf[ERRBUF_SIZE];

    do {

        // Basic syntax check : [<regexp>]
        
        if (i_argc > 1) {
            fprintf(stderr, g_usage);
            exit(1);
        }

        // Compile a regular expression if supplied

        if (i_argc == 1) {
            rc = regcomp(&(control.preg), i_argv[0], REG_NOSUB);
            if (rc) {
                regerror(rc, &(control.preg), errbuf, ERRBUF_SIZE);
                fprintf(stderr, "Error from regcomp() : %s\n", errbuf);
                exit(1);
            }
            control.regex = 1;
        } else {
            control.regex = 0;

            dumpHeader(io_image);
            printf("TOC Report\n\n");
        }
            
        // Map the TOC with the mapReport() function

        control.index = 0;
        rc = sbe_xip_map_toc(io_image, tocListing, (void*)(&control));
        if (rc) break;

    } while (0);

    return rc;
}


// Set a scalar or vector element values in the image.  The 'i_setv' argument
// indicates set/setv (0/1).

int
set(void* io_image, const int i_argc, const char** i_argv, int i_setv)
{
    int rc, arg, base, clause_args, index_val;
    SbeXipItem item;
    unsigned long long newValue;
    const char *key, *index, *value;
    char *endptr;

    do {

        // Basic syntax check: <item> <value> [ <item1> <value1> ... ]
        // Basic syntax check: <item> <index> <value> [ <item1> <index1> <value1> ... ]

        clause_args = (i_setv ? 3 : 2);

        if ((i_argc % clause_args) != 0) {
            fprintf(stderr, g_usage);
            exit(1);
        }

        for (arg = 0; arg < i_argc; arg += clause_args) {

            key = i_argv[arg];
            if (i_setv) {
                index = i_argv[arg + 1];
                index_val = strtol(index, 0, 0);
                value = i_argv[arg + 2];
            } else {
                index = "";
                index_val = 0;
                value = i_argv[arg + 1];
            }

            // Search for the item to see what type of data it expects, then
            // case split on the type.

            rc = sbe_xip_find(io_image, key, &item);
            if (rc) break;

            if (index_val < 0) {
                fprintf(stderr, 
                        "Illegal negative vector index %s for %s\n", 
                        index, key);
                exit(1);
            } else if ((item.iv_elements != 0) && 
                       (index_val >= item.iv_elements)) {
                fprintf(stderr, 
                        "Index %s out-of-bounds for %s (%d elements)\n", 
                        index, key, item.iv_elements);
                exit(1);
            }
            
            switch (item.iv_type) {
            case SBE_XIP_UINT8:
            case SBE_XIP_UINT16:
            case SBE_XIP_UINT32:
            case SBE_XIP_UINT64:

                // We need to do a bit of preprocessing on the string to
                // determine its format and set the base for strtoull(),
                // otherwise strtoull() will be confused by leading zeros
                // e.g. in time strings generated by `date +%H%M`, and try to
                // process the string as octal.

                if ((strlen(value) >= 2) && (value[0] == '0') &&
                    ((value[1] == 'x') || (value[1] == 'X'))) {
                    base = 16;
                } else {
                    base = 10;
                }

                errno = 0;
                newValue = strtoull(value, &endptr, base);
                if ((errno != 0) || (endptr != (value + strlen(value)))) {
                    fprintf(stderr, 
                            "Error parsing putative integer value : %s\n",
                            value);
                    exit(1);
                }

                switch (item.iv_type) {

                case SBE_XIP_UINT8:
                    if ((uint8_t)newValue != newValue) {
                        fprintf(stderr, 
                                "Value 0x%016llx too large for 8-bit type\n",
                                newValue);
                        exit(1);
                    }
                    break;
                    
                case SBE_XIP_UINT16:
                    if ((uint16_t)newValue != newValue) {
                        fprintf(stderr, 
                                "Value 0x%016llx too large for 16-bit type\n",
                                newValue);
                        exit(1);
                    }
                    break;

                case SBE_XIP_UINT32:
                    if ((uint32_t)newValue != newValue) {
                        fprintf(stderr, 
                                "Value 0x%016llx too large for 32-bit type\n",
                                newValue);
                        exit(1);
                    }
                    break;

                case SBE_XIP_UINT64:
                    break;

                default:
                    break;
                }

                rc = sbe_xip_set_element(io_image, key, index_val, newValue);
                if (rc) rc = SBE_XIP_BUG;
                break;

            case SBE_XIP_STRING:

                if (i_setv) {
                    fprintf(stderr, "Can't use 'setv' for string data %s\n",
                            key);
                    exit(1);
                }
                rc = sbe_xip_set_string(io_image, key, (char*)value);
                if (rc) rc = SBE_XIP_BUG;
                break;
            case SBE_XIP_INT8:
            case SBE_XIP_INT16:
            case SBE_XIP_INT32:
            case SBE_XIP_INT64:
                fprintf(stderr, 
                        "Item %s has int type %s, "
                        "which is not supported for '%s'.\n",
                        i_argv[arg], 
                        SBE_XIP_TYPE_STRING(g_typeStrings, item.iv_type),
                        (i_setv ? "setv" : "set"));
                exit(1);
                break;
            default:
                fprintf(stderr, 
                        "Item %s has type %s, "
                        "which is not supported for '%s'.\n",
                        i_argv[arg], 
                        SBE_XIP_TYPE_STRING(g_typeStrings, item.iv_type),
                        (i_setv ? "setv" : "set"));
                exit(1);
                break;
            }

            if (rc) break;

        }
    } while (0);

    //if good rc, we need to msync the mmaped file to push contents to
    //the actual file. Per man page this is required although some
    //file systems (notably AFS) don't seem to require (GSA does)
    if(!rc)
    {
        uint8_t i = 0;
        do {
                rc = msync(io_image, g_imageSize , MS_SYNC);
                if(rc)
                {
                    i++;
                    fprintf(stderr, 
                             "msync failed with errno %d\n", errno);
                }
        } while(rc && i < 5);

        if(rc)
        {
            exit(3);
        }
    }

    return rc;
}
    

// Get a value from the image, and return on stdout.  The 'i_getv' argument
// indicates get/getv (0/1)

int
get(void* i_image, const int i_argc, const char** i_argv, int i_getv)
{
    int rc, nargs, index_val;
    SbeXipItem item;
    const char *key, *index;
    uint64_t data;
    char* s;

    do {

        // Basic syntax check: <item>
        // Basic syntax check: <item> <index>

        nargs = (i_getv ? 2 : 1);

        if (i_argc != nargs) {
            fprintf(stderr, g_usage);
            exit(1);
        }

        key = i_argv[0];
        if (i_getv) {
            index = i_argv[1];
            index_val = strtol(index, 0, 0);
        } else {
            index = "";
            index_val = 0;
        }

        // Search for the item to determine its type, then case split on the
        // type. 

        rc = sbe_xip_find(i_image, key, &item);
        if (rc) break;

        if (index_val < 0) {
            fprintf(stderr, 
                    "Illegal negative vector index %s for %s\n", 
                    index, key);
            exit(1);
        } else if ((item.iv_elements != 0) && 
                   (index_val >= item.iv_elements)) {
            fprintf(stderr, "Index %s out-of-bounds for %s (%d elements)\n", 
                    index, key, item.iv_elements);
            exit(1);
        }

        switch (item.iv_type) {

        case SBE_XIP_UINT8:
        case SBE_XIP_UINT16:
        case SBE_XIP_UINT32:
        case SBE_XIP_UINT64:
            rc = sbe_xip_get_element(i_image, key, index_val, &data);
            if (rc) {
                rc = SBE_XIP_BUG;
                break;
            }
            switch (item.iv_type) {
            case SBE_XIP_UINT8:
                printf("0x%02x\n", (uint8_t)data);
                break;
            case SBE_XIP_UINT16:
                printf("0x%04x\n", (uint16_t)data);
                break;
            case SBE_XIP_UINT32:
                printf("0x%08x\n", (uint32_t)data);
                break;
            case SBE_XIP_UINT64:
                printf("0x%016llx\n", data);
                break;
            default:
                break;
            }
            break;

        case SBE_XIP_ADDRESS:
            if (i_getv) {
                fprintf(stderr, "Can't use 'getv' for address data : %s\n",
                        key);
                exit(1);
            }
            rc = sbe_xip_get_scalar(i_image, key, &data);
            if (rc) {
                rc = SBE_XIP_BUG;
                break;
            }
            printf("0x%012llx\n", data);
            break;

        case SBE_XIP_STRING:
            if (i_getv) {
                fprintf(stderr, "Can't use 'getv' for string data : %s\n",
                        key);
                exit(1);
            }
            rc = sbe_xip_get_string(i_image, key, &s);
            if (rc) {
                rc = SBE_XIP_BUG;
                break;
            }
            printf("%s\n", s);
            break;
        case SBE_XIP_INT8:
        case SBE_XIP_INT16:
        case SBE_XIP_INT32:
        case SBE_XIP_INT64:
            fprintf(stderr, "%s%d : Bug, int types not implemented %d\n",
                    __FILE__, __LINE__, item.iv_type);
            exit(1);
            break;
        default:
            fprintf(stderr, "%s%d : Bug, unexpected type %d\n",
                    __FILE__, __LINE__, item.iv_type);
            exit(1);
            break;
        }
    } while (0);

    return rc;
}


// strtoul() with application-specific error handling

unsigned long
localStrtoul(const char* s)
{
    unsigned long v;
    char* endptr;

    errno = 0;
    v = strtoul(s, &endptr, 0);
    if ((errno != 0) || (endptr != (s + strlen(s)))) {
        fprintf(stderr, 
                "Error parsing putative integer value : %s\n",
                s);
        exit(1);
    }
    return v;
}


// Append a file to section
int
append(const char* i_imageFile, const int i_imageFd, void* io_image, 
       int i_argc, const char** i_argv)
{
    int fileFd, newImageFd, sectionId, rc;
    struct stat buf;
    const char* section;
    const char* file;
    void* appendImage;
    void* newImage;
    uint32_t size, newSize, sectionOffset;
    uint64_t homerAddress;

    do {

        // Basic syntax check: <section> <file>

        if (i_argc != 2) {
            fprintf(stderr, g_usage);
            exit(1);
        }
        section = i_argv[0];
        file = i_argv[1];
        
        // Translate the section name to a section Id

        for (sectionId = 0; sectionId < SBE_XIP_SECTIONS; sectionId++) {
            if (strcmp(section, g_sectionNames[sectionId]) == 0) {
                break;
            }
        }
        if (sectionId == SBE_XIP_SECTIONS) {
            fprintf(stderr, "Unrecognized section name : '%s;\n",
                    section);
            exit(1);
        }


        // Open and mmap the file to be appended

        fileFd = open(file, O_RDONLY);
        if (fileFd < 0) {
            perror("open() of the file to be appended failed : ");
            exit(1);
        }

        rc = fstat(fileFd, &buf);
        if (rc) {
            perror("fstat() of the file to be appended failed : ");
            exit(1);
        }

        appendImage = mmap(0, buf.st_size, PROT_READ, MAP_SHARED, fileFd, 0);
        if (appendImage == MAP_FAILED) {
            perror("mmap() of the file to be appended failed : ");
            exit(1);
        }


        // malloc() a buffer for the new image, adding space for alignment

        rc = sbe_xip_image_size(io_image, &size);
        if (rc) break;

        newSize = size + buf.st_size + SBE_XIP_MAX_SECTION_ALIGNMENT;

        newImage = malloc(newSize);

        if (newImage == 0) {
            fprintf(stderr, "Can't malloc() a buffer for the new image\n");
            exit(1);
        }


        // Copy the image.  At this point the original image file must be
        // closed.

        memcpy(newImage, io_image, size);

        rc = close(i_imageFd);
        if (rc) {
            perror("close() of the original image file failed : ");
            exit(1);
        }


        // Do the append and print the image address where the data was loaded.
        // We will not fail for unaligned addresses, as we have no knowledge
        // of whether or why the user wants the final image address.

        rc = sbe_xip_append(newImage, sectionId, 
                            appendImage, buf.st_size,
                            newSize, &sectionOffset);
        if (rc) break;

        rc = sbe_xip_section2image(newImage, sectionId, sectionOffset,
                                  &homerAddress);
        if (rc && (rc != SBE_XIP_ALIGNMENT_ERROR)) break;

        printf("0x%016llx\n", homerAddress);


        // Now write the new image back to the filesystem

        newImageFd = open(i_imageFile, O_WRONLY | O_TRUNC);
        if (newImageFd < 0) {
            perror("re-open() of image file failed : ");
            exit(1);
        }

        rc = sbe_xip_image_size(newImage, &size);
        if (rc) break;

        rc = write(newImageFd, newImage, size);
        if ((rc < 0) || ((uint32_t)rc != size)) {
            perror("write() of modified image failed : ");
            exit(1);
        }

        rc = close(newImageFd);
        if (rc) {
            perror("close() of modified image failed : ");
            exit(1);
        }
    } while (0);

    return rc;
}

// Extract section from a file
int
extract(const char* i_imageFile, const int i_imageFd, void* io_image, 
        int i_argc, const char** i_argv)
{
    int fileFd, newImageFd, sectionId, rc;
    void* newImage;
    const char* section;
    const char* file;
    struct stat buf;
    SbeXipHeader header;
    SbeXipSection* xSection;
    uint32_t size;
    uint32_t offset;
    unsigned int i;

    do {

        if (i_argc != 2) {
            fprintf(stderr, g_usage);
            exit(1);
        }
        section = i_argv[0];
        file = i_argv[1];

        printf("%s %s\n", section , file);

        for (sectionId = 0; sectionId < SBE_XIP_SECTIONS; sectionId++) {
            if (strcmp(section, g_sectionNames[sectionId]) == 0) {
                break;
            }
        }
        if (sectionId == SBE_XIP_SECTIONS) {
            fprintf(stderr, "Unrecognized section name : '%s;\n",
                    section);
            exit(1);
        }

        sbe_xip_translate_header(&header, (SbeXipHeader*)io_image);

        for (i = 0; i < SBE_XIP_SECTIONS; i++) {
            xSection = &(header.iv_section[i]);
            
            if (strcmp(section, g_sectionNames[i]) == 0) {
              
                size = xSection->iv_size;
                offset = xSection->iv_offset;

                printf("%-16s 0x%08x 0x%08x (%d)\n",
                       g_sectionNames[i], 
                       xSection->iv_offset, xSection->iv_size, xSection->iv_size);
                
                break;
            }
        }

        newImage = malloc(size);

        if (newImage == 0) {
            fprintf(stderr, "Can't malloc() a buffer for the new image\n");
            exit(1);
        }

        memcpy(newImage, (void*)((uint64_t)io_image + offset), size);

        fileFd = open(file, O_CREAT | O_WRONLY | O_TRUNC, 0755);
        if (fileFd < 0) {
            perror("open() of the fixed section : ");
            exit(1);
        }

        rc = write(fileFd, newImage, size);
        if ((rc < 0) || ((uint32_t)rc != size)) {
            perror("write() of fixed section : ");
            exit(1);
        }

        rc = close(fileFd);
        if (rc) {
            perror("close() of fixed section : ");
            exit(1);
        }

    } while (0);

    return rc;

}
    

// Delete 0 or more sections in order.  

int
deleteSection(const char* i_imageFile, const int i_imageFd, void* io_image, 
              int i_argc, const char** i_argv)
{
    int newImageFd, sectionId, rc, argc;
    const char* section;
    const char** argv;
    void* newImage;
    uint32_t size;

    do {

        // malloc() a buffer for the new image

        rc = sbe_xip_image_size(io_image, &size);
        if (rc) break;

        newImage = malloc(size);

        if (newImage == 0) {
            fprintf(stderr, "Can't malloc() a buffer for the new image\n");
            exit(1);
        }


        // Copy the image. At this point the original image file must be
        // closed.

        memcpy(newImage, io_image, size);

        rc = close(i_imageFd);
        if (rc) {
            perror("close() of the original image file failed : ");
            exit(1);
        }

        // Delete the sections in argument order

        for (argc = i_argc, argv = i_argv; argc != 0; argc--, argv++) {

            // Translate the section name to a section Id

            section = *argv;

            for (sectionId = 0; sectionId < SBE_XIP_SECTIONS; sectionId++) {
                if (strcmp(section, g_sectionNames[sectionId]) == 0) {
                    break;
                }
            }
            if (sectionId == SBE_XIP_SECTIONS) {
                fprintf(stderr, "Unrecognized section name : '%s;\n",
                        section);
                exit(1);
            }

            // Delete the section

            rc = sbe_xip_delete_section(newImage, sectionId);
            if (rc) break;
        }
        if (rc) break;

        // Print the final size of the new image
    
        rc = sbe_xip_image_size(newImage, &size);
        if (rc) break;

        printf("%u\n", size);

        // Now write the new image back to the filesystem

        newImageFd = open(i_imageFile, O_WRONLY | O_TRUNC);
        if (newImageFd < 0) {
            perror("re-open() of image file failed : ");
            exit(1);
        }

        rc = write(newImageFd, newImage, size);
        if ((rc < 0) || ((uint32_t)rc != size)) {
            perror("write() of modified image failed : ");
            exit(1);
        }

        rc = close(newImageFd);
        if (rc) {
            perror("close() of modified image failed : ");
            exit(1);
        }
    } while (0);

    return rc;
}
    

// 'TEST' is an undocumented command provided to test the APIs.  It searches
// and modifies a copy of the image but puts the image back together as it
// was, then verifies that the the original image and the copy are identical.

#define BOMB_IF(test)                                   \
    if (test) {                                         \
        fprintf(stderr, "%s:%d : Error in TEST\n",      \
                __FILE__, __LINE__);                    \
        exit(1);                                        \
    }

#define BOMB_IF_RC                                              \
    if (rc) {                                                   \
        fprintf(stderr, "%s:%d : Error in TEST, rc = %s\n",     \
                __FILE__, __LINE__,                             \
                SBE_XIP_ERROR_STRING(g_errorStrings, rc));      \
        exit(1);                                                \
    }


int
TEST(void* io_image, const int i_argc, const char** i_argv)
{
    int rc;
    uint64_t linkAddress, entryPoint, data, data1, magicKey, entry_offset[2];
    char *key, *revision, *revdup, *longString, *shortString;
    void *originalImage, *deleteAppendImage;
    uint32_t imageSize;
    SbeXipItem item;
    SbeXipHeader header;
    SbeXipSection section;
    //ProcSbeFixed* fixed;
    uint32_t tocSize;

    do {
        rc = sbe_xip_image_size(io_image, &imageSize);
        BOMB_IF_RC;
        originalImage = malloc(imageSize);
        BOMB_IF(originalImage == 0);
        memcpy(originalImage, io_image, imageSize);
        
        rc = sbe_xip_get_scalar(io_image, "toc_sorted", &data);
        BOMB_IF_RC;
        BOMB_IF(data != 1);

        rc = sbe_xip_get_scalar(io_image, "image_size", &data);
        BOMB_IF_RC;
        BOMB_IF(data != (uint64_t)g_imageSize);

        rc = sbe_xip_get_scalar(io_image, "magic", &magicKey);
        BOMB_IF_RC;

        switch (magicKey) {
        case SBE_BASE_MAGIC:
            key = (char*)"proc_sbe_fabricinit_revision";
            rc = sbe_xip_get_string(io_image, key, &revision);
            BOMB_IF_RC;
            BOMB_IF(strncmp(revision, "1.", 2) != 0);
            break;
        case SBE_SEEPROM_MAGIC:
            key = (char*)"";
            // Can't do this test here as the TOC has been stripped
            break;
        case SBE_CENTAUR_MAGIC:
            key = (char*)"cen_sbe_initf_revision";
            rc = sbe_xip_get_string(io_image, key, &revision);
            BOMB_IF_RC;
            BOMB_IF(strncmp(revision, "1.", 2) != 0);
            break;
        default:
            BOMB_IF(1);
            break;
        }

        rc = sbe_xip_get_scalar(io_image, "link_address", &linkAddress);
        BOMB_IF_RC;
        if (magicKey != SBE_SEEPROM_MAGIC) {
            rc = sbe_xip_get_scalar(io_image, "entry_point", &entryPoint);
            BOMB_IF_RC;
        }
        rc = sbe_xip_get_scalar(io_image, "entry_offset", &data);
        BOMB_IF_RC;
        BOMB_IF((magicKey != SBE_SEEPROM_MAGIC) && (entryPoint != (linkAddress + data)));

        rc = 
            sbe_xip_set_scalar(io_image, "toc_sorted", 0) ||
            sbe_xip_set_scalar(io_image, "image_size", 0);
        BOMB_IF_RC;

        data = 0;
        data += (rc = sbe_xip_get_scalar(io_image, "toc_sorted", &data), data);
        BOMB_IF_RC;
        data += (rc = sbe_xip_get_scalar(io_image, "image_size", &data), data);
        BOMB_IF_RC;
        BOMB_IF(data != 0);

        // Write back keys found during read check.

        rc = 
            sbe_xip_set_scalar(io_image, "toc_sorted", 1) ||
            sbe_xip_set_scalar(io_image, "image_size", g_imageSize);
        BOMB_IF_RC;

        // We'll rewrite the revision keyword with a long string and a short
        // string, and verify that rewriting is being done correctly.  In the
        // end we copy the original revision string back in, which is safe
        // because the memory allocation for strings does not change when they
        // are modified.

        revdup = strdup(revision);
        longString = (char*)"A very long string";
        shortString = (char*)"?";

        if (magicKey != SBE_SEEPROM_MAGIC) {
            rc = 
                sbe_xip_set_string(io_image, key, longString) ||
                sbe_xip_get_string(io_image, key, &revision);
            BOMB_IF_RC;
            BOMB_IF((strlen(revision) != strlen(revdup)) ||
                    (strncmp(revision, longString, strlen(revdup)) != 0));

            rc = 
                sbe_xip_set_string(io_image, key, shortString) ||
                sbe_xip_get_string(io_image, key, &revision);
            BOMB_IF_RC;
            BOMB_IF(strcmp(revision, shortString) != 0);

            memcpy(revision, revdup, strlen(revdup) + 1);
        }

        // Use sbe_xip_[read,write]_uint64 to modify the image and restore it
        // to its original form.

        rc = sbe_xip_find(io_image, "entry_offset", &item);
        BOMB_IF_RC;
        rc = sbe_xip_get_scalar(io_image, "entry_offset", &(entry_offset[0]));
        BOMB_IF_RC;

        rc = sbe_xip_read_uint64(io_image, item.iv_address, &(entry_offset[1]));
        BOMB_IF_RC;
        BOMB_IF(entry_offset[0] != entry_offset[1]);

        rc = sbe_xip_write_uint64(io_image, item.iv_address, 
                                  0xdeadbeefdeadc0deull);
        BOMB_IF_RC;
        rc = sbe_xip_read_uint64(io_image, item.iv_address, &(entry_offset[1]));
        BOMB_IF_RC;
        BOMB_IF(entry_offset[1] != 0xdeadbeefdeadc0deull);

        rc = sbe_xip_write_uint64(io_image, item.iv_address, entry_offset[0]);
        BOMB_IF_RC;

        // Try sbe_xip_get_section against the translated header

        sbe_xip_translate_header(&header, (SbeXipHeader*)io_image);
        rc = sbe_xip_get_section(io_image, SBE_XIP_SECTION_TOC, &section);
        BOMB_IF_RC;
        BOMB_IF((section.iv_size != 
                 header.iv_section[SBE_XIP_SECTION_TOC].iv_size));


        // Make sure the .fixed section access compiles and seems to
        // work. Modify an entry via the .fixed and verify it with normal TOC
        // access. 

        if (magicKey == SBE_SEEPROM_MAGIC) {

            BOMB_IF(0 != 0);

            exit(1);

            rc = sbe_xip_get_scalar(io_image, "proc_sbe_ex_dpll_initf_control",
                                    &data);
            BOMB_IF_RC;
            //fixed = 
            //(ProcSbeFixed*)((unsigned long)io_image + SBE_XIP_FIXED_OFFSET);
            //fixed->proc_sbe_ex_dpll_initf_control = 0xdeadbeefdeadc0deull;
            rc = sbe_xip_get_scalar(io_image, "proc_sbe_ex_dpll_initf_control",
                                    &data1);
            BOMB_IF_RC;
#ifdef _BIG_ENDIAN
            BOMB_IF(data1 != 0xdeadbeefdeadc0deull);
#else
            BOMB_IF(data1 != 0xdec0addeefbeaddeull);
#endif
            rc = sbe_xip_set_scalar(io_image, "proc_sbe_ex_dpll_initf_control",
                                    data);
            BOMB_IF_RC;
        }

        // Temporarily "delete" the .toc section and try to get/set via the
        // mini-TOC for .fixed, and make sure that we can't get things that
        // are not in the mini-toc.

        tocSize = 
            ((SbeXipHeader*)io_image)->iv_section[SBE_XIP_SECTION_TOC].iv_size;

        ((SbeXipHeader*)io_image)->iv_section[SBE_XIP_SECTION_TOC].iv_size = 
            0;

        rc = sbe_xip_get_scalar(io_image, "proc_sbe_ex_dpll_initf_control",
                                &data);
        rc = sbe_xip_set_scalar(io_image, "proc_sbe_ex_dpll_initf_control",
                                0xdeadbeef);
        rc = sbe_xip_get_scalar(io_image, "proc_sbe_ex_dpll_initf_control",
                                &data1);
        BOMB_IF(data1 != 0xdeadbeef);
        rc = sbe_xip_set_scalar(io_image, "proc_sbe_ex_dpll_initf_control",
                                data);
        BOMB_IF_RC;

        BOMB_IF(sbe_xip_find(io_image, "proc_sbe_ex_dpll_initf", 0) !=
                SBE_XIP_ITEM_NOT_FOUND);

        ((SbeXipHeader*)io_image)->iv_section[SBE_XIP_SECTION_TOC].iv_size = 
            tocSize;

        if (magicKey != SBE_SEEPROM_MAGIC) {
            BOMB_IF(sbe_xip_find(io_image, "proc_sbe_ex_dpll_initf", 0) != 0);
        }


#ifdef DEBUG_SBE_XIP_IMAGE
        printf("\nYou will see an expected warning below "
               "about SBE_XIP_WOULD_OVERFLOW\n"
               "It means the TEST is working (not failing)\n\n");
#endif

        // Finally compare against the original

        BOMB_IF(memcmp(io_image, originalImage, imageSize));

    } while (0);

    return rc;
}


/// Function:  pairRingNameAndAddr()  to be used w/sbe_xip_map_toc()
///
/// Brief:  Looks for address match for both base and override rings and
///         for multi-chiplet rings. Returns the ring name and other good
///         stuff in the PairingInfo structure upon a match. 
/// 
/// \param[in] i_image  A pointer to an SBE-XIP image in host memory. 
///
/// \param[in] i_item  A pointer to the "next" SbeXipItem in the TOC.
///
/// \param[io] io_pairing   A pointer to the structure, PairingInfo.
///
/// Assumptions:
/// - On input, io_pairing contains 
///   - address = the backPtr of the next ring block,
///   - vectorpos = the next vector position, starting from 0,1,2,..,31 and
///                 which includes possible override pos.
/// - The TOC is then traversed and each TOC entry is put into i_item.
/// - The backPtr in io_pairing is kept constant until TOC has been exhausted.
/// - On output, io_pairing contains
///   - name = TOC name
///   - isvpd = whether it's a VPD or non-VPD ring
///   - overridable = whether it's an overridable ring
///   - override = whether it's a base or an override ring
/// - In general, we don't know if a ring is a base or an override of if it is a 
///   multi-chiplet type.  Thus, we first look for a match to the vector in
///   in position zero, then we try position one, then position two, ..., and up
///   to a max of position 31 (which would be an override for ex chiplet 0x1F).
/// 
static int pairRingNameAndAddr( void *i_image, const SbeXipItem *i_item, void *io_pairing)
{
  int rc=0,rcLoc=-1;
  SbeXipItem tocItem;
	PairingInfo *pairingInfo;
	RingIdList  *ringIdList;
  
  SBE_XIP_ERROR_STRINGS(g_errorStrings);

  rcLoc = sbe_xip_find( i_image, i_item->iv_id, &tocItem);
  if (rcLoc)  {
    fprintf( stderr, "sbe_xip_find() failed : %s\n", SBE_XIP_ERROR_STRING(g_errorStrings, rcLoc));
    rc = DIS_RING_NAME_ADDR_MATCH_FAILURE;
  }
  else  {
		pairingInfo = (PairingInfo*)io_pairing;
		// Do a sanity check.
		if (pairingInfo->vectorpos>31)  {
    	fprintf( stderr, "vectorpos (=%i) must be between [0;31]\n",pairingInfo->vectorpos);
    	rc = DIS_RING_NAME_ADDR_MATCH_FAILURE;
		}
    // Check for match.
    // - vectorpos is passed in such that first we look for base match, then for override
		//   or chiplet range match.
    if (tocItem.iv_address == pairingInfo->address-8*pairingInfo->vectorpos &&
        tocItem.iv_id!=NULL  )  {
      pairingInfo->name = tocItem.iv_id;
			rcLoc = get_vpd_ring_list_entry(pairingInfo->name,0,&ringIdList);
			if (!rcLoc)  {
				// It is a VPD ring...and they never have overrides.
				pairingInfo->isvpd = 1;
				pairingInfo->overridable = 0;
			}
			else  {
				// It is a non-VPD ring...and they all have override capability.
				pairingInfo->isvpd = 0;
				pairingInfo->overridable = 1;
			}
      if (pairingInfo->vectorpos==0)
			  // This can only be a base match.
				pairingInfo->override = 0;
      else  {
				// This is not a base match. Investigating further if override. (Note,
				//   this includes a multi-dim vector with multi-dim override.)
				if (pairingInfo->overridable && 
						2*(pairingInfo->vectorpos/2)!=pairingInfo->vectorpos)
					pairingInfo->override = 1;
				else
					pairingInfo->override = 0;
			}
    	rc = DIS_RING_NAME_ADDR_MATCH_SUCCESS;
		}
  }
  return rc;
}

// this function is just defined out, because there is a future need.
#ifdef BLUBBER
/// Function:  disassembleSection
///
/// Brief:  Disassembles a section and returns a pointer to a buffer that 
///         contains the listing.
/// 
/// \param[in] i_image  A pointer to an SBE-XIP image in host memory. 
///
/// \param[in] i_argc  Additional number of arguments beyond "dis" keyword.
///
/// \param[in] i_argv  Additional arguments beyond "dis" keyword.
///
/// Assumptions: 
/// 
int disassembleSection(void         *i_image, 
                       int          i_argc, 
                       const char   **i_argv)
{
    int rc=0, rcSet=0;
    uint32_t  rcCount=0;
    char      *disList=NULL;
    uint32_t  sizeSection=0, nextLinkOffsetBlock=0;
    uint32_t  sizeBlock=0, sizeData=0, sizeCode=0, sizeData2=0;
    uint32_t  sizeDisLine=0, sizeList=0, sizeListMax=0;
    uint32_t  offsetCode=0;
    uint8_t   typeRingsSection=0;  // 0: RS4  1: Wiggle-Flip
    uint8_t   bSummary=0, bFoundInToc=0;
    uint32_t  sectionId;
    uint64_t  backPtr=0, fwdPtr=0;
    PairingInfo pairingInfo;
    const char *sectionName;
	char      *ringName;
	uint32_t  ringSeqNo=0; // Ring sequence location counter.
	uint8_t   vectorPos,overRidable;
    void *nextBlock, *nextSection;
    SbeXipHeader hostHeader;
    SbeXipSection hostSection;
    ImageInlineContext ctx;
    ImageInlineDisassembly dis;
    char lineDis[LISTING_STRING_SIZE];
    void      *hostRs4Container;
    uint32_t  compressedBits=0, ringLength=0;
    double    compressionPct=0;

    if (i_argc != 1) {
        fprintf(stderr, g_usage);
        exit(1);
    }
    sectionName = i_argv[0];

    // Determine SBE-XIP section ID from the section name, e.g.
    //         .loader_text =>  SBE_XIP_SECTION_LOADER_TEXT
    //         .text     =>  SBE_XIP_SECTION_TEXT
    //         .rings    =>  SBE_XIP_SECTION_RINGS
    if (strcmp(sectionName, ".header")==0)
        sectionId = SBE_XIP_SECTION_HEADER;
    else
        if (strcmp(sectionName, ".fixed")==0)
            sectionId = SBE_XIP_SECTION_FIXED;
        else
            if (strcmp(sectionName, ".fixed_toc")==0)
                sectionId = SBE_XIP_SECTION_FIXED_TOC;
            else
                if (strcmp(sectionName, ".loader_text")==0)
                    sectionId = SBE_XIP_SECTION_LOADER_TEXT;
                else
                    if (strcmp(sectionName, ".loader_data")==0)
                        sectionId = SBE_XIP_SECTION_LOADER_DATA;
                    else
                        if (strcmp(sectionName, ".text")==0)
                            sectionId = SBE_XIP_SECTION_TEXT;
                        else
                            if (strcmp(sectionName, ".data")==0)
                                sectionId = SBE_XIP_SECTION_DATA;
                            else
                                if (strcmp(sectionName, ".toc")==0)
                                    sectionId = SBE_XIP_SECTION_TOC;
                                else
                                    if (strcmp(sectionName, ".strings")==0)
                                        sectionId = SBE_XIP_SECTION_STRINGS;
                                    else
                                        if (strcmp(sectionName, ".base")==0)
                                            sectionId = SBE_XIP_SECTION_BASE;
                                        else
                                            if (strcmp(sectionName, ".baseloader")==0)
                                                sectionId = SBE_XIP_SECTION_BASELOADER;
                                            else
                                                if (strcmp(sectionName, ".rings")==0)
                                                    sectionId = SBE_XIP_SECTION_RINGS;
                                                else
                                                    if (strcmp(sectionName, ".rings_summary")==0)  {
                                                        sectionId = SBE_XIP_SECTION_RINGS;
                                                        bSummary = 1;
                                                    }
                                                    else
                                                        if (strcmp(sectionName, ".overlays")==0)
                                                            sectionId = SBE_XIP_SECTION_OVERLAYS;
                                                        else  {
                                                            fprintf(stderr,"ERROR : %s is an invalid section name.\n",sectionName);
                                                            fprintf(stderr,"Valid <section> names for the 'dis' function are:\n");
                                                            fprintf(stderr,"\t.header\n");
                                                            fprintf(stderr,"\t.fixed\n");
                                                            fprintf(stderr,"\t.fixed_toc\n");
                                                            fprintf(stderr,"\t.loader_text\n");
                                                            fprintf(stderr,"\t.loader_data\n");
                                                            fprintf(stderr,"\t.text\n");
                                                            fprintf(stderr,"\t.data\n");
                                                            fprintf(stderr,"\t.toc\n");
                                                            fprintf(stderr,"\t.strings\n");
                                                            fprintf(stderr,"\t.base\n");
                                                            fprintf(stderr,"\t.baseloader\n");
                                                            fprintf(stderr,"\t.overlays\n");
                                                            fprintf(stderr,"\t.rings\n");
                                                            fprintf(stderr,"\t.rings_summary\n");
                                                            exit(1);
                                                        }

    // Get host header and section pointer.
    //
    sbe_xip_translate_header( &hostHeader, (SbeXipHeader*)i_image);
    rc = sbe_xip_get_section( i_image, sectionId, &hostSection);
    if (rc)  {
        fprintf( stderr, "sbe_xip_get_section() failed : %s\n", SBE_XIP_ERROR_STRING(g_errorStrings, rc));
        return SBE_XIP_DISASSEMBLER_ERROR;
    }
    sizeSection = hostSection.iv_size;
    nextBlock = (void*)(hostSection.iv_offset + (uintptr_t)i_image);
    nextSection = (void*)((uint64_t)nextBlock + (uint64_t)sizeSection);
  
    // Relocatable offset of section at hand.
    nextLinkOffsetBlock = (uint32_t)hostHeader.iv_linkAddress + hostSection.iv_offset;

    // Allocate buffer to hold disassembled listing. (Start out with minimum 10k buffer size.)
    //
    if (sizeSection>10000)
        sizeListMax = sizeSection; // Just to use something as an initial guess.
    else
        sizeListMax = 10000;
    disList = (char*)malloc(sizeListMax);
    if (disList==NULL) {
        fprintf( stderr, "ERROR : malloc() failed.\n");
        fprintf( stderr, "\tMore info: %s\n", DIS_ERROR_STRING(g_errorStringsDis, DIS_MEMORY_ERROR));
        return SBE_XIP_DISASSEMBLER_ERROR;
    }
    *disList = '\0'; // Make sure the buffer is NULL terminated (though probably not needed.)
    sizeList = 0;

    // Create context and point it to image section.
    //
    rc = image_inline_context_create(  &ctx,
                                       nextBlock,
                                       sizeSection,
                                       nextLinkOffsetBlock,
                                       0);
    if (rc)  {
        fprintf( stderr, "ERROR : %s (rc=%i)\n",image_inline_error_strings[rc],rc);
        fprintf( stderr, "\tMore info: %s\n", DIS_ERROR_STRING(g_errorStringsDis, DIS_DISASM_ERROR));
        return SBE_XIP_DISASSEMBLER_ERROR;
    }

    while ((uint64_t)nextBlock<(uint64_t)nextSection) {

        // Disassemble sections based on their types and intents.
        //
        if (sectionId==SBE_XIP_SECTION_RINGS || sectionId==SBE_XIP_SECTION_OVERLAYS)  {
            // Ring section (with a mix of data and code.)
            // ...use BaseRingLayout structure to decode each ring block.
            offsetCode = (uint32_t)myRev64(((BaseRingLayout*)nextBlock)->entryOffset);
            sizeBlock = myRev32(((BaseRingLayout*)nextBlock)->sizeOfThis);
            // ...determine ring type, either RS4 or Wiggle-flip.
            if (offsetCode-(myRev32(((BaseRingLayout*)nextBlock)->sizeOfMeta)+3)/4*4>28)  {
                typeRingsSection = 0;  // RS4 w/32-byte header.
                sizeData2 = sizeBlock - offsetCode - ASM_RS4_LAUNCH_BUF_SIZE;
            }
            else
                typeRingsSection = 1;  // Wiggle-flip w/24-byte header.
            // ...get the backPtr and fwdPtr and put at top of disasm listing.
            backPtr = myRev64(((BaseRingLayout*)nextBlock)->backItemPtr);
            sbe_xip_read_uint64(i_image,
                                backPtr,
                                &fwdPtr);
      
            // Calculate RS4 compression efficiency if RS4 rings.
            if (typeRingsSection==0)  {
                hostRs4Container = (void*)( (uintptr_t)nextBlock + 
                                            offsetCode + ASM_RS4_LAUNCH_BUF_SIZE );
                compressedBits = myRev32(((CompressedScanData*)hostRs4Container)->iv_algorithmReserved) * 4;
                ringLength = myRev32(((CompressedScanData*)hostRs4Container)->iv_length);
                compressionPct = (double)compressedBits / (double)ringLength * 100.0;
			}
      
            //
			//   Map over TOC or do a targeted search of FIXED_TOC to pair backPtr addr 
			//   with ring name and override and/or vector position (i.e. multi-chiplet).
			//
            sbe_xip_get_section( i_image, SBE_XIP_SECTION_TOC, &hostSection);
			if (hostSection.iv_offset)  {
                // TOC exists.
                pairingInfo.address = backPtr;
				// Search for pairing. First exhaust base position (pos=0), then next, then next, ...
				for (pairingInfo.vectorpos=0;pairingInfo.vectorpos<32;pairingInfo.vectorpos++)  {
                    rc = sbe_xip_map_toc( i_image, pairRingNameAndAddr, (void*)(&pairingInfo));
					if (rc)
						break;
				}
                if (rc==DIS_RING_NAME_ADDR_MATCH_FAILURE)  {
                    fprintf( stderr,"ERROR : Error associated with sbe_xip_map_toc().\n");
                    fprintf( stderr, "\tMore info: %s\n", DIS_ERROR_STRING(g_errorStringsDis, DIS_RING_NAME_ADDR_MATCH_FAILURE));
                    return SBE_XIP_DISASSEMBLER_ERROR;
                }
                ringSeqNo++;
				if (rc==DIS_RING_NAME_ADDR_MATCH_SUCCESS)  {
					bFoundInToc = 1;
                    ringName = pairingInfo.name;       // The ring name matched in pairRingNameAndAddr()
					vectorPos = pairingInfo.vectorpos; // The vector position matched in pairRingNameAndAddr()
					overRidable = pairingInfo.overridable; // Whether the ring supports on override ring.
                    if (pairingInfo.override)  {
                        sizeDisLine = snprintf(lineDis,LISTING_STRING_SIZE,
                                               "# ------------------------------\n# %i.\n# ringName = %s (override)\n# vectorPos = %i\n# overRidable = %i\n# backPtr = 0x%08x\n# fwdPtr  = 0x%08x\n# Compressed Bits  = %u\n# Ring Length Bits = %u\n# Compression      = %0.2f%%\n",
                                               ringSeqNo, ringName,vectorPos,overRidable,(uint32_t)backPtr,(uint32_t)fwdPtr,compressedBits,ringLength,compressionPct);
                    }
                    else  {
                        sizeDisLine = snprintf(lineDis,LISTING_STRING_SIZE,
                                               "# ------------------------------\n# %i.\n# ringName = %s (base)\n# vectorPos = %i\n# overRidable = %i\n# backPtr = 0x%08x\n# fwdPtr  = 0x%08x\n# Compressed Bits  = %u\n# Ring Length Bits = %u\n# Compression      = %0.2f%%\n",
                                               ringSeqNo,ringName,vectorPos,overRidable,(uint32_t)backPtr,(uint32_t)fwdPtr,compressedBits,ringLength,compressionPct);
                    }
				}
                else  {
                    sizeDisLine = snprintf(lineDis,LISTING_STRING_SIZE,
                                           "# ------------------------------\n# %i.\n# ringName = Not found (but TOC's available)\n# backPtr = 0x%08x\n# fwdPtr  = 0x%08x\n",
                                           ringSeqNo,(uint32_t)backPtr,(uint32_t)fwdPtr);
				}
            }
			else  {
                // TOC doesn't exist. First try targeted search of MVPD ring names in FIXED_TOC.
                bFoundInToc = 0; // If we find in fixed_toc, then change to 1.
				// 2012-11-13: CMO TBD. Try using pairRingNameAndAddr by enabling a sequential 
			    //             traversing of each of the MVPD lists inside that function. You'll
                //             need to call pairRing manually from right here (or from a
                //             sbe_xip_search_fixed_toc()-like function). Maybe you can add a
				//             4th arg to pairRing that is zero by default, meaning it is to be
				//             used by xip_map_toc(). But if non-zero, it is to be used in a
				//             traversing manner. Or you could add another member to the
                //             PairingInfo struct to indirectly pass this info to the function.
				//             You'd also need to pass two more arguments to get_vpd_ring_list_
                //             entry() to indicate sequence number and the MVPD keyword.
                // rc = pairRingNameAndAddr();
				// if (rc==DIS_RING_NAME_ADDR_MATCH_SUCCESS)  {
                //   bFoundInToc = 1;
                // // Do same as in TOC section above.
                //   break;
                // }
                // // OK, so ring name wasn't in TOC nor in FIXED_TOC. That happens if the ring
                // //   is a non-Mvpd ring and the TOC has been removed, such as in an IPL or
                // //   Seeprom image.
                ringSeqNo++;
                if (typeRingsSection==0)  {
                    // RS4 header, which has override info
                    if (((Rs4RingLayout*)nextBlock)->override==0)  {
                        sizeDisLine = snprintf(lineDis,LISTING_STRING_SIZE,
                                               "# ------------------------------\n# %i.\n# ringName = Not available (base)\n# backPtr = 0x%08x\n# fwdPtr  = 0x%08x\n# Compressed Bits  = %u\n# Ring Length Bits = %u\n# Compression      = %0.2f%%\n",
                                               ringSeqNo,(uint32_t)backPtr,(uint32_t)fwdPtr,compressedBits,ringLength,compressionPct);
                    }
					else  {
                        sizeDisLine = snprintf(lineDis,LISTING_STRING_SIZE,
                                               "# ------------------------------\n# %i.\n# ringName = Not available (override)\n# backPtr = 0x%08x\n# fwdPtr  = 0x%08x\n# Compressed Bits  = %u\n# Ring Length Bits = %u\n# Compression      = %0.2f%%\n",
                                               ringSeqNo,(uint32_t)backPtr,(uint32_t)fwdPtr,compressedBits,ringLength,compressionPct);
					}
                }
				else  {
                    // WF header, which doesn't have override info
                    sizeDisLine = snprintf(lineDis,LISTING_STRING_SIZE,
                                           "# ------------------------------\n# %i.\n# ringName and override = Not available\n# backPtr = 0x%08x\n# fwdPtr  = 0x%08x\n",
                                           ringSeqNo,(uint32_t)backPtr,(uint32_t)fwdPtr);
				}
            }
            sizeList = sizeList + sizeDisLine;
            disList = strcat(disList,lineDis);
		}
        else  if (  sectionId==SBE_XIP_SECTION_LOADER_TEXT || 
                    sectionId==SBE_XIP_SECTION_TEXT)  {
            // Sections that have only code.
            offsetCode = 0;
            sizeBlock = sizeSection;
        }
        else  {
            // Sections that have only data.
            offsetCode = sizeSection;
            sizeBlock = sizeSection;
        }
        sizeData = offsetCode;
        sizeCode = sizeBlock - offsetCode - sizeData2;

        if (sectionId==SBE_XIP_SECTION_RINGS && bSummary)  {  
            //
            // Summarize rings section.

            if (typeRingsSection==0)  {  // RS4 header.
                sizeDisLine = snprintf(lineDis,LISTING_STRING_SIZE,
                                       "# ddLevel = 0x%02x\n# override= %i\n# sysPhase= %i\n# Block size= %i\n",
                                       myRev32(((Rs4RingLayout*)nextBlock)->ddLevel),
                                       ((Rs4RingLayout*)nextBlock)->override,
                                       ((Rs4RingLayout*)nextBlock)->sysPhase,
                                       sizeBlock);
            }
            else  {                      // WF header.
				if (bFoundInToc)  {
                    sizeDisLine = snprintf(lineDis,LISTING_STRING_SIZE,
                                           "# override= %i\n# Block size= %i\n",
                                           pairingInfo.override, sizeBlock);
				}
				else  {
                    sizeDisLine = snprintf(lineDis,LISTING_STRING_SIZE,
                                           "# override= Not available\n# Block size= %i\n",
                                           sizeBlock);
				}
            }
            sizeList = sizeList + sizeDisLine;
            disList = strcat(disList,lineDis);
            // Readjust list buffer size, if needed.
            if (sizeList > sizeListMax-1000)  {
                sizeListMax = 2*sizeListMax;
                disList = (char*)realloc( (void*)(disList), sizeListMax);
            }
    
        }
        else  {  
            //
            // Do disassembly.
    
            // ...data disassembly
            if (sizeData>0)  {
                ctx.options = IMAGE_INLINE_LISTING_MODE | IMAGE_INLINE_DISASSEMBLE_DATA;
                do  {
                    rc = image_inline_disassemble( &ctx, &dis);
                    sizeDisLine = snprintf(lineDis,LISTING_STRING_SIZE,"%s\n",dis.s);
                    sizeList = sizeList + sizeDisLine;
                    disList = strcat(disList,lineDis);
                    if (rc)  {
                        rcSet = rcSet | 0x1;
                        sizeDisLine = snprintf(lineDis,LISTING_STRING_SIZE,
                                               "WARNING: %s (rc=%i) -> Stopping disasm. Check code and sectionID=%i.\n",
                                               image_inline_error_strings[rc],rc,sectionId);
                        sizeList = sizeList + sizeDisLine;
                        disList = strcat(disList,lineDis);
                    }
                    // Readjust list buffer size, if needed.
                    if (sizeList > sizeListMax-1000)  {
                        sizeListMax = 2*sizeListMax;
                        disList = (char*)realloc( (void*)(disList), sizeListMax);
                    }
                } while (rc==0 && ctx.lc<nextLinkOffsetBlock+sizeData);
            }
            if (rcSet)
                rc = 0;
      
            // ...code disassembly
            if (sizeCode>0)  {
                ctx.options = IMAGE_INLINE_LISTING_MODE;
                do  {
                    rc = image_inline_disassemble( &ctx, &dis);
                    ctx.options = IMAGE_INLINE_LISTING_MODE;
                    sizeDisLine = snprintf(lineDis,LISTING_STRING_SIZE,"%s\n",dis.s);
                    sizeList = sizeList + sizeDisLine;
                    disList = strcat(disList,lineDis);
                    if (rc && rcCount<100)  {
                        rcSet = rcSet | 0x2;
                        rcCount++;
                        if (sectionId==SBE_XIP_SECTION_RINGS)  {
                            sizeDisLine = snprintf(lineDis,LISTING_STRING_SIZE,
                                                   "WARNING: %s (rc=%i) -> Trying data disasm mode. Check code, xyzRingLayout structures and image section.\n",
                                                   image_inline_error_strings[rc],rc);
                        }
                        else  {
                            sizeDisLine = snprintf(lineDis,LISTING_STRING_SIZE,
                                                   "WARNING: %s (rc=%i) -> Trying data disasm mode.\n",
                                                   image_inline_error_strings[rc],rc);
                        }
                        sizeList = sizeList + sizeDisLine;
                        disList = strcat(disList,lineDis);
                        ctx.options = IMAGE_INLINE_LISTING_MODE | IMAGE_INLINE_DISASSEMBLE_DATA;
                        rc = 0;
                    }
                    else  {
                        if (rc && rcCount>=1000)  {
                            fprintf(stderr, "Too many disasm warnings. Check output listing.\n");
                            fprintf( stderr, "\tMore info: %s\n", DIS_ERROR_STRING(g_errorStringsDis, DIS_TOO_MANY_DISASM_WARNINGS));
                            return SBE_XIP_DISASSEMBLER_ERROR;
                        }
                    }
                    // Readjust list buffer size, if needed.
                    if (sizeList > sizeListMax-1000)  {
                        sizeListMax = 2*sizeListMax;
                        disList = (char*)realloc( (void*)(disList), sizeListMax);
                    }
                } while (rc==0 && ctx.lc<nextLinkOffsetBlock+sizeData+sizeCode);
            }
            if (rcSet)
                rc = 0;
        
            // ...data2 disassembly (only done for rings section if RS4 type.)
            if (sizeData2>0)  {
                ctx.options = IMAGE_INLINE_LISTING_MODE | IMAGE_INLINE_DISASSEMBLE_DATA;
                do  {
                    rc = image_inline_disassemble( &ctx, &dis);
                    sizeDisLine = snprintf(lineDis,LISTING_STRING_SIZE,"%s\n",dis.s);
                    sizeList = sizeList + sizeDisLine;
                    disList = strcat(disList,lineDis);
                    if (rc)  {
                        rcSet = rcSet | 0x4;
                        sizeDisLine = snprintf(lineDis,LISTING_STRING_SIZE,
                                               "WARNING: %s (rc=%i) -> Stopping disasm. Check code and sectionID=%i.\n",
                                               image_inline_error_strings[rc],rc,sectionId);
                        sizeList = sizeList + sizeDisLine;
                        disList = strcat(disList,lineDis);
                    }
                    // Readjust list buffer size, if needed.
                    if (sizeList > sizeListMax-1000)  {
                        sizeListMax = 2*sizeListMax;
                        disList = (char*)realloc( (void*)(disList), sizeListMax);
                    }
                } while (rc==0 && ctx.lc<nextLinkOffsetBlock+sizeBlock);
            }
            if (rcSet)
                rc = 0;
      
        }  // End of if (bSummary) condition.

        nextBlock = (void*)((uint64_t)nextBlock + (uint64_t)sizeBlock);
        nextLinkOffsetBlock = nextLinkOffsetBlock + sizeBlock;

    }  // End of while(nextBlock...) loop.

    // Adjust final buffer size, add 1 for NULL char and print it.
    if (disList)  {
        disList = (char*)realloc( (void*)(disList), sizeList+1);
        fprintf(stdout,"%s\n",disList);
        free(disList);
    }
  
    if (rcSet)
        fprintf( stderr, "INFO : There were some hickups: %s\n", DIS_ERROR_STRING(g_errorStringsDis, DIS_DISASM_TROUBLES));

    return 0;

}
#endif

// open() and mmap() the file 

void
openAndMap(const char* i_fileName, int i_writable, int* o_fd, void** o_image, const uint32_t i_maskIgnores)
{
    int rc, openMode, mmapProt, mmapShared;
    struct stat buf;

    if (i_writable) {
        openMode = O_RDWR;
        mmapProt = PROT_READ | PROT_WRITE;
        mmapShared = MAP_SHARED;
    } else {
        openMode = O_RDONLY;
        mmapProt = PROT_READ;
        mmapShared = MAP_PRIVATE;
    }

    *o_fd = open(i_fileName, openMode);
    if (*o_fd < 0) {
        perror("open() of the image failed : ");
        exit(1);
    }

    rc = fstat(*o_fd, &buf);
    if (rc) {
        perror("fstat() of the image failed : ");
        exit(1);
    }
    g_imageSize = buf.st_size;

    *o_image = mmap(0, g_imageSize, mmapProt, mmapShared, *o_fd, 0);
    if (*o_image == MAP_FAILED) {
        perror("mmap() of the image failed : ");
        exit(1);
    }

    if ( !(i_maskIgnores & SBE_XIP_IGNORE_ALL) )  {
        rc = sbe_xip_validate2(*o_image, g_imageSize, i_maskIgnores);
        if (rc) {
            fprintf(stderr, "sbe_xip_validate2() failed : %s\n",
                    SBE_XIP_ERROR_STRING(g_errorStrings, rc)); 
            exit(1);
        }
    }

}


static inline void
openAndMapWritable(const char* i_imageFile, int* o_fd, void** o_image, const uint32_t i_maskIgnores)
{
    openAndMap(i_imageFile, 1, o_fd, o_image, i_maskIgnores);
}


static inline void
openAndMapReadOnly(const char* i_imageFile, int* o_fd, void** o_image, const uint32_t i_maskIgnores)
{
    openAndMap(i_imageFile, 0, o_fd, o_image, i_maskIgnores);
}


// Parse and execute a pre-tokenized command

void
command(const char* i_imageFile, const int i_argc, const char** i_argv, const uint32_t i_maskIgnores)
{
    void* image;
    int fd, rc = 0;

    if (strcmp(i_argv[0], "normalize") == 0) {

        openAndMapWritable(i_imageFile, &fd, &image, i_maskIgnores);
        rc = normalize(image, i_argc - 1, &(i_argv[1]), i_maskIgnores);

    } else if (strcmp(i_argv[0], "set") == 0) {

        openAndMapWritable(i_imageFile, &fd, &image, i_maskIgnores);
        rc = set(image, i_argc - 1, &(i_argv[1]), 0);

    } else if (strcmp(i_argv[0], "setv") == 0) {

        openAndMapWritable(i_imageFile, &fd, &image, i_maskIgnores);
        rc = set(image, i_argc - 1, &(i_argv[1]), 1);

    } else if (strcmp(i_argv[0], "get") == 0) {

        openAndMapReadOnly(i_imageFile, &fd, &image, i_maskIgnores);
        rc = get(image, i_argc - 1, &(i_argv[1]), 0);

    } else if (strcmp(i_argv[0], "getv") == 0) {

        openAndMapReadOnly(i_imageFile, &fd, &image, i_maskIgnores);
        rc = get(image, i_argc - 1, &(i_argv[1]), 1);

    } else if (strcmp(i_argv[0], "report") == 0) {

        openAndMapReadOnly(i_imageFile, &fd, &image, i_maskIgnores);
        rc = report(image, i_argc - 1, &(i_argv[1]));

    } else if (strcmp(i_argv[0], "append") == 0) {

        openAndMapWritable(i_imageFile, &fd, &image, i_maskIgnores);
        rc = append(i_imageFile, fd, image, i_argc - 1, &(i_argv[1]));

    } else if (strcmp(i_argv[0], "extract") == 0) {

        openAndMapWritable(i_imageFile, &fd, &image, i_maskIgnores);
        rc = extract(i_imageFile, fd, image, i_argc - 1, &(i_argv[1]));

    } else if (strcmp(i_argv[0], "delete") == 0) {

        openAndMapWritable(i_imageFile, &fd, &image, i_maskIgnores);
        rc = deleteSection(i_imageFile, fd, image, i_argc - 1, 
                           &(i_argv[1]));

    } else if (strcmp(i_argv[0], "dis") == 0) {

        //openAndMapReadOnly(i_imageFile, &fd, &image, i_maskIgnores);
        //rc = disassembleSection(image, i_argc - 1, &(i_argv[1]));
        fprintf(stderr, "not supported\n"); 
        exit(1);


    } else if (strcmp(i_argv[0], "TEST") == 0) {

        openAndMapWritable(i_imageFile, &fd, &image, i_maskIgnores);
        rc = TEST(image, i_argc - 1, &(i_argv[1]));

    } else {
        fprintf(stderr, g_usage);
        exit(1);
    }
    if (rc) {
        fprintf(stderr, "Command failed : %s\n",
                SBE_XIP_ERROR_STRING(g_errorStrings, rc)); 
        exit(1);
    }
}


// Open, map and validate the image, then parse and execute the command.  The
// image is memory-mapped read/write, i.e, it may be modified in-place.
// Commands that modify the size of the image will close and recreate the
// file.

int
main(int argc, const char** argv)
{
    uint8_t   argcMin, idxArgvFlagsStart;
    uint8_t   numFlags=0, idxArgv, bMoreFlags;
    uint32_t  maskIgnores=0;

    argcMin = 3;
    idxArgvFlagsStart = argcMin - 1; // -i flags must start after image file name.

    numFlags = 0;
    bMoreFlags = 1;
    do  {
      idxArgv = idxArgvFlagsStart + numFlags;
      if (idxArgv <= (argc-1))  {
        if (strncmp(argv[idxArgv], "-i", 1) == 0)  {
          numFlags++;
          bMoreFlags = 1;
          if (strncmp(argv[idxArgv], "-ifs", 4) == 0)  {
              maskIgnores = maskIgnores | SBE_XIP_IGNORE_FILE_SIZE;
          }
          else
          if (strncmp(argv[idxArgv], "-iv", 3) == 0)  {
              maskIgnores = maskIgnores | SBE_XIP_IGNORE_ALL;
          }
          else  {
            fprintf(stderr, g_usage);
            fprintf(stderr, "\n");
            fprintf(stderr,"argv[%i]=%s is an unsupported flag.",idxArgv,argv[idxArgv]);
            fprintf(stderr,"See top of above help menu for supported flags.\n");
            exit(1);
          }
        }
        else
          bMoreFlags = 0;
      }
      else  {
        bMoreFlags = 0;
        break;
      }
    }  while (bMoreFlags);

    if ((argc < (argcMin+numFlags)) ||
        (strncmp(argv[1], "-h", 2) == 0) ||
        (strncmp(argv[1], "--h", 3) == 0) )  {
      fprintf(stderr, g_usage);
      exit(1);
    }

    command(argv[1], argc - idxArgv, &(argv[idxArgv]), maskIgnores);
				
    return 0;
}
