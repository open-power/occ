/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/tools/ppetracepp/ppe2fsp_cmd.c $                      */
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
#include <stdio.h>
#include "ppe2fsp.h"
#include "pk_trace.h"

#define MAX_INPUT_SIZE 0x2040       //8k
#define MAX_OUTPUT_SIZE (4 * MAX_INPUT_SIZE) 

char* inbuf[MAX_INPUT_SIZE];
char* outbuf[MAX_OUTPUT_SIZE];
;
int main(int argc, char** argv)
{
    FILE* in;
    FILE* out;
    size_t  input_size;
    size_t  output_size;
    size_t  bytes_written;
    int rc = -1;

    do
    {
        if(argc > 3)
        {
            fprintf(stderr, "Usage: %s [input file] [output file]\n", argv[0]);
        }

        if(argc < 3)
        {
            out = stdout;
        }
        else
        {
            //open the output file for writing
            out = fopen(argv[2], "w");
            if(!out)
            {
                perror("failed to open file for writing");
                break;
            }
        }
    
        if(argc < 2)
        {
            in = stdin;
        }
        else
        {
            //open the input file for reading
            in = fopen(argv[1], "r");
            if(!in)
            {
                perror("failed to open file for reading");
                break;
            }
        }
    
        //read the input stream until we reach EOF or the max size
        input_size = fread(inbuf, 1, MAX_INPUT_SIZE, in);
        if(!feof(in))
        {
            if(ferror(in))
            {
                perror("failed to read input file");
                break;
            }
            else
            {
                fprintf(stderr, "Input stream exceeds max size of %d bytes. Exiting.\n", MAX_INPUT_SIZE);
                break;
            }
        }

        output_size = MAX_OUTPUT_SIZE;

        //Actual size of output buffer will be set upon successful completion
        rc = ppe2fsp(inbuf, input_size, outbuf, &output_size);
        if(rc)
        {
            fprintf(stderr, "Failed converting ppe trace to fsp trace. rc = %d\n", rc);
            if(rc == P2F_INVALID_VERSION)
            {
                fprintf(stderr, "PPE trace buffer must be version %d.\n", PK_TRACE_VERSION);
            }
            break;
        }

        rc = -1;
        //operation was successful.  Write out the fsp trace data
        bytes_written = fwrite(outbuf, 1, output_size, out);
        if(bytes_written != output_size)
        {
            if(ferror(out))
            {
                perror("Failed to write output stream");
                break;
            }
            fprintf(stderr, "Failure: Only able to write %d of %d bytes to output stream\n", bytes_written, output_size);
            break;
        }

        fclose(in);
        fclose(out);
        fclose(stderr);

        rc = 0;
    }while(0);

    return rc;
}

    
    
    
    

