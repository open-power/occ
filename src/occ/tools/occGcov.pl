#!/usr/bin/perl


use strict;
use Getopt::Long;

use lib "$ENV{sb}/occc/405/occ/tools";
use Gcov;

my $dumpfile = "$ENV{sb}/../simics/occ_gcov.bin";
my $symfile = "$ENV{sb}/../obj/ppc/occc/405/occ/occ.syms";

GetOptions( "file:s" => \$dumpfile,
            "symfile:s" => \$symfile);

Gcov::main($dumpfile, $symfile);



