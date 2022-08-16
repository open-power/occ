#!/usr/bin/perl
# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/tools/parser/ebmc/genPelSrcParser.pl $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2013,2022
# [+] International Business Machines Corp.
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.
#
# IBM_PROLOG_END_TAG

# Purpose: Generates files needed to parse SRCs from OCC error logs

use strict;
use Time::localtime;

#------------------------------------------------------------------------------
# Process Arguments
#------------------------------------------------------------------------------
my $DEBUG = 0;
my $base = "../../../..";
my $outputDir = ".";

while( $ARGV = shift )
{
    if( $ARGV =~ m/-b/ )
    {
        $base = shift;
    }
    elsif( $ARGV =~ m/-o/i )
    {
        $outputDir = shift;
    }
    elsif( $ARGV =~ m/-d2/i )
    {
        $DEBUG = 2;
    }
    elsif( $ARGV =~ m/-d/i )
    {
        $DEBUG = 1;
    }
    else
    {
        usage();
    }
}

#------------------------------------------------------------------------------
# Global variables
#------------------------------------------------------------------------------
my $compIdFile = $base."/src/occ_405/incl/comp_ids.h";
my $compPath =    $base."/src/occ_405";
my @justBmcPlugins = ("src/usr/pldm/plugins");
my $targetSrcFilename = "b2a00.py";
#------------------------------------------------------------------------------
# Call subroutines to populate the following arrays:
# - @reasonCodeFiles   (The list of files to parse through for reason codes)
# - @filesToParse      (The list of files to parse through for SRC tags)
# - @pluginDirsToParse (the list of plugin directories containing User Detail
#                       Data parsers)
#------------------------------------------------------------------------------
my @reasonCodeFiles;
my @filesToParse;
my @prdfFilesToParse;
my @pluginDirsToParse;
getReasonCodeFiles($compPath);

if ($DEBUG)
{
    print("---> ReasonCode files to process\n");
    foreach my $file(@reasonCodeFiles)
    {
        print("RC File: $file\n");
    }
}

getFilesToParse($compPath);

if ($DEBUG)
{
    print("---> Files to parse for error log tags\n");
    foreach my $file(@filesToParse)
    {
        print("File to Parse: $file\n");
    }
}


#------------------------------------------------------------------------------
# Process the compIdFile, recording all of the component ID values
#------------------------------------------------------------------------------
my %compIdToValueHash;
my %compIdToHexValueHash;

open(COMP_ID_FILE, $compIdFile) or die("Cannot open: $compIdFile: $!");

$compIdToValueHash{"NO_COMP"} = 0;
$compIdToHexValueHash{"NO_COMP"} = 0;
while (my $line = <COMP_ID_FILE>)
{
    # An example of the component ID line is:
    # const compId_t DEVFW_COMP_ID = 0x0200;
    if ($line =~ /^#define (\w+)\s+0x([\dA-Fa-f]+)00/)
    {
        my $compId = $1;
        my $compValue = $2;

        # Strip off any leading zeroes from the component value
        #$compValue =~ s/^0//g;

        $compIdToValueHash{$compId} = $compValue;
        # Need the integer value for compId sorting purposes
        $compIdToHexValueHash{$compId} = hex $compValue;
        if ($DEBUG > 1)
        {
            printf("...ADDING compIdToValueHash($compId) = $compValue\n");
        }
    }
}

close(COMP_ID_FILE);

if ($DEBUG)
{
    print("---> Component ID values\n");
    foreach my $key (keys %compIdToValueHash)
    {
        print ("  $key = 0x$compIdToValueHash{$key} ($compIdToHexValueHash{$key}),\n");
    }
}


#------------------------------------------------------------------------------
# Process the reasonCodeFiles, recording all of the module ids, reason codes
# and user detail data sections in hashes: The module ids and reason codes
# can be duplicated across components and so the namespace is also stored.
# Example hashes:
#
# %modIdToValueHash = (
#     'PNOR'   => { 'MOD_PNORDD_ERASEFLASH'     => '1A',
#                   'MOD_PNORDD_READFLASH'      => '12'},
#     'IBSCOM' => { 'IBSCOM_GET_TARG_VIRT_ADDR' => '03',
#                   'IBSCOM_SANITY_CHECK'       => '02'}
# );
#
# %rcToValueHash = (
#     'PNOR'   => { 'RC_UNSUPPORTED_OPERATION'  => '0607',
#                   'RC_STARTUP_FAIL'           => '0605'},
#     'IBSCOM' => { 'IBSCOM_INVALID_CONFIG'     => '1C03',
#                   'IBSCOM_INVALID_OP_TYPE'    => '1C02'}
# );
#
# %udIdToValueHash = (
#     'HWPF_UDT_HWP_RCVALUE' => '0x01',
#     'ERRL_UDT_CALLOUT'     => '0x07'
# );
#------------------------------------------------------------------------------
my %modIdToValueHash;
my %rcToValueHash;
my %ercToValueHash;
my %udIdToValueHash;

foreach my $file (@reasonCodeFiles)
{
    open(RC_FILE, $file) or die("Cannot open: $file: $!");
    my $processing = 0;
    my $processingModIds = 1;
    my $processingRcs = 2;
    my $processingUds = 3;
    my $processingERcs = 4;
    my $compId = "";
    my $enumName = "";
    my @taggedLines;

    while (my $line = <RC_FILE>)
    {
        if ($line =~ /enum.+.*ModuleId/i)
        {
            $processing = $processingModIds;
            next;
        }
        elsif ($line =~ /enum.+(.*ExtReasonCode)/i)
        {
            # avoids comment lines '@enum *ReasonCode'
            if ($line !~ /\s*\@/ )
            {
                # strip leading whitespace and trailing characters
                $enumName = $line;
                $enumName =~ s!^\s+!!;
                chomp($enumName);
                if ($DEBUG > 1)
                {
                    print "...found ERC $enumName in $file\n";
                }
            }
            $processing = $processingERcs;
            next;
        }
        elsif ($line =~ /enum.+(.*ReasonCode)/i)
        {
            # avoids comment lines '@enum *ReasonCode'
            if ($line !~ /\s*\@/ )
            {
                # strip leading whitespace and trailing characters
                $enumName = $line;
                $enumName =~ s!^\s+!!;
                chomp($enumName);
                if ($DEBUG > 1)
                {
                    print "...found RC $enumName in $file\n";
                }
            }
            $processing = $processingRcs;
            next;
        }
        elsif ($line =~ /enum.+UserDetail/i)
        {
            $processing = $processingUds;
            next;
        }
        elsif ($line =~ /}/)
        {
            $processing = 0;
            next;
        }

        if ($processing == $processingModIds)
        {
            # Example: "MAIN_MID                        =  MAIN_COMP_ID | 0x01,"
            if ($line =~ /(\w+)\s+=\s+(\w+)\s+\|\s+0x([\dA-Fa-f]+)/)
            {
                $modIdToValueHash{$1} = $compIdToValueHash{$2}.$3;
                if ($DEBUG > 1)
                {
                    printf("Adding modId: $1 = %s$3\t\t(%s|%s)\n", $compIdToValueHash{$2}, $2, $3);
                }
            }
        }
        elsif ($processing == $processingRcs)
        {
            if ($line !~ /\s*(\w+)\s+=\s+(\w+)\s+\|\s+0x([\dA-Fa-f]+)/)
            {
                # Reason code line does not contain Component ID
                # Example: "    INTERNAL_FAILURE                = 0x00,"
                if ($line =~ /\s*(\w+)\s+=\s+0x([\dA-Fa-f]+)/)
                {
                    $compId="NO_COMP";
                    if (! exists $compIdToValueHash{$compId})
                    {
                        print ("$0: Could not find CompID '$compId'\n");
                        exit(1);
                    }
                    $rcToValueHash{$1} = "2A" . $2;
                    if ($DEBUG > 1)
                    {
                        print "...ADDING rcToValueHash($1) = $rcToValueHash{$1}\n";
                    }
                }
            }
        }
        elsif ($processing == $processingERcs)
        {
            # Reason code line does not contain Component ID
            # Example: "    INTERNAL_FAILURE                = 0x00,"
            if ($line =~ /\s*(\w+)\s+=\s+0x([\dA-Fa-f]+)/)
            {
                my $ercName=$1;
                my $ercValue=$2;
                $ercToValueHash{$ercName} = $ercValue;
                if ($DEBUG > 1)
                {
                    print "...ADDING ercToValueHash($ercName) = $ercValue\n";
                }
            }
        }
        elsif ($processing == $processingUds)
        {
            # Example: "HWPF_UDT_HWP_RCVALUE       = 0x01,"
            if ($line =~ /(\w+)\s+=\s+(0x[\dA-Fa-f]+)/)
            {
               my $udId = $1;
               my $udValue = $2;

               if (exists($udIdToValueHash{$udId}))
               {
                   print ("$0: duplicate user data section in '$file'\n");
                   print ("$0: section is '$udId'\n");
                   exit(1);
               }

               $udIdToValueHash{$udId} = $udValue;
            }
        }
    }
    close(RC_FILE);
}
# print footer of file
print TERM_RC_FILE "};\n#endif\n";
close(TERM_RC_FILE);

if ($DEBUG)
{
    print("---> ModuleId values\n");
    foreach my $modIdKey (keys %modIdToValueHash)
    {
        print ("  $modIdKey = 0x$modIdToValueHash{$modIdKey}\n");
    }

    print("---> ReasonCode values\n");
    foreach my $rcKey (keys %rcToValueHash)
    {
        printf("  $rcKey = 0x%04s\n",$rcToValueHash{$rcKey});
    }

    print("---> User Detail Data Section values\n");
    foreach my $udKey (keys %udIdToValueHash)
    {
        print ("$udKey:$udIdToValueHash{$udKey}\n");
    }
}


#------------------------------------------------------------------------------
# Process the code files, looking for error log tags, save the parsing for each
# in a hash indexed by the component value
#------------------------------------------------------------------------------
my %rcModValuesUsed;
my %rcModValuesLineNos;
my %srcList;
my %PYdisplayDataEntries;

foreach my $file (@filesToParse)
{
    open(PARSE_FILE, $file) or die("Cannot open: $file: $!");
    if ($DEBUG) { printf("Parsing $file...\n"); }

    #-------------------------------------------------------------------------
    # Example Tag:
    #   /*@
    #     * @errortype
    #     * @reasoncode     I2C_INVALID_OP_TYPE
    #     * @severity       ERRL_SEV_UNRECOVERABLE
    #     * @moduleid       I2C_PERFORM_OP
    #     * @userdata1      i_opType
    #     * @userdata2      addr
    #     * @userdata3      data
    #     * @userdata4      ERC_GENERIC_ERROR
    #     * @devdesc        Invalid Operation type.
    #     * @custdec        Firmware Error
    #     */
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    my $srcLineNo = 0;
    my $errorTagLineNo = 0;
    while (my $line = <PARSE_FILE>)
    {
        ++$srcLineNo;

        if ($line =~ /\@errortype/)
        {
            # Found the start of an error log tag
            $errorTagLineNo = $srcLineNo;
            my $modId = "";
            my $modIdValue = "";
            my $rc = "";
            my $rcValue = "";
            my @userData;
            my @userDataTextOnly;
            my $desc = "";
            my $cdesc = "";
            my $ercValue="0000";

            # Read the entire error log tag into an array
            my @tag;

            while ($line = <PARSE_FILE>)
            {
                ++$srcLineNo;
                if ($line =~ /\*\//)
                {
                    # Found the end of an error log tag
                    last;
                }
                push(@tag, $line);
            }

            # Process the error log tag
            my $numLines = scalar (@tag);

            for (my $lineNum = 0; $lineNum < $numLines; $lineNum++)
            {
                $line = $tag[$lineNum];

                if ($line =~ /\@moduleid\s+(\S+)/i)
                {
                    # Found a moduleid, find out the value
                    $modId = $1;

                    if (exists $modIdToValueHash{$modId})
                    {
                        $modIdValue = $modIdToValueHash{$modId};
                        next;
                    }

                    if ($modIdValue eq "")
                    {
                        print ("$0: Error finding moduleid value for '$file:$line'\n");
                        exit(1);
                    }
                }
                elsif ($line =~ /\@reasoncode\s+(\S+)/i)
                {
                    # Found a reasoncode, figure out the value
                    $rc = $1;

                    if (exists $rcToValueHash{$rc})
                    {
                        $rcValue = "2A".substr($rcToValueHash{$rc},-2);
                        if ($DEBUG > 1)
                        {
                            printf("...found RC: $rcValue ($rc)\n");
                        }
                        next;
                    }

                    if ($rcValue eq "")
                    {
                        print ("$0: Error finding reasoncode value for '$file:$line'\n");
                        exit(1);
                    }
                }
                elsif ($line =~ /\@(userdata\S+)\s+(\S+.*)/i)
                {
                    # Found user data, strip out any double-quotes and trailing
                    # whitespace
                    my $udDesc = $1;
                    my $udText = $2;
                    $udText =~ s/\"//g;
                    $udText =~ s/\s+$//;
                    if ($udDesc eq "userdata4")
                    {
                        if (exists($ercToValueHash{$udText}))
                        {
                            $ercValue = $ercToValueHash{$udText};
                        }
                    }

                    # Look for follow-on lines
                    for ($lineNum++; $lineNum < $numLines; $lineNum++)
                    {
                        $line = $tag[$lineNum];

                        if ($line =~ /\@/)
                        {
                            # Found the next element, rewind
                            $lineNum--;
                            last;
                        }

                        # Continuation of user data, strip out any double-quotes
                        # and leading / trailing whitespace
                        $line =~ s/^.+\*\s+//;
                        $line =~ s/\"//g;
                        $line =~ s/\s+$//;

                        if ($line ne "")
                        {
                            $udText = $udText . " " . $line;
                        }
                    }

                    my $udString = "\"$udDesc\", \"$udText\"";
                    push(@userData, $udString);
                    push(@userDataTextOnly, $udText);
                }
                elsif ($line =~ /\@devdesc\s+(\S+.*)/i)
                {
                    # Found a description, strip out any double-quotes and
                    # trailing whitespace
                    $desc = $1;
                    $desc =~ s/\"//g;
                    $desc =~ s/\s+$//;

                    # Look for follow-on lines
                    for ($lineNum++; $lineNum < $numLines; $lineNum++)
                    {
                        $line = $tag[$lineNum];

                        if ($line =~ /\@/)
                        {
                            # Found the next element, rewind
                            $lineNum--;
                            last;
                        }

                        # Continuation of description, strip out any double-
                        # quotes and leading / trailing whitespace
                        $line =~ s/^.+\*\s+//;
                        $line =~ s/\"//g;
                        $line =~ s/\s+$//;

                        if ($line ne "")
                        {
                            $desc = $desc . " " . $line;
                        }
                    }
                }
                elsif ($line =~ /\@custdesc\s+(\S+.*)/i)
                {
                    # Found a customer description. Strip out any
                    # double-quotes and trailing whitespace
                    $cdesc = $1;
                    $cdesc =~ s/\"//g;
                    $cdesc =~ s/\s+$//;

                    # Look for follow-on lines
                    for ($lineNum++; $lineNum < $numLines; $lineNum++)
                    {
                        $line = $tag[$lineNum];

                        if ($line =~ /\@/)
                        {
                            # Found the next element, rewind
                            $lineNum--;
                            last;
                        }

                        # Continuation of description, strip out any
                        # double-quotes and leading / trailing
                        # whitespace
                        $line =~ s/^.+\*\s+//;
                        $line =~ s/\"//g;
                        $line =~ s/\s+$//;

                        if ($line ne "")
                        {
                            $cdesc = $cdesc . " " . $line;
                        }
                    }
                }
            }

            # Check that the required fields were found
            if ($modId eq "")
            {
                print ("$0: moduleid missing from error log tag in '$file'\n");
                print ("$0: reasoncode is '$rc'\n");
                exit(1);
            }

            if ($rc eq "")
            {
                print ("$0: reasoncode missing from error log tag in '$file'\n");
                print ("$0: moduleid is '$modId'\n");
                exit(1);
            }

            if ($desc eq "")
            {
                print ("$0: description missing from error log tag in '$file'\n");
                print ("$0: moduleid is '$modId', reasoncode is '$rc'\n");
                exit(1);
            }

            # if no customer desc is provided, then use generic message
            if (0 == 1) # TODO: Ignore warning for now
            {
                if ($cdesc eq "")
                {
                    $cdesc = "An internal firmware error occurred.";
                    print ("$0: customer description missing from error log tag in '$file'\n");
                    print ("$0:   moduleid is '$modId', reasoncode is '$rc'\n");
                    print ("$0:   using \"$cdesc\" as customer description\n");
                }
            }

            # SRC list - Don't add testcase SRCs
            if(not $file =~ /\/test\//)
            {
                my $srcText = sprintf("%04X", hex($rcValue));
                # eliminate dups
                if($srcList{$srcText} eq "")
                {
                    $srcList{$srcText} .= $cdesc;
                }
            }

            # Create the combined returncode/moduleid value that the parser looks for and
            # ensure that it is not a duplicate of one already found
            my $rcModValue = $rcValue . "-" . $modIdValue . "-" . $ercValue;
            {
                if ($DEBUG)
                {
                    print "...found PEL $rcModValue\n";
                }
            }

            my $shortFile = substr($file, length($base)+1);
            if (exists($rcModValuesUsed{$rcModValue}))
            {
                print ("$0: ERROR: duplicate moduleid/reasoncode tag ($rcModValue) in\n");
                my $i;
                for $i ( 0 .. $#{ $rcModValuesLineNos{$rcModValue} } )
                {
                    print "    $rcModValuesLineNos{$rcModValue}[$i]\n";
                }
                print "    $shortFile\:$errorTagLineNo\n";
                print ("$0:   moduleid is '$modId', reasoncode is '$rc'\n");
                exit(1);
            }

            $rcModValuesUsed{$rcModValue} = 1;
            push(@{ $rcModValuesLineNos{$rcModValue} }, $shortFile.":".$errorTagLineNo);

            # Create the Python version of the data entry code for this error
            my $pythonDataEntryCode = "      \"$rcModValue\": { \"devdesc\": \"$desc\",\n"
                                    . "                         \"moduleid\": \"$modId\",\n"
                                    . "                       \"reasoncode\": \"$rc\",\n";
            # Add all userdata sections to the python file. Note that there may be more than two sections,
            # since userdata1 or userdata2 could have sub-sections.
            foreach my $udString(@userData)
            {
                # The format of the string is "userdata1/2[bits]", "description". There is a comma
                # between the two fields. There may be commas in the description field as well,
                # but we need to ignore those, so split the string into just two using the first
                # comma.
                my ($userDetailsFormat, $detailsString) = split(",", $udString, 2);
                # Add spaces so that the lines align with the above format
                $pythonDataEntryCode = $pythonDataEntryCode . "                        ";
                $pythonDataEntryCode = $pythonDataEntryCode . $userDetailsFormat;
                $pythonDataEntryCode = $pythonDataEntryCode . ":";
                $pythonDataEntryCode = $pythonDataEntryCode . $detailsString;
                $pythonDataEntryCode = $pythonDataEntryCode . ",\n";
            }
            $pythonDataEntryCode = $pythonDataEntryCode . "                      },\n";

            # The component value is the first two characters of the 4 character rc
            my $compValue = $rcValue;
            $compValue =~ s/..$//;

            # Add the data entry code to PYdisplayDataEntries
            $PYdisplayDataEntries{$rcModValue} = $pythonDataEntryCode;
        }
    }

    close(PARSE_FILE);
}

#------------------------------------------------------------------------------
# Figure out the user detail data files to compile for each component
#------------------------------------------------------------------------------
my %compValToUdFilesHash;

foreach my $dir(@pluginDirsToParse)
{
    my $ofiles = "";
    my $compId = "";
    my $compVal = 0;

    # Open the directory and read all entries (files) skipping any beginning
    # with "."
    my @dirEntries;
    opendir(DH, $dir) or die("Cannot open $dir directory");
    @dirEntries = grep { !/^\./ } readdir(DH);
    closedir(DH);

    # The plugin directory must contain a <COMP_ID>_Parse.C file which contains
    # the user detail data parser for that component
    foreach my $file(@dirEntries)
    {
        if ($file =~ /^(.+).C$/)
        {
            # Found a C file add it to the files to compile list
            $ofiles .= "$1\.o ";

            if ($file =~ /^(.+COMP_ID)_Parse.C$/)
            {
                # Found the main Parse.C file for a component
                $compId = $1;
            }
        }
    }

    if ($compId eq "")
    {
        print ("$0: Could not find CompID Parser file in $dir\n");
        exit(1);
    }

    # Find the component ID value
    if (! exists $compIdToValueHash{"$compId"})
    {
        print ("$0: Could not find CompID $compId while processing $dir\n");
        exit(1);
    }
    $compVal = $compIdToValueHash{$compId};

    $compValToUdFilesHash{$compVal} = $ofiles;
}

#------------------------------------------------------------------------------
# Helper function for sort method, convert hex string to decimal and compare
#------------------------------------------------------------------------------
sub hexToDecCmp
{
   hex($a) <=> hex($b);
}

#------------------------------------------------------------------------------
# Generate the srcdisplaydata.py file that contains the dictionaries of Hostboot
# component IDs and detailed error data for the use of the python SRC parser.
#------------------------------------------------------------------------------

my $PYoutputFileName = $outputDir . "/" . $targetSrcFilename;
print("Creating $PYoutputFileName\n");
open(PYFILE, ">", $PYoutputFileName) or die("Cannot open: $PYoutputFileName: $!");
print PYFILE "\"\"\" Automatically generated by OCC's $0\n\n";
print PYFILE "The following dictionary contains the OCC SRC data with the key:\n";
print PYFILE "      \"2Arr-mmmm-eeee\": 2A = Component ID (OCC)\n";
print PYFILE "                        rr = OCC Reason Code\n";
print PYFILE "                        mmmm = OCC Module ID\n";
print PYFILE "                        eeee = OCC Extended Reason Code\n";
print PYFILE "\"\"\"\n\n";
print PYFILE "srcInfo = {\n";
foreach my $modID (sort hexToDecCmp keys(%PYdisplayDataEntries))
{
    print PYFILE $PYdisplayDataEntries{$modID};
}
print PYFILE "          }\n\n";

print PYFILE "import importlib\n";
print PYFILE "import json\n\n";
print PYFILE "def parseSRCToJson(refcode: str,\n";
print PYFILE "                   word2: str, word3: str, word4: str, word5: str,\n";
print PYFILE "                   word6: str, word7: str, word8: str, word9: str) -> str:\n";
print PYFILE "    \"\"\"\n";
print PYFILE "    SRC parser for OCC generated PELs.\n\n";
print PYFILE "    This returns a string containing formatted JSON data. The data is simply\n";
print PYFILE "    appended to the end of the \"Primary SRC\" section of the PEL and will not\n";
print PYFILE "    impact any other fields in that section.\n";
print PYFILE "    \"\"\"\n\n";
print PYFILE "    #refcode is in the form: Bsxxccrr\n";
print PYFILE "    #    where cc = Component ID\n";
print PYFILE "    #          rr = Reason Code\n\n";
print PYFILE "    #word9 (userdata4) is in the form: mmmmeeee\n";
print PYFILE "    #    where mmmm = OCC Module ID\n";
print PYFILE "    #          eeee = OCC Extended Reason Code\n\n";
print PYFILE "    #Search dictionary for corresponding key: ccrr-mmmm-eeee\n";
print PYFILE "    d = srcInfo.get(refcode[4:8] + '-' + word9[0:4] + '-' + word9[4:8])\n\n";
print PYFILE "    out = json.dumps(d, indent = 2)\n\n";
print PYFILE "    return out\n";

close(PYFILE);

#------------------------------------------------------------------------------
# Subroutine that prints the usage
#------------------------------------------------------------------------------
sub usage
{
    print "Usage: $0 <-b base> <-d> <-o output dir>\"\n";
    print "\n";
    print "-b:     Base directory containing Hostboot src directory (default is pwd)\n";
    print "-o:     Output directory where files are created (default is pwd)\n";
    print "-d      Enable Debug messages.\n";
    print "\n\n";
    exit 1;
}

#------------------------------------------------------------------------------
# Subroutine that updates @reasonCodeFiles
#------------------------------------------------------------------------------
sub getReasonCodeFiles
{
    # Open the input directory and read all entries (files/directories)
    # Skipping any beginning with "."
    my $inputDir = @_[0];
    my @dirEntries;
    opendir(DH, $inputDir) or die("Cannot open $inputDir directory");
    @dirEntries = grep { !/^\./ } readdir(DH);
    closedir(DH);

    foreach my $dirEntry (@dirEntries)
    {
        my $dirEntryPath = "$inputDir/$dirEntry";

        if (-d $dirEntryPath)
        {
            # Recursively call this function
            getReasonCodeFiles($dirEntryPath);
        }
        elsif( (($dirEntry =~ /reasoncodes/i) ||
                ($dirEntry =~ /service_codes/i) ||
                ($dirEntry =~ /examplerc/i))
              # Only look at header files (protects against scratch files)
              && ($dirEntry =~ /[Hh]$/) )
        {
            # Found reason-codes file
            push(@reasonCodeFiles, $dirEntryPath);
        }
    }
}

#------------------------------------------------------------------------------
# Subroutine that updates @filesToParse
#------------------------------------------------------------------------------
sub getFilesToParse
{
    # Open the input directory and read all entries (files/directories)
    # Skipping any beginning with "."
    my $inputDir = @_[0];
    my @dirEntries;
    opendir(DH, $inputDir) or die("Cannot open $inputDir directory");
    @dirEntries = grep { !/^\./ } readdir(DH);
    closedir(DH);

    if ($DEBUG)
    {
        print("getFilesToParse: scanning $inputDir\n");
    }
    #push(@filesToParse, "$inputDir/main.c");
    #return;

    foreach my $dirEntry (@dirEntries)
    {
        my $dirEntryPath = "$inputDir/$dirEntry";

        if (-d $dirEntryPath)
        {
            # Recursively call this function
            getFilesToParse($dirEntryPath);
        }
        elsif($dirEntry =~ /\.[H|h|C|c]$/)
        {
            # Found file to parse
            push(@filesToParse, $dirEntryPath);
        }
        # We will keep the prdf files separate from the rest of the list as we
        # only want to get the SRC listing from them and nothing else.
        #elsif($dirEntry =~ /\.[H|h|C|c]$/)
        #{
        #    # Found prdf file to parse
        #    push(@prdfFilesToParse, $dirEntryPath);
        #}
    }
}

#------------------------------------------------------------------------------
# Subroutine that updates @pluginDirsToParse
#------------------------------------------------------------------------------
sub getPluginDirsToParse
{
    # Open the input directory and read all entries (files/directories)
    # Skipping any beginning with "."
    my $inputDir = @_[0];
    my @dirEntries;
    opendir(DH, $inputDir) or die("Cannot open $inputDir directory");
    @dirEntries = grep { !/^\./ } readdir(DH);
    closedir(DH);

    foreach my $dirEntry (@dirEntries)
    {
        my $dirEntryPath = "$inputDir/$dirEntry";

        if (-d $dirEntryPath)
        {
            if ($dirEntryPath =~ /plugins/)
            {
                # Exclude plugin directories with just eBMC parsers
                my $skipDir = 0;
                foreach my $bmcOnlyPlugin (@justBmcPlugins)
                {
                    if ($dirEntryPath =~ $bmcOnlyPlugin)
                    {
                        $skipDir = 1;
                        last;
                    }
                }
                if (!$skipDir)
                {
                    push(@pluginDirsToParse, $dirEntryPath);
                }
            }
            else
            {
                # Exclude PRD directory
                if ( !($dirEntryPath =~ /prdf/) )
                {
                    # Recursively call this function
                    getPluginDirsToParse($dirEntryPath);
                }
            }
        }
    }
}

