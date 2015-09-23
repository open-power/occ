#!/usr/bin/perl -w
# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/tools/scripts/ppeCreateAttrGetSetMacros.pl $
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
#find enums in AttributeId
#for each enum check for ${enum}_Type
#check for type and array values
#Check Plat file for ${enum}_GETMACRO and ${enum}_SETMACRO
#If they do not exist add apporpriate _SETMACRO and _GETMACRO to Plat file

use strict;
use File::Copy;
use Getopt::Long;

sub enumParser ($);
sub help;

my $DEBUG = 0;
my $VERBOSE = 0;

my $state = 0;
my $last_value = -1;
my $current_entry;
my $current_enum_name;

my %enums;

my %attributeTypes;
my %attributeArrayTypes;

my %getMacros;
my %setMacros;
my %targetMacros;



my $fapiAttributeIdsFile = "fapi2AttributeIds.H";
my $fapiPlatAttributeServiceFile= "fapi2PlatAttributeService.H";
my $fapiPlatAttributeServiceImpl= "fapi2PlatAttributeService.C";


my $includePath = "./";
my $srcPath = "./";

my @newAttributeDefines;
my @newTargetDefines;
my @newTargetImplementations;


my $servicePath;
my $help;

GetOptions ("verbose" => \$VERBOSE, 
            "help" => \$help,
            "debug" => \$DEBUG,
            "path=s" => \$servicePath,
            "inc=s" => \$includePath,
            "src=s" => \$srcPath,
);

help() if $help;

open (FILE, $includePath . "/" . $fapiAttributeIdsFile) or die "ERROR:: could not open $fapiAttributeIdsFile\n";

while (<FILE>) {
  # attempt to parse attributes from current line in file
  enumParser($_);

  # see if the line describes an attribute
  if (m/\s*typedef\s+(\w+)\s+(\w+)_Type(\S*)\s*;/) {
    my $type = $1;
    my $attribute = $2;
    my $arrayType = $3;

    if ($DEBUG) { print "DEBUG:: type = $type : attribute = $attribute : arrayType = $arrayType\n"; }

    # save attribute type and if it is an array and its size
    $attributeTypes{$attribute} = $type;
    if ($arrayType) {
      $attributeArrayTypes{$attribute} = $arrayType;
    } else {
      $attributeArrayTypes{$attribute} = "none";
    }
  }

  # look for MACROs
  # look for: #define ATTR_CHIP_HAS_SBE_GETMACRO ATTRIBUTE_NOT_WRITABLE
  if (m/\s*#define\s+(\w+)_GETMACRO\s+(\S+)\s*/) {
    $getMacros{$1} = $2;
    if ($DEBUG) { print "DEBUG:: attribute = $1 : GETMACRO = $2\n"; }
  # look for: #define ATTR_CHIP_EC_FEATURE_TEST1_GETMACRO(ID, PTARGET, VAL) fapi::fapiQueryChipEcFeature(ID, PTARGET, VAL)
  } elsif (m/\s*#define\s+(\w+)_GETMACRO\(ID\,\sPTARGET\,\sVAL\)\s(.+)/) {
    $getMacros{$1} = $2;
    if ($DEBUG) { print "DEBUG:: attribute = $1 : GETMACRO = $2\n"; }
  # look for: #define ATTR_CHIP_HAS_SBE_SETMACRO ATTRIBUTE_NOT_WRITABLE
  } elsif (m/\s*#define\s+(\w+)_SETMACRO\s+(\S+)\s*/) {
    $setMacros{$1} = $2;
    if ($DEBUG) { print "DEBUG:: attribute = $1 : SETMACRO = $2\n"; }
  # look for: #define ATTR_CHIP_EC_FEATURE_TEST2_SETMACRO(ID, PTARGET, VAL) CHIP_EC_FEATURE_ATTRIBUTE_NOT_WRITABLE
  } elsif (m/\s*#define\s+(\w+)_SETMACRO\(ID\,\sPTARGET\,\sVAL\)\s(.+)/) {
    $setMacros{$1} = $2;
    if ($DEBUG) { print "DEBUG:: attribute = $1 : SETMACRO = $2\n"; }
  } elsif (m/\s*const\s*TargetTypes_t\s+(\w+)_TargetTypes\s*=\s*(\S+)\s*;\s*/) {
    $targetMacros{$1} = $2;
#    print "DEBUG:: attribute = $1 : TARGET = $2\n";
    if ($DEBUG) { print "DEBUG:: attribute = $1 : TARGET = $2\n"; }
  }
}

close (FILE);

#find copy of fapiPlatAttributeService.H
if (!$servicePath) {
  #$CTEPATH/tools/ecmd/$ECMD_RELEASE/ext/fapi/capi
  my $ctepath = $ENV{CTEPATH};
  my $ecmd_release = $ENV{ECMD_RELEASE};
  if ($DEBUG) { print "DEBUG:: ctepath = $ctepath\n"; }
  if ($DEBUG) { print "DEBUG:: ecmd_release = $ecmd_release\n"; }
  if (!$ctepath) {
    print "ERROR:: environment variable CTEPATH not defined!\n";
    exit 1;
  }
  if (!$ecmd_release) {
    print "ERROR:: environment variable ECMD_RELEASE not defined!\n";
    exit 1;
  }
  $servicePath = "$ctepath/tools/ecmd/$ecmd_release/ext/fapi/capi";
}

if ($DEBUG) { print "DEBUG:: servicePath = $servicePath\n"; }

# test that servicePath exists
if (!-d $servicePath) {
  print "ERROR:: path $servicePath does not exist!\n";
  exit 1;
}

# test that fapiPlatAttributeService.H is in that directory
if (!-f "$servicePath/$fapiPlatAttributeServiceFile") {
  print "ERROR:: $fapiPlatAttributeServiceFile does not exist in $servicePath\n";
  exit 1;
}

# copy fapiPlatAttributeService.H to local dir
#my $systemRc = system("cp $servicePath/$fapiPlatAttributeServiceFile $includePath");
copy("$servicePath/$fapiPlatAttributeServiceFile","$includePath") or die "Copy failed: $!";

#if ($systemRc) {
#  print "ERROR:: error copying $fapiPlatAttributeServiceFile from $servicePath\n";
#  exit 1;
#}



# look in fapiPlatAttributeService.H for MACROs
open (FILE, $includePath . "/". $fapiPlatAttributeServiceFile) or die "ERROR:: could not open $fapiPlatAttributeServiceFile\n";
while (<FILE>) {
  if (m/\s*#define\s+(\w+)_GETMACRO\s+(\S+)\s*/) {
    $getMacros{$1} = $2;
    if ($DEBUG) { print "DEBUG:: attribute = $1 : GETMACRO = $2\n"; }
  } elsif (m/\s*#define\s+(\w+)_SETMACRO\s+(\S+)\s*/) {
    $setMacros{$1} = $2;
    if ($DEBUG) { print "DEBUG:: attribute = $1 : SETMACRO = $2\n"; }
  } elsif (m/\s*const\s*TargetTypes_t\s+(\w+)_TargetTypes\s*=\s*(\S+)\s*;\s*/) {
    $targetMacros{$1} = $2;
    if ($DEBUG) { print "DEBUG:: attribute = $1 : TARGET = $2\n"; }
  }
}
close (FILE);

# go through attributes found in file
for my $attribute (sort keys %{$enums{AttributeId}}) {
  if ($DEBUG) { print "DEBUG:: attribute = $attribute\n"; }
  my $type;
  my $arrayType;
  my $dimension = 0;

  # check that each attribute has attributeType
  if ($attributeTypes{$attribute}) {
    if ($attributeArrayTypes{$attribute}) {
      $type = $attributeTypes{$attribute};
      $arrayType = $attributeArrayTypes{$attribute};
    } else {
      print "ERROR:: arrayType not found for $attribute\n";
      next;
    }
  } else {
    print "ERROR:: type not found for $attribute\n";
    next;
  }

  # determine if attribute is an array
  if ($arrayType =~ m/none/) {
    if ($DEBUG) { print "DEBUG:: $attribute = $type\n"; }
  } else {
    # find dimension for array
    my $tempArrayType = $arrayType;
    while ($tempArrayType =~ m/\[\d*\].*/) {
      $dimension++;
      $tempArrayType =~ s/\[\d*\]//;
    }
    if ($DEBUG) { print "DEBUG:: $attribute = $type$arrayType dimension = $dimension\n"; }
  }

  my $setMacro = $setMacros{$attribute};
  my $getMacro = $getMacros{$attribute};
  my $targetMacro = $targetMacros{$attribute};

#  print "$attribute $setMacro $getMacro $targetMacro \n";

  # if an attribute is missing the SET or GET MACRO add to list in insert into file later
  if (!($getMacro && $setMacro)) {
    my $macroPrefix = "PLAT_ATTR_";
    my $macroPostfix = "";

    if ($dimension == 0) {
      $macroPostfix = "_GLOBAL_INT";
    } else {
      if ($type =~ m/uint8_t/) {
        $macroPostfix = "_UINT8_" . $dimension . "D_ARRAY";
      } elsif ($type =~ m/uint32_t/) {
        $macroPostfix = "_UINT32_" . $dimension . "D_ARRAY";
      } elsif ($type =~ m/uint64_t/) {
        $macroPostfix = "_UINT64_" . $dimension . "D_ARRAY";
      } elsif ($type =~ m/int8_t/) {
        $macroPostfix = "_INT8_" . $dimension . "D_ARRAY";
      } elsif ($type =~ m/int32_t/) {
        $macroPostfix = "_INT32_" . $dimension . "D_ARRAY";
      } elsif ($type =~ m/int64_t/) {
        $macroPostfix = "_INT64_" . $dimension . "D_ARRAY";
      } else {
        print "ERROR:: unknown type $type for attribute $attribute\n";
        next;
      }
    }

    my $macroTarget = "";
    if(defined $targetMacro) {
        if($targetMacro eq "TARGET_TYPE_PROC_CHIP") {
            $macroTarget = "ProcChipAttributes_t";
        } elsif ($targetMacro eq "TARGET_TYPE_CORE") {
            $macroTarget = "CoreAttributes_t";
        } elsif ($targetMacro eq "TARGET_TYPE_EX") {
            $macroTarget = "EXAttributes_t";
        } elsif ($targetMacro eq "TARGET_TYPE_EQ") {
            $macroTarget = "EQAttributes_t";
        } elsif ($targetMacro eq "TARGET_TYPE_PERV") {
            $macroTarget = "PervAttributes_t";
        } else {
        print "ERROR:: unknown type $targetMacro for attribute $attribute\n";
        next;
      }
    }

    if (!$getMacro) {
      if ($VERBOSE) { print "INFO:: did not find ${attribute}_GETMACRO\n"; }
      my $attributeDefine = "#define ${attribute}_GETMACRO ${macroPrefix}GET${macroPostfix}";
      push(@newAttributeDefines, $attributeDefine);
      
      if(defined $targetMacro) {


          my $targetFunction = "template<> void __get<fapi2::$targetMacro, fapi2attr::$macroTarget, $type, fapi2::${attribute}>   ( const fapi2::Target<fapi2::$targetMacro>* i_ptarget, fapi2attr::$macroTarget* object, const fapi2::AttributeId attrid, $type *value )";
          push(@newTargetDefines, $targetFunction . ";");

          my $targetImplementation = "";
          if($targetMacro eq "TARGET_TYPE_PROC_CHIP") {

              $targetImplementation .= "\n" . $targetFunction . "\n{\n   *value = object->fapi2attr::${macroTarget}::${attribute};\n}\n";
           
          } else {

              $targetImplementation .= "\n" . $targetFunction . "\n{\n   uint32_t index = (i_ptarget)->getTargetNumber();\n   *value = object->fapi2attr::${macroTarget}::${attribute}[index];\n}\n";

          }
          push(@newTargetImplementations, $targetImplementation);
      }
    }
    if (!$setMacro) {
      if ($VERBOSE) { print "INFO:: did not find ${attribute}_SETMACRO\n"; }
      my $attributeDefine = "#define ${attribute}_SETMACRO ${macroPrefix}SET${macroPostfix}";
      push(@newAttributeDefines, $attributeDefine);

      if(defined $targetMacro) {

          my $targetFunction = "template<> void __set<fapi2::$targetMacro, fapi2attr::$macroTarget, $type, fapi2::${attribute}>   ( const fapi2::Target<fapi2::$targetMacro>* i_ptarget, fapi2attr::$macroTarget* object, const fapi2::AttributeId attrid,  $type* value )";
          push(@newTargetDefines, $targetFunction . ";");

          my $targetImplementation = "";

          if($targetMacro eq "TARGET_TYPE_PROC_CHIP") {

              $targetImplementation = "\n" . $targetFunction . "\n{\n  object->fapi2attr::${macroTarget}::${attribute} = *value;\n}\n";
          } else {
              $targetImplementation = "\n" . $targetFunction . "\n{\n   uint32_t index = (i_ptarget)->getTargetNumber();\n   object->fapi2attr::${macroTarget}::${attribute}[index] = *value;\n}\n";
          }

          push(@newTargetImplementations, $targetImplementation);
      }
    }
  }
}

# if file is missing any GET or SET MACROs
if (@newAttributeDefines != 0) {

  my $updatedFapiPlatAttributeServiceFile = "$fapiPlatAttributeServiceFile.temp";
  open (OUTFILE, ">$updatedFapiPlatAttributeServiceFile") or die "ERROR:: could not open $updatedFapiPlatAttributeServiceFile\n";
  open (FILE, $includePath . "/" . $fapiPlatAttributeServiceFile) or die "ERROR:: could not open $fapiPlatAttributeServiceFile\n";

  my $insertTagFound = 0;

  while (<FILE>) {
    print OUTFILE $_;
    # search for tag to insert after
    if (m/\/\*.*INSERT NEW ATTRIBUTES HERE.*\*\//) {
      $insertTagFound = 1;
      # insert missing GET or SET MACROs
      print OUTFILE "\n";
      foreach my $attributeDefine (@newAttributeDefines) {
        print OUTFILE "$attributeDefine\n";
        if ($VERBOSE) { print "INFO:: adding $attributeDefine\n"; }
      }
    }

    if (m/\/\*.*INSERT NEW GETTER AND SETTER FUNCTIONS HERE.*\*\//) {

      $insertTagFound = 1;
      # insert missing GET or SET MACROs
      print OUTFILE "\n";
      foreach my $targetDefine (@newTargetDefines) {
        print OUTFILE "$targetDefine\n";
        if ($VERBOSE) { print "INFO:: adding getter setter method\n"; }
      }
    }


  }
  close (FILE);
  close (OUTFILE);

  if ($insertTagFound == 0) {
    # remove file that we did not update
    system("rm $updatedFapiPlatAttributeServiceFile");
    print ("WARNING:: did not find tag \"INSERT NEW ATTRIBUTES HERE\" in $fapiPlatAttributeServiceFile. no updates performed.\n");
  } else {
    # copy new file over the old one
    system("mv $updatedFapiPlatAttributeServiceFile $includePath/$fapiPlatAttributeServiceFile");
  }


  my $updatedFapiPlatAttributeServiceImpl = $srcPath . "/" . $fapiPlatAttributeServiceImpl; 
  open (OUTFILE, ">$updatedFapiPlatAttributeServiceImpl") or die "ERROR:: could not open $updatedFapiPlatAttributeServiceImpl\n";

  print OUTFILE "// $fapiPlatAttributeServiceImpl\n";
  print OUTFILE "// This file is generated by perl script ppeCreateAttrGetSetMacros.pl\n\n";

  print OUTFILE "#include <fapi2.H>\n";
  print OUTFILE "#include <fapi2PlatAttributeService.H>\n";
  print OUTFILE "#include \"proc_sbe_fixed.H\"\n";
  print OUTFILE "#include \"plat_target_parms.H\"\n\n";
  print OUTFILE "namespace fapi2 {\n";


  foreach my $impl (@newTargetImplementations) {
   
      print OUTFILE $impl;

  }

  print OUTFILE "\n} // namespace fapi2\n";


  close (OUTFILE);
}




exit;

# enumParser state machine
#     "enum"      "enum_name"      "{"        "entry"          "}"
# [0] ------> [1] -------------> [2] ---> [3] -----------> [4] -------------------------------------> [7]
#                                                      ","     "="       "value"       "}"
#                                         [3] <----------- [4] ----> [5] --------> [6] -------------> [7]
#                                              "}"                                                        ";"
#                                         [3]  -----------------------------------------------------> [7] ---> [0]
#                                                                              ","
#
#                                         [3] <----------------------------------- [6]
#

sub enumParser ($){
  my $line = $_[0];
  chomp($line);

  if ($DEBUG) { print "DEBUG:: state = $state : line = \"$line\"\n"; }

  if ($state == 0) {
    # find enum as first word
    if ($line =~ m/\s*enum\s*.*/) {
      if ($DEBUG) { print "DEBUG:: found enum in $line\n"; }
      $state = 1;
      # reset last_value
      $last_value = -1;
      # reset current_entry
      $current_entry = "";
      # reset current_enum_name;
      $current_enum_name = "";
      # remove enum from line recheck
      $line =~ s/\s*enum\s*//;
      enumParser($line);
    }
  } elsif ($state == 1) {
    # find ENUM_NAME as first word
    if ($line =~ m/\s*(\w+)\s*.*/) {
      if ($DEBUG) { print "DEBUG:: found ENUM_NAME $1 in $line\n"; }
      $state = 2;
      my $enum_name = $1;
      $current_enum_name = $enum_name;
      # remove ENUM_NAME from line
      $line =~ s/\s*$enum_name\s*//;
      #recheck
      enumParser($line);
    }
  } elsif ($state == 2) {
    # find { as first word
    if ($line =~ m/\s*{\s*.*/) {
      if ($DEBUG) { print "DEBUG:: found { in $line\n"; }
      $state = 3;
      # remove { from line recheck
      $line =~ s/\s*{\s*//;
      enumParser($line);
    }
  } elsif ($state == 3) {
    # find ENTRY as first word
    if ($line =~ m/\s*(\w+)\s*.*/) {
      if ($DEBUG) { print "DEBUG:: found ENTRY $1 in $line\n"; }
      my $entry = $1;
      $current_entry = $entry;
      # remove ENTRY from line
      $line =~ s/\s*$entry\s*//;
      $state = 4;
      #recheck
      enumParser($line);
    }
    # find } as first word
    elsif ($line =~ m/\s*}\s*.*/) {
      if ($DEBUG) { print "DEBUG:: found } in $line\n"; }
      $state = 7;
      # remove } from line recheck
      $line =~ s/\s*}\s*//;
      enumParser($line);
    }
  } elsif ($state == 4) {
    # find = as first word
    if ($line =~ m/\s*=\s*.*/) {
      if ($DEBUG) { print "DEBUG:: found = in $line\n"; }
      $state = 5;
      # remove = from line recheck
      $line =~ s/\s*=\s*//;
      enumParser($line);
    }
    # find , as first word
    elsif ($line =~ m/\s*,\s*.*/) {
      if ($DEBUG) { print "DEBUG:: found , in $line\n"; }
      $state = 3;
      # assign next last_value to entry
      $last_value++;
      my $value = $last_value;
      if ($DEBUG) { print "DEBUG:: default VALUE $value assigned to $current_entry\n"; }
      $enums{$current_enum_name}{$current_entry} = $value;
      # remove , from line recheck
      $line =~ s/\s*,\s*//;
      enumParser($line);
    }
    # find } as first word
    elsif ($line =~ m/\s*}\s*.*/) {
      if ($DEBUG) { print "DEBUG:: found } in $line\n"; }
      $state = 7;
      # remove } from line recheck
      $line =~ s/\s*}\s*//;
      enumParser($line);
    }
  } elsif ($state == 5) {
    # find VALUE as first word
    if ($line =~ m/\s*(\w+)\s*.*/) {
      if ($DEBUG) { print "DEBUG:: found VALUE $1 in $line\n"; }
      my $value = $1;
      $last_value = $value;
      # assign value to entry
      if ($DEBUG) { print "DEBUG:: VALUE $value assigned to $current_entry\n"; }
      $enums{$current_enum_name}{$current_entry} = $value;
      # remove VALUE from line
      $line =~ s/\s*$value\s*//;
      $state = 6;
      #recheck
      enumParser($line);
    }
  } elsif ($state == 6) {
    # find , as first word
    if ($line =~ m/\s*,\s*.*/) {
      if ($DEBUG) { print "DEBUG:: found , in $line\n"; }
      $state = 3;
      # remove , from line recheck
      $line =~ s/\s*,\s*//;
      enumParser($line);
    }
    # find } as first word
    elsif ($line =~ m/\s*}\s*.*/) {
      if ($DEBUG) { print "DEBUG:: found } in $line\n"; }
      $state = 7;
      # remove } from line recheck
      $line =~ s/\s*}\s*//;
      enumParser($line);
    }
  } elsif ($state == 7) {
    # find ; as first word
    if ($line =~ m/\s*;\s*.*/) {
      if ($DEBUG) { print "DEBUG:: found ; in $line\n"; }
      $state = 0;
      # remove ; from line recheck
      $line =~ s/\s*;\s*//;
      enumParser($line);
    }
  }
}

sub help {
  printf("Usage:  ppeCreateAttrGetSetMacros.pl [--path <pathToFapiHeaders>] [--verbose] [--help] [--src <pathToFapiTemplate>] [--inc <pathToFapiOutput]\n");
  printf("-path <pathToFapiHeaders>     Option to enable specifying alternate path to fapi headers\n");
  printf("-v | -verbose     Inform user of findings and changes\n");
  printf("-h | -help        print this message\n");
  exit(0);
}
