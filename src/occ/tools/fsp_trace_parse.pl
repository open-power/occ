#!/usr/bin/perl
#
# This file is used by the trace.simics script to format an OCC trace.  It pulls
# data out of the memory of the simics session, packs it into hex, outputs as a
# file and then runs fsp-trace.
#
# FUTURE IMPROVEMENTS
#   1.
#
#

my $filename = "/tmp/occ_output_trace_data.txt";
my $fsp_trace_loc = "fsp-trace";
my $data;

while (defined($line = <STDIN>))
{    
    chomp $line;

    # Initial format is p:0xfffc30b0  0000 0000 0000 0000 0000 0000 0000 0000  ................
    # and we just want to keep the hex digits
    $line =~ /^p:\S+\s\s+(\S+.*)\s\s/;
    my $hex_string = $1;

    #print "$line  ---> $hex_string\n";
    # Split it up into 2 byte segments
    my @array = split(/\s+/, $hex_string);

    # And pack as hex
    foreach $entry (@array)
    {
	$data .= pack('H4', $entry);
    }
}

# Output toa file
open(FILEHANDLE, ">$filename");
print FILEHANDLE $data;
close(FILEHANDLE);


# Format the trace
open(TRAC, "fsp-trace -s trexStringFile $filename |") || die "Failed: $!\n";
while( <TRAC> )
{
    my $line = $_;
    chomp $line;

    # Find the timestamp.  Example:  0015543.977001
    if($line =~ /^(\d+\.\d+)\|/)
    {
	$num = $1;

	$text = sprintf("%014f", $num / 3.6);
	#print $text . "    ";

	$line =~ s/^(\d+\.\d+)\|/$text\|/;
    }


    if($line =~ /Can't open device 2/)
    {
	# get rid of stupid warning message
    }
    else
    {
	print $line . "\n";
    }
}
close(TRAC);

# And delete the file
#unlink($filename);
