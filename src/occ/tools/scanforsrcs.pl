#!/usr/bin/perl

# This is a utility to generate occSrcParse.C automatically.
# It will searching backing build if file is not present in the sandbox.

use strict;
use File::Find ();
use Time::localtime;

#use re 'debug';
my $VERSION = 1.5;


sub usage() {
    print "Usage: $0 < -b base > <-d> < -o output dir > < -w Word N > \"location of fips_comp_id.H\"\n";
    print "\n";
    print "-b:     base directory ( default is pwd )\n";
    print "-d:     Allow duplicates - default is to ignore\n";
    print "-o:     Used as the output directory where plugins are dropped\n";
    print "        Default is PWD\n";
    print "-w:     Use word N in generating unique SRCs ( N is between 1 and 4 )\n";
    print "-e:     Word N is errno\n";
    print "-n:     Don't display tag lines equal to NA or N/A\n";
    print "-V      Display version.\n";
    print "-h      Display version.\n";
    print "\n\n";
    exit 1;
}

# Locals
my $base = $ENV{PWD};
my $baseBB = $ENV{PWD};
my $output = $base;
my $arg;
my %Comps;
my $ignore = 1;
my $fips_comp_id = "$ENV{bs}/fstp/fsp/fips_comp_id.H";
#occ/incl/comp_ids.h
my $wordn = "userdata";
my $wordnd = 0;
my $Errno = 0;
my $NA = 1;

while ($ARGV = shift) {
    if ( $ARGV =~ m/-b/ ) {
	$base = shift;
    }
    elsif ( $ARGV =~ m/-d/i ) {
	$ignore = 0;
    }
    elsif ( $ARGV =~ m/-o/i ) {
	$output = shift;
    }
    elsif ( $ARGV =~ m/-w/i ) {
	$wordnd = shift;
	$wordn .= $wordnd;
    }
    elsif ( $ARGV =~ m/-e/i ) {
	$Errno = shift;
    }
    elsif ( $ARGV =~ m/-n/i ) {
	$NA = 0;
    }
    elsif ( $ARGV =~ m/-h/i ) {
	usage();
    }
    elsif ( $ARGV =~ m/-V/ ) {
	print "$0 version = $VERSION\n\n";
	exit 0;
    }
    else {
	$fips_comp_id = $ARGV . "/$fips_comp_id";
    }
}

# Hardcode just the values that occ needs
$base="$ENV{SANDBOXBASE}/src/occc/405/";
#$baseBB="$ENV{bb}/src/occc/405/";
#$baseBB="/afs/austin.ibm.com/projects/esw/occ810/Builds/occ810_130926a/src/occc/405/";
$baseBB="$ENV{BACKED_SANDBOXDIR}";
my @values = split(':', $baseBB);
foreach my $val (@values) 
{
    if ( $val =~ m/occ810/)
    {
        $baseBB=$val;
    }
 }

#we should not input -d option, let $ignore = 1;  #so compile won't fail if there are dups
$output=$base; # put output file in base directory  
$wordnd = 4;  #look at userdata4
$wordn .= $wordnd;
#$NA = 0;
my $IgnoreDir = "ssx";


FindFiles();

my $cntr;

foreach my $key ( sort keys %Comps )
{
    DumpPlugin( $key, \@{$Comps{$key}} );   
}


#DumpMakefile();

print " +++ $0 is DONE. \n";

exit;

sub FindFiles
{
    # Array which will have the final set of files to scan
    my @FilteredArray;

    # Array holding Base Sandbox files
    my $BaseFiles=`find $base -name $IgnoreDir -prune -type f -name \'*.h\' -or -name \'*.c\'`;
    my @BaseFileArray= split("\n", $BaseFiles);
    #foreach my $val (@BaseFileArray) 
    #{
    #    print "SandboxFiles:: $val \n";
    #}
    # Array holding the Backing Build files
    my $BBFiles=`find $baseBB -name $IgnoreDir -prune -type f -name \'*.h\' -or -name \'*.c\'`;
    my @BBFileArray= split("\n", $BBFiles);
    #foreach my $val (@BBFileArray) 
    #{
    #    print "BackingBuildFiles:: $val \n";
    #}

    # find all build files from makefile and remove unnecessary files from src scan list
    my @BuildList = FindBuildList();
    #foreach my $val (@BuildList) 
    #{
    #    print "BuildList:: $val \n";
    #}


    CheckBuildList(\@BBFileArray, @BuildList);
    CheckBuildList(\@BaseFileArray, @BuildList);

    # Start with Sandbox Files
    @FilteredArray=@BaseFileArray;   

    # Cycle through all Backing Build Files 
    for my $BBFileArrayCntr(@BBFileArray)
    {
        # found flag
        my $found = 0;

        # Cycle through all Sandbox Files
        for my $BaseFileArrayCntr(@BaseFileArray)
        {

            # strip off everything an get current Backing Build File
            my $tmp1 = rindex($BBFileArrayCntr, "/") + 1;
            my $BBFileName = substr($BBFileArrayCntr, $tmp1);
            
            # strip off everyting and get current Sandbox File
            my $tmp2 = rindex($BaseFileArrayCntr, "/") + 1;
            my $FileName = substr($BaseFileArrayCntr, $tmp2);
        
            # if Backing Build File name is same is SandBox file name
            # then we are done checking this Backing Build file, exit!
            if ( "$BBFileName" eq "$FileName" )
            {    
                $found = 1;
                last;
            }
        }

        # if we didnt find any matches then it means that the
        # backing build file name should be used
        if ( $found eq 0 )
        {   
            # add to final array 
            @FilteredArray=(@FilteredArray,$BBFileArrayCntr);
        }
    }

    # sort final arry based on ascii of strings
    @FilteredArray = sort( @FilteredArray );   
 
    # now called wanted to extract SRCs 
    foreach my $FilteredArrayCntr(@FilteredArray)
    {
        # extract srcs from this file
        &wanted( $FilteredArrayCntr );    
    }
}

# Find build lists from makefile
sub FindBuildList
{
    # Array holding all makefiles
    my @MakefileArray;

    # Array holding makefiles in Base Sandbox
    my $BaseMakefile = `find $base -type f -name *.mk -or -name "Makefile*"`;
    my @BaseMakefileArray = split("\n", $BaseMakefile);
    
    # Array holding makefiles in Backing Build
    my $BBMakefile = `find $baseBB -type f -name *.mk -or -name "Makefile*"`;
    my @BBMakefileArray = split("\n", $BBMakefile);

    # Use makefiles in sandbox as the base
    @MakefileArray = @BaseMakefileArray;

    # Cycle through all Backing Build Files 
    for my $BBMakefileArrayCntr(@BBMakefileArray)
    {
        # found flag
        my $found = 0;

        # Cycle through all Sandbox Files
        for my $BaseMakefileArrayCntr(@BaseMakefileArray)
        {
            # strip off everything and get current Backing Build File
            my $tmp1 = rindex($BBMakefileArrayCntr, "/src/occc/405") + 1;
            my $BBMakefileName = substr($BBMakefileArrayCntr, $tmp1);
            
            # strip off everyting and get current Sandbox File
            my $tmp2 = rindex($BaseMakefileArrayCntr, "/src/occc/405") + 1;
            my $FileName = substr($BaseMakefileArrayCntr, $tmp2);
        
            # To check if the makefiles is only found in Backing Build
            if ("$BBMakefileName" eq "$FileName")
            {    
                $found = 1;
                last;
            }
        }

        # if we didnt find any matches, use makefile from backing build 
        if ($found == 0)
        {   # add to final array 
            @MakefileArray = (@MakefileArray,$BBMakefileArrayCntr);
        }
    }

    # sort final array based on ascii of strings
    @MakefileArray = sort( @MakefileArray );

    # loop for all makefiles to collect build files
    my @TempArray;
    foreach my $makefileCnt (@MakefileArray)
    {   
        #will display which makefile we cross reference against
        #print "Using $makefileCnt\n";
        my $var;
        {
            local $/;
            open my $fh, '<', $makefileCnt or die "can't open $makefileCnt: $!";
            $var = <$fh>;
            close $fh;
        }
        $var =~ s/#.*//g; # remove comments
#        print "$var \n";
        while ($var =~ m/(\w+)\.o/g) 
        {   # Add to build list
            @TempArray = (@TempArray,$1);
        }
        
        while ($var =~ m/(\w+)\.c/g)
        {   # Add to build list
            @TempArray = (@TempArray,$1);
        }
    }

    # remove duplicate elements
    my %hash = map { $_, 1 } @TempArray;
    my @BuildArray = keys %hash;
    # sort final arry based on ascii of strings
    @BuildArray = sort(@BuildArray);
    
 #   foreach my $val (@BuildArray) 
 #   {
  #      print "$val \n";
  #  }

    return @BuildArray
}

# Remove files which are not in the build list
sub CheckBuildList
{
    my ($FileArray, @BuildList) = @_;

    my @tempArray; # since we cannot delete elements in array in loop

    foreach my $ArrayCnt(@$FileArray)
    {   # strip off everything and get filename
        my $pos = rindex($ArrayCnt, "/") + 1;
        my $File = substr($ArrayCnt, $pos, -2);
        
        if ( $ArrayCnt =~ /\/lib\// )
        {
            #print "LIB file found! $ArrayCnt \n";
            next;
        }
        
        if ( $ArrayCnt =~ /\/ssx\// )
        {
            #print "SSX file found! $ArrayCnt \n";
            next;
        }
    
        #print "Found $File.c\n";

        my $found = 0;
        foreach my $Buildfile(@BuildList)
        {
            if ($Buildfile =~ /$File/)
            {  
                # found filename in makefile, add to scan SRC
                
                # will give you the full path of file we
                # adding to our scan list
                # print "Adding $ArrayCnt to src scan list. \n";

                push @tempArray, $ArrayCnt;
                $found = 1;
                last;
            }
        }
        
        if ($found == 0)
        {   # not found, ignore this file
            print "Ignoring $ArrayCnt. \n";
        }
    }
    
    @$FileArray = @tempArray;
}

sub FileFind
{
    # Set the variable $File::Find::dont_use_nlink if you're using AFS,
    # since AFS cheats.

    # for the convenience of &wanted calls, including -eval statements:
    use vars qw/*name *dir *prune/;
    *name   = *File::Find::name;
    *dir    = *File::Find::dir;
    *prune  = *File::Find::prune;

    # Traverse desired filesystems
    File::Find::find({wanted => \&wanted}, $base);

}


sub wanted {
    #my ($dev,$ino,$mode,$nlink,$uid,$gid);
    my $data;
    my $file = "@_";
   
    print "Processing: $file\n";

    open FH, $file or die "Cannot open: $file";
    read FH, $data, -s FH or die "Error reading $file: $!";
    close FH;

    # extract filename from full path
    my $pos = rindex($file, "/") + 1;
    my $fileName = substr($file, $pos);
   
    # extract component 
#    $fileName =~ /^(\w{4})/;
#    my $comp = $1;
    # No matter filename has prefix "hwsv"(applet) or "tmgt"(all other tmgt code), put all function in one file which is "occSrcParse.C", 
    my $comp = 'occc';

    while ( $data =~ /\@errortype(?:.*?)\n((.+?\n)+?).+?\*\//mgo )
    {
        my $text = $1;
        my $found = 0;
        my %hash = $text =~ /\@(\w+)(?:\s+|\.+)\b(.+?)$/gm;
        my $tmp;

        foreach my $key (sort keys %hash)
        { 
            # Extract multi-line string
            $text =~ /\@$key(?:\s+|\.+)(.+?)(?:\@|$).*/s;
            $tmp = $1;
            
            # take off comment starting with "//"
            $tmp =~ s/\s*\/\/.*//gm;
            
            # replace "\*" with "\n"
            $tmp =~ s/\s*\*\s*/ \n/gm;

            chomp($tmp);
            $hash{$key} = $tmp;
        }
    
        #add on file info
        $hash{file} = $file;
    
#        print "------------------------>\n";
#        print "$text\n\n";
#        foreach my $key (sort keys %hash)
#        {
#            print "\t$key => $hash{$key}\n";
#        }
#        print "<------------------------\n";

        # Remove trailing words
        $hash{moduleid} =~ s/^(\w+).*$/$1/;
        $hash{reasoncode} =~ s/^(\w+).*$/$1/;

        if ( $ignore )
        {
            my $res;
            my @RedundantComps;
            
            # Find duplicates
            foreach my $href ( @{ $Comps{$comp} } )
            {
                $res = hasheq( 3, $href, \%hash);
                $found |= $res;
                
                # showing up duplicated files
                if ($res) 
                {
                    push @RedundantComps, $href;
                }
            }

            if ($found) {
                foreach my $href ( @RedundantComps )
                {
                    print "!!!! Also found the same combination in file => $href->{file}\n";
                }

                die "ERROR: $hash{moduleid}/$hash{reasoncode}/$hash{$wordn} combination used more than once in file $fileName!\n";
            }
        }
        
        if ( ! $found )
        {
            push @{ $Comps{$comp} }, \%hash;
        }
    }
}


sub hasheq
{
    my ($cmpnum, $ha1, $ha2) = @_;
    my $h1w2 = "";
    my $h2w2 = "";

    $h1w2 = $1 if $ha1->{$wordn} =~ /^(\w+)/;
    $h2w2 = $1 if $ha2->{$wordn} =~ /^(\w+)/;

    if ( $ha1->{moduleid} eq $ha2->{moduleid} && 
         $ha1->{reasoncode} eq $ha2->{reasoncode} )
    {
        if ($cmpnum eq 2)
        {
            return 1;
        }
        # compare one more parm (userdataX)
        elsif ($cmpnum eq 3 && 
               $h1w2 eq $h2w2 )
        {
            return 1;
        }
        else
        {    
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

sub TakeOffRedundantFunc
{
    my ( $aref ) = @_;
    
    my @nonRedundantComps;
    my $sameflag = 0;
    my $i;
    my $j;
    
    #take off redundent componets 
    for($j = 0; $j < @$aref; $j++)
    {
        for($i = ($j + 1); $i < @$aref; $i++)
        {
            if ( hasheq( 2, @$aref[$j], @$aref[$i]) )
            {
                $sameflag = 1;
                last;
            }
        }
        
        if ($sameflag == 0)
        {
            push @nonRedundantComps, @$aref[$j];
        }
        $sameflag = 0;
    }
    
    return @nonRedundantComps;
}


sub DumpPlugin
{
    my ( $compName, $aref ) = @_;
    my $filename = "$output/$compName" . "SrcParse.C";
    my $filename2 = "$output/$compName" . "SrcParse.H";
    
    my $fh;
    my $fh2;

    #if the plugins subdir doesn't exist, create it
    if (!-e $output) 
    {
        print "Creating directory $output\n";
        `mkdir -p $output`;
    }

    `rm -f $filename`;
    `rm -f $filename2`;

    open $fh, ">$filename" or die "Cannot open $filename: $!\n";
    open $fh2, ">$filename2" or die "Cannot open $filename2: $!\n";

    DumpHeader( $compName, $fh, $fh2 );

    DumpPrototypes( $fh, $aref );

    DumpTable( $fh, $compName, $aref );

    DumpBody( $fh );

    DumpParsers( $fh, $aref );

    close $fh or die "Cannot close $filename: $!\n";
    close $fh2 or die "Cannot close $filename: $!\n";
}




sub DumpHeader
{
    my ( $compName, $fh, $fh2 ) = @_;
    my $schdr = $compName . "SrcParse.H";
    my $struct = $compName . "SrcParser";
    my $timestamp = ctime();

    #lets look for the header includes we will need!
    my $compIdFiles=`find $base -type f -name \"comp_ids.h\"`;
    my @compIdFilesArray= split("\n", $compIdFiles);


    my $headerString = "// includes from occ code base \n// $base \n";
    foreach my $val (@compIdFilesArray) 
    {
        #print "IncludeFiles:: $val \n";
        
        open ( my $l_fh, $val ) or die "Could not open $val";
    
        my $filedata = "";
        my $start=0;
        while (my $row = <$l_fh>)
        {
            chomp $row;
            if ( $row =~ /#ifndef/)
            {
                $start = 1;
            }
            
            if ($start == 1)
            {
                if ( $row =~ /#include/)
                {
                    #ignore the line
                }
                else    
                {
                    $filedata .= "$row \n";
                }
            }

            if ( $row =~ /#endif/)
            {
                $start = 0;
            }
            
        }
        
        #print "$filedata \n";
        close ($l_fh); 

        # strip off everyting and get current Sandbox File
        #my $tmp2 = rindex($val, "/") + 1;
        #my $FileName = substr($val, $tmp2);

        #$headerString .= "#include <$FileName> \n";
        $headerString .= "$filedata \n";
    }


    #lets look for the header includes we will need!
    my $IncludeFiles=`find $base -type f -name \"*_service_codes.h\"`;
    my @IncludeFilesArray= split("\n", $IncludeFiles);

    foreach my $val (@IncludeFilesArray) 
    {
        #print "IncludeFiles:: $val \n";
        
        open ( my $l_fh, $val ) or die "Could not open $val";
    
        my $filedata = "";
        my $start=0;
        while (my $row = <$l_fh>)
        {
            chomp $row;
            if ( $row =~ /#ifndef/)
            {
                $start = 1;
            }
            
            if ($start == 1)
            {
                if ( $row =~ /#include/)
                {
                    #ignore the line
                }
                else    
                {
                    $filedata .= "$row \n";
                }
            }

            if ( $row =~ /#endif/)
            {
                $start = 0;
            }
            
        }
        
        #print "$filedata \n";
        close ($l_fh); 

        # strip off everyting and get current Sandbox File
        #my $tmp2 = rindex($val, "/") + 1;
        #my $FileName = substr($val, $tmp2);

        #$headerString .= "#include <$FileName> \n";
        $headerString .= "$filedata \n";
    }

    
    #print "$headerString ";

    my $filehdr = "

/*
 * Automatically generated by $0
 *
 * TimeStamp: $timestamp
 *
*/

/*****************************************************************************/
// I n c l u d e s
/*****************************************************************************/
#include <cstring>
#include <errno.h>

#include <errlplugins.H>
#include <errlusrparser.H>
#include <srcisrc.H>

#include <$schdr>

typedef void (*parser_t) ( ErrlUsrParser & i_parser, const SrciSrc & i_src );


// Returns specified userdata word (1-4) for supplied SRC
uint32_t get_usrword(const SrciSrc & i_src, const uint8_t i_userdata_num)
{
    const uint32_t  *l_wordarray = NULL;
    uint32_t        l_wordcount = 0;
    uint32_t        l_word = 0;
    uint16_t        l_index = i_userdata_num + 3; // userdata1 is at array index 4, ...

    // Access user words 2-9 and get count
    l_wordarray = i_src.getHexData(l_wordcount);
    if ( l_index < l_wordcount)
    {
        l_word = l_wordarray[l_index];
    }

    return l_word;
}


/*****************************************************************************/
// Forward
/*****************************************************************************/
static bool SrcDataParse(
    ErrlUsrParser &	i_parser,
    const SrciSrc &	i_src
);


/*****************************************************************************/
// User types
/*****************************************************************************/
struct $struct
{
    uint32_t	iv_uid;
    parser_t	iv_parser;
};


/*****************************************************************************/
// Prototypes
/*****************************************************************************/
";

    print $fh  $filehdr;
    print $fh2 $headerString;
}


sub DumpPrototypes
{
    my ( $fh, $aref ) = @_;
    my $forward;

    # get nonRedundant functions to avoid declare same prototype for miltiple times.
    my @nonRedundant = TakeOffRedundantFunc($aref);
    
    foreach my $href ( @nonRedundant )
    {
        $forward = "void Parse_$href->{moduleid}_$href->{reasoncode}( ErrlUsrParser & i_parser, const SrciSrc & i_src );\n";
        print $fh $forward;
    }

    print $fh "\n\n";

}


sub DumpTable
{
    my ( $fh, $compName, $aref ) = @_;
    my $structname = $compName . "SrcParser";
    my $compId = ( uc $compName ) . "_COMP_ID";
    my $header = "
/*****************************************************************************/
// Globals
/*****************************************************************************/
static errl::SrcPlugin	g_SrcPlugin( $compId, SrcDataParse );

static $structname	g_ParserList[] =
{
";

    my $footer =
"    { 0, 0 }
};";

    my $forward;

    print $fh $header;

    # get nonRedundant functions to avoid declare same function for miltiple times.
    my @nonRedundant= TakeOffRedundantFunc($aref);
    
    foreach my $href ( @nonRedundant )
    {
        $forward = "    { ( ( $href->{moduleid} << 16 ) | ( OCCC_COMP_ID | $href->{reasoncode}) ), Parse_$href->{moduleid}_$href->{reasoncode} },\n";
        print $fh $forward;
    }
    
    print $fh "$footer\n\n";

}

sub DumpBody
{
    my ( $fh ) = @_;
    print $fh "
/*****************************************************************************/
// Main Parser
/*****************************************************************************/
bool SrcDataParse(
    ErrlUsrParser &	i_parser,
    const SrciSrc &	i_src
)
{
    // Locals
    int         l_cntr = 0;
    uint32_t    l_uid = 0;
    bool        l_rc = false;";

    
    if ( $Errno > 0 )  # || $wordnd > 0
    {
	print $fh "
    const uint32_t *l_wa = 0;
    uint32_t	    l_wc = 0;

    // Access user words
    l_wa = i_src.getHexData(l_wc);";

    }

	print $fh "

    // Form Unique Id
    uint8_t l_w4 = (uint8_t) ( ( get_usrword(i_src,4) & 0xFF000000) >> 24);    //extract the component id from word 4
    l_uid = (l_w4 << 24) | ( i_src.moduleId() << 16 ) | i_src.reasonCode();

    //i_parser.PrintNumber( \"original l_uid: \", \"0x%08X\", l_uid );

    //i_parser.PrintNumber( \"new gList value\", \"0x%08X\", ( (AMEC_MST_CHECK_UNDER_PCAP & 0x00FF) << 16 ) | (OCCC_COMP_ID|INTERNAL_FAILURE) );


    ";


    if ( $Errno > 0 )
    {
	print $fh "

    // Set errno
    errno = l_wa[$Errno+3];";

    }

    print $fh "

    // Find unique parser and transfer control
    while ( g_ParserList[l_cntr].iv_uid )
    {
        if ( g_ParserList[l_cntr].iv_uid == l_uid )
        {
            g_ParserList[l_cntr].iv_parser( i_parser, i_src );
            l_rc = true;
            break;
        }
        l_cntr++;
    }

    return l_rc;
}


";


}


sub DumpParsers
{

    my ( $fh, $aref ) = @_;
    my $FuncName;

    foreach my $href ( @$aref )
    {
        my $mid = $href->{moduleid};
        my $erc = $href->{reasoncode};
    
	    $FuncName = "Parse_$mid\_$erc";

        my $header = "
/*****************************************************************************/
// $mid, $erc
/*****************************************************************************/
void $FuncName( ErrlUsrParser & i_parser, const SrciSrc & i_src )
{
";
        print $fh $header;

        my @sameComps;
        my @othersComps;
        my $other;
        #check if we have the same modid and reason code, but different data4. 
        while (@$aref != "")
        {
            $other = pop (@$aref);
            if ( ($href != $other) &&
                 hasheq( 2, $href, $other) )
            {
                push @sameComps, $other;
            }
            else
            {
                push @othersComps, $other;
            }
        }
        @sameComps = reverse(@sameComps);
        @othersComps = reverse(@othersComps);
        @$aref = @othersComps;
        
#        print "I am $href, othersComps = @othersComps\n";
#        print "Current Array: @$aref\n";  
#        printf "--------------------\n";       

        #add itself into array
        push @sameComps, $href;
        
        my %hashtemp;
        #if more than one errl has same moduleID/reason, sort errl by userdata4.
        if (@sameComps > 1)
        {
            my @tempComps;
            my @Result;
            my $temp;
            my $wdnString;

            while (@sameComps)
            {
                $temp = pop (@sameComps);
                %hashtemp = %{ $temp };
                
                # if userdata4 filed is not null, put it to the front of result array. 
                if ($hashtemp{$wordn} ne "")
                {
                    # check if userdata4 still has string
                    $wdnString = $hashtemp{$wordn} =~ /[:alpha:]/;
                    if ($wdnString)
                    {
                        print "WARNING: userdata4 can only be number!!! Current value =\"$hashtemp{$wordn}\", file : $hashtemp{file}\n";
                    }

                    push @Result, $temp;
                }
                else
                {
                    push @tempComps, $temp;
                }
            }
            
            # push remain elements(userdata4 == null) into end of result array
            while (@tempComps)
            {
                $temp = pop (@tempComps);
                push @Result, $temp;
            }
            # write Result back
            @sameComps = @Result;            
        }

        my $i;
        my $errl;
        # create code for Errl combination
        for($i = 0; $i < scalar(@sameComps); $i++)
        {
            $errl = @sameComps[$i];
            %hashtemp = %{ $errl };
            
            #print if/esle case
            if (scalar(@sameComps) > 1)
            {
                if ( $i eq 0 )
                {
                    print $fh "
    uint32_t l_word = (get_usrword(i_src,4) & 0x00FFFFFF)\;
    if (l_word == $hashtemp{$wordn})
    {\n";
                }
                elsif ($i eq (@sameComps - 1))
                {
                    print $fh "
    else
    {\n";
                }
                else
                {            
                    print $fh "
    else if (l_word == $hashtemp{$wordn})
    {\n";
                }
            }

            #print code line by line for errl detail.
            foreach my $tag ( sort keys %$errl )
            {
                $errl->{$tag} =~ s/\"/\\\"/g;
                
                # Do not print out file information which is not require by LOG
                if ( $tag =~ /file/ )
                {
                    next;
                }
                
                my @lines = split /^/, $errl->{$tag};
                
                # if we have if/else cases, shift 4 space for coding standard.
                if (scalar(@sameComps) > 1)
                {
                    print $fh "    ";
                }
            
                if ( scalar(@lines) > 1 )
                {
                    print $fh "        i_parser.PrintString(\"$tag\",\n";
                    foreach my $line ( @lines )
                    {
                        $line =~ s/\n//g;
                        print $fh "    \"$line\"\n";
                    }
                    print $fh "    );\n";
                }
                else
                {
                    my $line = $errl->{$tag};
                    $line =~ s/\n//g;
            
                    if ( $tag =~ /$Errno/ )
                    {
                        print $fh "    i_parser.PrintString(\"errno\", strerror(errno));\n";
                    }
                    elsif ( $NA == 1 || ! ( $line =~ /^N\/??A/ ) )
                    {
                        print $fh "    i_parser.PrintString(\"$tag\", \"$line\");\n";
                    }
                }
        
            }
            
            if ( scalar(@sameComps) > 1)
            {
                print $fh "    }"
            }
        }
        print $fh "\n}\n\n\n";
    }

}

sub DumpMakefile
{
    my $filename = "$output/makefile";
    my $fh;
    my $fcid; # Fips Component Ids
    my %comphash;

    if ( ! open $fcid, "<$fips_comp_id" )
    {
	print "Cannot open $fips_comp_id: $! - makefile will NOT be created\n";
	return;
    }
    else
    {
	my $data;
	read $fcid, $data, -s $fcid or die "Error reading $fips_comp_id: $!";
	close $fcid;
	%comphash = $data =~ /(\w{4})_COMP_ID.*?=.*?0x(\w{4})/mg;
	#foreach my $key ( sort keys %comphash )
	#{
	#    print "MAP: $key->$comphash{$key}\n";
	#}
    }
    if ( ! open $fh, ">$filename" )
    {
	print "Cannot open $filename: $! - makefile will NOT be created\n";
	return;
    }

    my @keys = keys %Comps;
    my %libnames;

    print $fh "\n";
    foreach my $key ( @keys )
    {
	my $uKey = $key;
	$uKey =~ tr/a-z/A-Z/;
	$libnames{$key} = "libE-" . $comphash{$uKey} . ".so";
	my $srcfile = $key . "SrcParse.o\n";
	print $fh "$libnames{$key}_OFILES = $srcfile";
	#$libnames{$key} = $libnames{$key} . ".so";
    }
    print $fh "\n";

    print $fh "INCFLAGS\t+= ";
    foreach my $key ( @keys )
    {
	print $fh "-I../$key/fsp ";
    }
    print $fh "\n\n";

    print $fh "SHARED_LIBRARIES\t=\t\t\\\n";
    foreach my $key ( @keys )
    {
	print $fh "\t$libnames{$key} \t\\\n";
    }
    print $fh "\n\n";

    print $fh ".if ( \$(CONTEXT) != \"x86\" )\n";
    foreach my $key ( @keys )
    {
	print $fh "$libnames{$key}_IDIR = /fs/nfs.fips/lib/\n";
    }
    print $fh ".endif\n\n";

    print $fh "SHIPLIST = \$\{ILIST\}\n\n";

    print $fh ".include<\$\{RULES_MK\}>\n\n";

    close $fh or die "Cannot close $filename: $!\n";
}
