#!/usr/local/bin/perl5


# ------------------------------------------------------------------------------
# Global variables
$DFLT_FILE = "build.mak";
$VERSION = "1.0";
$DEBUG = 0;
my $numargs = 0;
my $filecnt = 0;
my $SILENT = 0;
my @filelist;
my %namelist;

# ------------------------------------------------------------------------------
# if the number of command line arguments is > 1 (e.g., the perl file),
# take those arguments and run silently.
while ($ARGV = shift)
{
	$numargs++;
	if ($ARGV =~ /^-[Hh]/)	{&ShowHelp;}	# ShowHelp never returns...
	if ($ARGV =~ /^-[Dd]/)	{$DEBUG = 1;	next; }
	if ($ARGV =~ /^-(.*)/)	{print "?Unknown option <-$1>\n";	&ShowHelp; }
	@filelist[$filecnt++] = $ARGV;
}
goto PROMPT_PROCESSING if ($filecnt == 0);
if ($filecnt != 2)
{
	print "?ERROR: $0 requires 2 name arguments\n";
	&ShowHelp;
}
$SILENT = 1;
$mak = @filelist[0];
-e $mak or die "Can't find file <$mak>: $!\n";
$bgf = @filelist[1];
goto SILENT_START;

# ------------------------------------------------------------------------------
PROMPT_PROCESSING:

print "----------------------------\n";
print "Graphics file update utility\n";
print "Version $VERSION\n";
print "----------------------------\n";

START_OVER:	# Come here when the chips are down

print "Enter the MAK file to update (ENTER=$DFLT_FILE): ";
chop($mak = <STDIN>);
if ($mak eq "")
{
	$mak = $DFLT_FILE;
}
-e $mak or die "Can't find file <$mak>: $!\n";

# Loop until we get a good BGF name...
while (1)
{
	print "Enter the BGF file name to add: ";
	chop($bgf = <STDIN>);
	if ($bgf eq "")
	{
		print "*** You must enter a name.  Press ^C to exit.\n\n";
		next;
	}
	unless ($bgf =~ /[-a-zA-Z_0-9]+\.bgf/)
	{
		print "?Name must end in \".bgf\".  Press ^C to exit.\n\n";
		next;
	}
	unless ($bgf =~ /[-a-zA-Z_0-9]+\.[-a-zA-Z_0-9]+/)
	{
		print "?Improperly formatted name - try again.  Press ^C to exit.\n\n";
		next;
	}
	last;
}
print "The MAK file to be changed is: <$mak>\n";
print "The BGF file to be included is: <$bgf>\n";
while (1)
{
	print "If this is correct enter YES, to exit enter NO: ";
	chop($ans = <STDIN>);
	$ans =~ tr/[a-z]/[A-Z]/;
	if ($ans eq "NO")
	{
		goto START_OVER;
	}
	last if ($ans eq "YES");
	print "?Invalid response <$ans>\n";
}

# ------------------------------------------------------------------------------
SILENT_START:

# Open our temp file...
$bak = $mak;
$bak =~ s/([\-a-zA-Z_0-9]+)\..*/$1/;
$bak .= ".TMP";
open(OUTFILE, ">".$bak) or die "Can't open <$bak> for output: $!\n";

# Open the file to update...
$didsomething = 0;		# Check to be sure we could do the work requested
$inBGFS = 0;			# Set once we found the BGFS line...
open(INFILE, "<".$mak) or die "Can't open <$mak> for input: $!\n";
$inline = 0;
while (<INFILE>)
{

	chop;
	$inline += 1;
	if ($inBGFS == 0)	# TRUE if still looking for BGFS line
	{
		unless ($_ =~ /\s*BGFS.*/)
		{
			print OUTFILE "$_\n";		# We're copying every line to the TMP file
			next;
		}
		$inBGFS = 1;
		unless ($_ =~ /\s*BGFS\s*=\s*\\\s*/)
		{
			die "?Badly formed BGFS line at $inline <$_>\n";
		}
		next;
	}

	# Here when processing inside the BGFS lines
	$line = $_;
	$_ =~ s/\s*(.*)/$1/;	# Strip any leading whitespace
	last if ($_ eq "");		# Blank line terminates BGFS section...
	unless ($_ =~ /[-a-zA-Z_0-9]+\.[-a-zA-Z_0-9]+\s*\\\s*/)
	{
		print "Line #$inline is bad <$_>\n";
	}
	$_ =~ s/([-a-zA-Z_0-9]+\.[-a-zA-Z_0-9]+).*/$1/;	# Extract the name
	$namelist{$_} = 1;
	print "Name <$_> added.\n" if $DEBUG;
	$didsomething = 1;
}

$duplicate = defined($namelist{$bgf});
if ($duplicate)
{
	print "*** ERROR: <$bgf> is already specified in file <$mak>\n";
	$didsomething = 0;
	goto CLOSE_EXIT;
}
$namelist{$bgf} = 1;

# We've fallen out of the loop, rewrite the BGFS lines
print OUTFILE "BGFS = \\\n";
foreach $key (sort keys %namelist)
{
	print OUTFILE "\t$key \\\n";
}
print OUTFILE "\n";

# Finish out the file...
while (<INFILE>)
{
	print OUTFILE "$_";		# We're copying every line to the TMP file
}

CLOSE_EXIT:
close(INFILE);
close(OUTFILE);

# We could have found a duplicate or no BGFS keyword
if ($didsomething == 0)
{
	print "*** WARNING ***\n";
	print "*** File is unchanged: <$mak>.\n";
	print "*** Either the BGFS keyword was not found or the new BGF name was duplicate.\n";
	unlink $bak;
	exit 1;
}

# ------------------------------------------------------------------------------
# Rename the old version filename.Vnn where nn = 1 thru 99
# Name the filename.TMP file to filename.xxx where xxx is the original file's extension
$vers = 1;
$vbase = $mak;
$vbase =~ s/([\-a-zA-Z_0-9]+)\..*/$1/;
while ($vers <= 99)
{
	$vname = $vbase . ".V" . $vers;
	if (-e $vname)
	{
		$vers += 1;
		next;
	}
	rename $mak, $vname;
	rename $bak, $mak;
	print "Addition of <$bgf> to <$mak> complete.\n" unless $SILENT;
	print "Original file now called <$vname>\n" unless $SILENT;
	exit 0;
}
# Get here if too many versions
print "### WARNING: Too many old versions (99) to rename properly.\n";
print "### New file is <$bak>.\n";
print "### Old file is <$mak> (still).\n";


exit 1;

# ------------------------------------------------------------------------------
sub ShowHelp
{
	print "\n";
	print "# $0 [-help] [-debug] makefilename newbgfname\n";
	print "\n";
	print "# $0 - Adds a new BGF file to a graphics MAK file\n";
	print "# -help         Shows this message\n";
	print "# -debug        Turns on debugging messages\n";
	print "# makefilename  Name of the makefile to update\n";
	print "# newbgfname    Name of the BGF file to add to the BGFS list\n";
	print "\n";
	exit 1;
}