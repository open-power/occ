#!/usr/bin/perl
# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/tools/scripts/ppeParseProcSbeFixed.pl $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2015
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
# Purpose:  This perl script will parse HWP Attribute XML files and
# initfile attr files and create the fapiGetInitFileAttr() function
# in a file called fapiAttributeService.C

use strict;

#------------------------------------------------------------------------------
# Print Command Line Help
#------------------------------------------------------------------------------
my $numArgs = $#ARGV + 1;
if ($numArgs < 3)
{
    print ("Usage: ppeParseProcSbeFixed.pl <output dir>\n");
    print ("           [<if-attr-file1> <if-attr-file2> ...]\n");
    print ("           -a <attr-xml-file1> [<attr-xml-file2> ...]\n");
    print ("  This perl script will parse if-attr files (containing the\n");
    print ("  attributes used by the initfile) and attribute XML files\n");
    print ("  (containing all HWPF attributes) and create the\n");
    print ("  fapiGetInitFileAttr() function in a file called\n");
    print ("  fapiAttributeService.C. Only the attributes specified in\n");
    print ("  the if-attr files are supported. If no if-attr files are\n");
    print ("  specified then all attributes are supported\n");
    exit(1);
}

#------------------------------------------------------------------------------
# Specify perl modules to use
#------------------------------------------------------------------------------
use XML::Simple;
my $xml = new XML::Simple (KeyAttr=>[]);

# Uncomment to enable debug output
#use Data::Dumper;

#------------------------------------------------------------------------------
# Open output file for writing
#------------------------------------------------------------------------------
my $chipFile = $ARGV[0];
$chipFile .= "/";
$chipFile .= "proc_sbe_fixed_proc_chip.H";
open(CHFILE, ">", $chipFile);

my $exFile = $ARGV[0];
$exFile .= "/";
$exFile .= "proc_sbe_fixed_ex.H";
open(EXFILE, ">", $exFile);

my $coreFile = $ARGV[0];
$coreFile .= "/";
$coreFile .= "proc_sbe_fixed_core.H";
open(COFILE, ">", $coreFile);

my $eqFile = $ARGV[0];
$eqFile .= "/";
$eqFile .= "proc_sbe_fixed_eq.H";
open(EQFILE, ">", $eqFile);

my $pervFile = $ARGV[0];
$pervFile .= "/";
$pervFile .= "proc_sbe_fixed_perv.H";
open(PEFILE, ">", $pervFile);



my $xmlFiles = 0;
my $attCount = 0;
my $numIfAttrFiles = 0;
my @attrChipIds;
my @attrExIds;
my @attrCoreIds;
my @attrEqIds;
my @attrPervIds;



#------------------------------------------------------------------------------
# Element names
#------------------------------------------------------------------------------
my $attribute = 'attribute';

#------------------------------------------------------------------------------
# For each argument
#------------------------------------------------------------------------------
my $argfile = $ARGV[1];
my $entries = $xml->XMLin($argfile, ForceArray => ['entry']);
foreach my $entr (@{$entries->{entry}}) {

    my $inname = $entr->{name};

    # read XML file. The ForceArray option ensures that there is an array of
    # elements even if there is only one such element in the file

    foreach my $argnum (2 .. $#ARGV)
    {
        my $infile = $ARGV[$argnum];

        my $attributes = $xml->XMLin($infile, ForceArray => ['attribute']);

        # Uncomment to get debug output of all attributes
        #print "\nFile: ", $infile, "\n", Dumper($attributes), "\n";

        #--------------------------------------------------------------------------
        # For each Attribute
        #--------------------------------------------------------------------------
        foreach my $attr (@{$attributes->{attribute}})
        {

            if($attr->{id} eq $inname) {

            #------------------------------------------------------------------
            # Check that the AttributeId exists
            #------------------------------------------------------------------
            if (! exists $attr->{id})
            {
                print ("fapiParseAttributeInfo.pl ERROR. Att 'id' missing\n");
                exit(1);
            }


            if($attr->{targetType} eq "TARGET_TYPE_PROC_CHIP") {

                #push(@attrChipIds, $attr->{id});
                push(@attrChipIds, $attr);

            } elsif($attr->{targetType} eq "TARGET_TYPE_CORE") {

   #             push(@attrCoreIds, $attr->{id});
                push(@attrCoreIds, $attr);
                
            } elsif($attr->{targetType} eq "TARGET_TYPE_EQ") {

  #              push(@attrEqIds, $attr->{id});
                push(@attrEqIds, $attr);

            } elsif($attr->{targetType} eq "TARGET_TYPE_EX") { 

 #               push(@attrExIds, $attr->{id});
                push(@attrExIds, $attr);

            } elsif($attr->{targetType} eq "TARGET_TYPE_PERV") { 

#                push(@attrPervIds, $attr->{id});
                push(@attrPervIds, $attr);

            } else {

                print ("ppeParseProcSbeFixed.pl ERROR. Wrong attribute type: $attr->{targetType} for attribute $attr->{id} in $infile\n");
                exit(1);

            }

            }
        }
    }

}


print CHFILE "// proc_sbe_fixed_proc_chip.H\n";
print CHFILE "// This file is generated by perl script ppeParseProcSbeFixed.pl\n\n";
print CHFILE "#ifndef __PROC_SBE_FIXED_PROC_CHIP_H__\n";
print CHFILE "#define __PROC_SBE_FIXED_PROC_CHIP_H__\n\n";
foreach my $attr (@attrChipIds)
{

    my $value = uc $attr->{valueType};
    print CHFILE "PROC_SBE_FIXED_$value($attr->{id});\n"
    

}
print CHFILE "\n#endif  // __PROC_SBE_FIXED_PROC_CHIP_H__";

print EXFILE "// proc_sbe_fixed_ex.H\n";
print EXFILE "// This file is generated by perl script ppeParseProcSbeFixed.pl\n\n";
print EXFILE "#ifndef __PROC_SBE_FIXED_EX_H__\n";
print EXFILE "#define __PROC_SBE_FIXED_EX_H__\n";
foreach my $attr (@attrExIds)
{

    my $value = uc $attr->{valueType};
    print EXFILE "PROC_SBE_FIXED_TARGET_$value($attr->{id}, EX_TARGET_COUNT);\n"
    

}
print EXFILE "\n#endif  // __PROC_SBE_FIXED_EX_H__";


print COFILE "// proc_sbe_fixed_co.H\n";
print COFILE "// This file is generated by perl script ppeParseProcSbeFixed.pl\n\n";
print COFILE "#ifndef __PROC_SBE_FIXED_CO_H__\n";
print COFILE "#define __PROC_SBE_FIXED_CO_H__\n";
foreach my $attr (@attrCoreIds)
{

    my $value = uc $attr->{valueType};
    print COFILE "PROC_SBE_FIXED_TARGET_$value($attr->{id}, CORE_TARGET_COUNT);\n"
    

}
print COFILE "\n#endif  // __PROC_SBE_FIXED_CO_H__";



print EQFILE "// proc_sbe_fixed_eq.H\n";
print EQFILE "// This file is generated by perl script ppeParseProcSbeFixed.pl\n\n";
print EQFILE "#ifndef __PROC_SBE_FIXED_EQ_H__\n";
print EQFILE "#define __PROC_SBE_FIXED_EQ_H__\n";
foreach my $attr (@attrEqIds)
{

    my $value = uc $attr->{valueType};
    print EQFILE "PROC_SBE_FIXED_TARGET_$value($attr->{id}, EQ_TARGET_COUNT);\n"
    

}
print EQFILE "\n#endif  // __PROC_SBE_FIXED_EQ_H__";



print PEFILE "// proc_sbe_fixed_perv.H\n";
print PEFILE "// This file is generated by perl script ppeParseProcSbeFixed.pl\n\n";
print PEFILE "#ifndef __PROC_SBE_FIXED_PERV_H__\n";
print PEFILE "#define __PROC_SBE_FIXED_PERV_H__\n";
foreach my $attr (@attrPervIds)
{

    my $value = uc $attr->{valueType};
    print PEFILE "PROC_SBE_FIXED_TARGET_$value($attr->{id}, PERV_TARGET_COUNT);\n"
    

}
print PEFILE "\n#endif  // __PROC_SBE_FIXED_PERV_H__";








#print ASFILE "#endif  // __PROC_SBE_FIXED_H__";


#------------------------------------------------------------------------------
# Close output file
#------------------------------------------------------------------------------
close(CHFILE);
close(COFILE);
close(EXFILE);
close(PEFILE);
close(EQFILE);

