#!/usr/local/bin/perl5
# ------------------------------------------------------------------------------
#	Script:		AddTxtr.pl
#
#	Contains:	PERL script to add a BGF to the textures make file
#
#	Author:		Kevin S. MacDonell
#
#	Date:			June 6, 1997
#
#	Copyright:	© 1997 by The 3DO Company. All rights reserved.
#					This material constitutes confidential and proprietary
#					information of the author and shall not be used by
#					any Person or for any purpose except as expressly
#					authorized in writing by the 3DO Company.
#

# ------------------------------------------------------------------------------
# Global variables
#
$DFLT_FILE = "build.mak";
$VERSION = "1.0";
$DEBUG = 0;
$TRACE = 0;
$SILENT = 0;

# ------------------------------------------------------------------------------
# Hidden from subroutines
#
my $numargs = 0;
my $filecnt = 0;
my @filelist;
my %namelist;
my $mak, $bgf, $desc;

# ------------------------------------------------------------------------------
# Command line argument processing...
# (Ed wanted scripts to prompt or run silently...)
# If no arguments prompt for everything, else run silently.
#
while ($ARGV = shift)
{
	$numargs++;
	if ($ARGV =~ /^-[Hh]/)	{&ShowHelp;}	# ShowHelp never returns...
	if ($ARGV =~ /^-[Dd]/)	{$DEBUG = 1;	next; }
	if ($ARGV =~ /^-[Tt]/)	{$TRACE = 1;	next; }
	if ($ARGV =~ /^-(.*)/)	{print "?Unknown option <-$1>\n";	&ShowHelp; }
	@filelist[$filecnt++] = $ARGV;
}

goto PROMPT_PROCESSING if ($filecnt == 0); # No args passed, prompt for 'em

if ($filecnt != 3) # Check for correct # of args
{
	print "?ERROR: $0 requires 3 name arguments \n";
	print "(HINT: quote the description string)" if $filecnt > 3;
	print "\n";
	&ShowHelp;
}
($mak,$bgf,$desc,@ignore) = @filelist;
-e $mak or die "Can't find file <$mak>: $!\n";	# Insure file exists...

$SILENT = 1;		# Remember that we're running silently...
goto SILENT_START;

# ------------------------------------------------------------------------------
PROMPT_PROCESSING:

print "\n\n----------------------------\n";
print "Texture file update utility\n(the Rob Ellis non-memorial Perl Script)\n";
print "Version $VERSION\n";
print "----------------------------\n\n";

START_OVER:	# Come here when the chips are down

$mak = &PromptForMAK;					# Either we get something good or die...
$bgf = &PromptForBGF;					# Either we get something good or die...
$desc = &PromptForDescription;		# Either we get something good or die...

# ----------
# Use Rob's name overloading to extract the pieces
$realbgf = "grd".substr($bgf,0,5).".bgf";
$bgftype = substr($bgf,5,1);
$bgftype =~ tr/[A-Z]/[a-z]/;
if ($bgftype ne "w" and $bgftype ne "f")
{
        print "?ERROR: BGFType<$bgftype>\nThe only BGF types supported are 'w' and 'f'.\n";
	goto START_OVER;
}
$sflag = substr($bgf,6);

print "The MAK file to be changed is: <$mak>\n";
print "The BGF file to be included is: <$realbgf>\n";
print "The texture type is: <$bgftype>\n";
print "The shrink factor is: <$sflag>\n";
print "The description is: <$desc>\n";

$ans = &PromptForYesOrNo("Is this correct?");	# We get either 1 or 0 or die...
goto START_OVER if ($ans = 0);
#die;

# ------------------------------------------------------------------------------
#	Main processing section...
#
SILENT_START:

# ----------
# Open our temp file...
$bak = $mak;
$bak =~ s/([\-a-zA-Z_0-9]+)\..*/$1/;
$bak .= ".TMP";
open(OUTFILE, ">".$bak) or die "Can't open <$bak> for output: $!\n";

# ----------
# Use Rob's name overloading to extract the pieces
$realbgf = "grd".substr($bgf,0,5).".bgf";
$bgftype = substr($bgf,5,1);
$bgftype =~ tr/[A-Z]/[a-z]/;
$sflag = substr($bgf,6);
if ($bgftype ne "w" and $bgftype ne "f")
{
        print "?ERROR: BGFType<$bgftype>\nThe only BGF types supported are 'w' and 'f'.\n";
        die;
}

# ----------
# Open the file to update...
$didsomething = 0;		# Check to be sure we could do the work requested
$inBGFS = 0;				# Set once we found the BGFS line...
open(INFILE, "<".$mak) or die "Can't open <$mak> for input: $!\n";
$inline = 0;
print STDERR "Starting main loop...\n" if $TRACE;
while (<INFILE>)
{
	die "Early end-of-file encountered (1)...\n" if eof(INFILE);
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
	#print "Name <$_> added.\n" if $DEBUG;
	$didsomething = 1;
}

# ----------
# Out of the loop when we're read all the BGFS items...
print STDERR "Out of loop...\n" if $TRACE;
$duplicate = defined($namelist{$realbgf});
if ($duplicate)
{
	print "*** ERROR: <$realbgf> is already specified in file <$mak>\n";
	$didsomething = 0;
	goto CLOSE_EXIT;
}
$namelist{$realbgf} = 1;

# ----------
# We've fallen out of the loop, rewrite the BGFS lines
print OUTFILE "BGFS = \\\n";
print STDERR "Rewriting BGFS section...\n" if $TRACE;
foreach $key (sort keys %namelist)
{
	print OUTFILE "\t$key \\\n";
}
print OUTFILE "\n";		# the blank line was the last one skipped...

# ----------
# Find the 'all:' line...
print STDERR "Searching for the ALL line...\n" if $TRACE;
while (<INFILE>)
{
	die "Early end-of-file encountered...\n" if eof(INFILE);
	$inline += 1;
	print OUTFILE "$_";			# We're copying every line to the TMP file
	last if ($_ =~ /^all:/);	# Does this line start with 'all:' ?
}

# ----------
# Find the first 'grd#####.bgf:' line...
print STDERR "Searching for the first GRD line...\n" if $TRACE;
while (<INFILE>)
{
	die "Early end-of-file encountered...\n" if eof(INFILE);
	$inline += 1;
	last if ($_ =~ /^grd/);		# Does this line start with 'grd'?
	print OUTFILE "$_";			# We're copying every line to the TMP file
}

# ----------
# Collect up the grd make lines into the grd hash...
print STDERR "Scooping up the GRD lines...\n" if $TRACE;
my %grdh = ();
while (1)
{
	my $grd, $cmd;
	chop;
	unless ($_ =~ /^grd/)
	{
		print "OUT OF SYNC ERROR: <$_>\n";
		die;
	}
	($grd = $_) =~ s/(grd[0-9A-Za-z]+\.bgf).*/$1/;	# Trim any trailing junk
	$cmd = '';
	while (<INFILE>)  # glob the make lines together...
	{
		die "Early end-of-file encountered...\n" if eof(INFILE);
		#print "GLOBBING: <$_>";
		if ($_ =~ /^\s*\n/)	# Extra continuation mark on previous line...
		{
			my $i, $term;
			$i = length($cmd);						# length of the string
			$i = rindex($cmd,"\\", $i);			# Search backward for it.
			$term = substr($cmd,$i+1);				# Hack out the newline (might be 2 chars)... 
			$cmd = substr($cmd,0,$i) . $term;	# Splice it back together again with cont char.
			last;
		}
		$cmd .= $_;
		last unless ($_ =~ /\\\n/);		# Bail unless continuation line
	}
	$grdh{$grd} = $cmd;
	#print "Adding <$grd> <$cmd>\n" if $DEBUG;
	while (<INFILE>)	# skip any intervening blank lines
	{
		die "Early end-of-file encountered...\n" if eof(INFILE);
		last unless ($_ =~ /^\s*\n$/);
	}
	last if ($_ =~ /^clean:/);		# done when we find the clean mark
	#print "looping with <$_>\n";
}

# ----------
# Add the new line into the grd hash...
print "Adding the new GRD line <$realbgf>...\n" if $TRACE;
if (defined($grdh{$realbgf}))
{
	print "*** ERROR: <$realbgf> has a make line in file <$mak>\n";
	$didsomething = 0;
	goto CLOSE_EXIT;
}
$tmp = "\tmakebgf -o \$*.bgf -r -n \"";
$tmp .= $desc;
$tmp .= "\" \-s $sflag 1 \$(SOURCEDIR)\\$bgf.BMP 1 1 1\n";
$grdh{$realbgf} = $tmp;

# ----------
# We've fallen out of the loop, rewrite the makefile lines
print "Rewriting GRD lines...\n" if $TRACE;
$cleanline = $_;
foreach $key (sort keys %grdh)
{
	print OUTFILE "$key:\n";
	print OUTFILE "$grdh{$key}";
	print OUTFILE "\n";				# restore blank...
}
print OUTFILE "$cleanline";		# restore clean line...

# Finish out the file...
while (<INFILE>)
{
	print OUTFILE "$_";		# We're copying every line to the TMP file
}

CLOSE_EXIT:
print "Close_Exit...\n" if $TRACE;
close(INFILE);
close(OUTFILE);

# We could have found a duplicate or no BGFS keyword
if ($didsomething == 0)
{
	print "*** WARNING ***\n";
	print "*** File is unchanged: <$mak>.\n";
	print "*** Either the BGFS keyword was not found or the new BGF name was duplicate.\n";
	unlink $bak;
	exit (1);
}

# ------------------------------------------------------------------------------
# Rename the old version filename.Vnn where nn = 1 thru 99
# Name the filename.TMP file to filename.xxx where xxx is the original file's extension
print "Rename and exit...\n" if $TRACE;
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

# ------------------------------------------------------------------------------
exit (1);					# END OF MAIN LINE OF PROCESSING
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
sub PromptForMAK		# Prompt for the name of the make file to be updated
{
	my $mak;
	while (1)
	{
		print "Enter the MAK file to update (ENTER=$DFLT_FILE): ";
		chop($mak = <STDIN>);	die if ($mak eq "EXIT");
		$mak = $DFLT_FILE if ($mak eq "");
		last if (-e $mak);
		print "Can't find file <$mak>: $! - try again or press ^C to exit.\n";
	}
	return ($mak);
}

# ------------------------------------------------------------------------------
sub PromptForBGF		# Prompt for the BGF file to add into the make file
{
	my $ans;
	while (1)
	{
		print "Enter the BGF code to add (#####X#): ";
		chop($ans = <STDIN>);	die if ($mak eq "EXIT");
		$ans =~ tr/[a-z]/[A-Z]/;
		last if ($ans =~ /^[0-9]{5,5}(W|F)[0-9]$/);
		print "The BGF file names are in the format #####x#,\n";
		print "  where # is a number and x is either the letter'W' or 'F'\n";
	}
	return ($ans);
}

# ------------------------------------------------------------------------------
sub PromptForDescription		# Prompt for the description string
{
	my $ans;
	while (1)
	{
		print "Enter a description of the BGF: ";
		chop($ans = <STDIN>);	die if ($mak eq "EXIT");
		if ($ans eq "")
		{
			$ans = &PromptForYesOrNo("Are you sure you want a blank description?");
			next if ($ans == 0);
			$ans = "";
		}
		last;
	}
	return ($ans);
}

# ------------------------------------------------------------------------------
sub PromptForYesOrNo	# Generic confirm routine - only allows YES and NO
{	# Return 1 for YES and 0 for NO...
	my $ans;
	#my $prompt = @_;
	while (1)
	{
		print "@_ (YES or NO): ";
		chop($ans = <STDIN>);	die if ($mak eq "EXIT");
		last if ($ans eq "YES" || $ans eq "NO");
		print "Please answer YES or NO or press ^C to exit.\n";
	}
	return ($ans eq "YES");
}

# ------------------------------------------------------------------------------
sub ShowHelp
{
	print "\n";
	print "$0 [-help] [-debug] [-trace] makefilename newbgfname description\n";
	print "\n";
	print "# $0 - Adds a new BGF file to a textures MAK file\n";
	print "# -help         Shows this message\n";
	print "# -debug        Turns on debugging messages\n";
	print "# -trace        Trace the major steps being processed\n";
	print "# makefilename  Name of the makefile to update\n";
	print "# newbgfname    Name of the texture BGF file to add\n";
	print "# description   Description of the texture file (must be last and quoted)\n";
	print "\n";
	exit (1);
}
