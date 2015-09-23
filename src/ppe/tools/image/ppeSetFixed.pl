#!/usr/bin/perl
# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/tools/image/ppeSetFixed.pl $
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
# Purpose:  This perl script will parse the attribute and default list and
# and set the default values into the image.

use strict;

my $attrpath = "../../importtemp/xml";
my $sbedefaultpath = "../../script/image";

#------------------------------------------------------------------------------
# Print Command Line Help
#------------------------------------------------------------------------------
my $numArgs = $#ARGV + 1;
if ($numArgs < 3)
{
    print ("Usage: ppeSetFixed.pl <image> <attributes and default list> <attribute file> ...\n");
    print ("  This perl script will the attributes and default list to lookup the defaults\n");
    print ("  and parse the attribute file to lookup the types.\n");
    print ("  The default values will be set in the image.\n");
    print ("example:\n");
    print ("./ppeSetFixed.pl \\\n" );
    print (". \\\n" );
    print (" ../../sbe/obj/seeprom_main.bin \\\n" );
    print ("$attrpath/p9_ppe_attributes.xml \\\n" );
    print ("$attrpath/attribute_info/perv_attributes.xml \\\n" );
    print ("$attrpath/attribute_info/proc_attributes.xml \\\n" );
    print ("$attrpath/attribute_info/ex_attributes.xml \\\n" );
    print ("$attrpath/attribute_info/eq_attributes.xml \\\n" );
    print ("$attrpath/attribute_info/core_attributes.xml \n");
    exit(1);
}

#------------------------------------------------------------------------------
# Specify perl modules to use
#------------------------------------------------------------------------------
use XML::Simple;
my $xml = new XML::Simple (KeyAttr=>[]);


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
my $sbedefaultpath = $ARGV[0];
my $image = $ARGV[1];
my $argfile = $ARGV[2];
my $entries = $xml->XMLin($argfile, ForceArray => ['entry']);

if ( ! -e $image) {die "ppeSetFixed.pl: $image $!"};

foreach my $entr (@{$entries->{entry}}) {

    my $inname = $entr->{name};

    # read XML file. The ForceArray option ensures that there is an array of
    # elements even if there is only one such element in the file

    foreach my $argnum (3 .. $#ARGV)
    {
        my $infile = $ARGV[$argnum];

        if ( ! -e $infile) {die "ppeSetFixed.pl: $infile $!"};

        my $attributes = $xml->XMLin($infile, ForceArray => ['attribute']);

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
                print ("ppeSbeFixed.pl ERROR. Att 'id' missing\n");
                exit(1);
            }


            if($attr->{targetType} eq "TARGET_TYPE_PROC_CHIP") {

                push(@attrChipIds, $entr);

            } elsif($attr->{targetType} eq "TARGET_TYPE_CORE") {

                push(@attrCoreIds, $entr);
                
            } elsif($attr->{targetType} eq "TARGET_TYPE_EQ") {

                push(@attrEqIds, $entr);

            } elsif($attr->{targetType} eq "TARGET_TYPE_EX") { 

                push(@attrExIds, $entr);

            } elsif($attr->{targetType} eq "TARGET_TYPE_PERV") { 

                push(@attrPervIds, $entr);

            } else {

                print ("ppeSetFixed.pl ERROR. Wrong attribute type: $attr->{targetType}\n");
                exit(1);

            }

            }
        }
    }

}



setFixed("TARGET_TYPE_PROC_CHIP", @attrChipIds);
setFixed("TARGET_TYPE_CORE", @attrCoreIds);
setFixed("TARGET_TYPE_EQ", @attrEqIds);
setFixed("TARGET_TYPE_EX", @attrExIds);
setFixed("TARGET_TYPE_PERV", @attrPervIds);



sub setFixed {
 
    my ($string, @entries) =  @_;

foreach my $attr (@entries)
{

    my $inname = $attr->{name};
    
    my @values = $attr->{value};


    if(scalar @values > 0) {

    foreach my $val (@values)
    {

        if(defined $val && ref($val) eq "") {

            if ($val =~ /(0x)?[0-9a-fA-F]+/) {
             
                my $systemRc = system("$sbedefaultpath/sbe_default_tool $image $inname $val $string 0");
        
                if ($systemRc) {
                    print "sbe_default_tool: error in execution\n";
                    exit 1;
                }
                
            } else {
                print ("ppeSetFixed.pl ERROR. not hex\n");
                exit(1);
            }

        } elsif(defined $val && ref($val) eq "ARRAY") {

            my $index = 0;

            foreach my $arr (@{$val}) {

                if(defined $arr && ref($arr) eq "") {
                    if ($arr =~ /(0x)?[0-9a-fA-F]+/) {

                        my $systemRc = system("$sbedefaultpath/sbe_default_tool $image $inname $arr $string $index");

                        if ($systemRc) {
                            print "sbe_default_tool: error in execution\n";
                            exit 1;
                        }


                    }
                }
                $index++;
            }



        }


    }
    }


}

}


