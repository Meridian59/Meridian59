#!/usr/local/bin/perl5
# ------------------------------------------------------------------------------
#	Usage: perl <scriptname> <logfilename> [<acct#> | <ipsubset>]
#  When an account ID is entered, we will show all the IP address used
#  When an IP subset is entered, we will show all the accounts used by that IP
#  NOTE: An IP subset is a full IP address or any of the first sections.
#  For example: 192.33 will find every account using these first 2 in their IP.
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Log lines of note:
# 1) LogUserData/4 got #### ....
#    This line tells us the account number (####) of someone logging on...
# 2) CloseSession/4 logging off ####
#    This line tells us the account number (####) of someone logging off...
# 3) Starting BlakSton Server v1.6 (May  8 1997 13:45:12)
#    This line tells us the server just booted up (i.e., log everyone off)...
# 4) ExitServer terminating server
#    This line tells us the server was asked to shut down...
# 5) Saving game (time stamp 864925200)...
#    Save game successful (time stamp 864925200).
#    These 2 lines tell us that the log (up to that point) was saved OK.
#    We'll use this as a marker for crediting time if a crash happened.
# ------------------------------------------------------------------------------
# Global variables
$VERSION = "1.0";
$DEBUG = 0;

$MAXLINES = 4294967295;
$MAXLINES = 2500 if $DEBUG;

%HoH = ();

# ------------------------------------------------------------------------------
if (@ARGV[0] =~ /^-[Hh]/)
{
	&help_msg;	# never returns...
}
# ------------------------------------------------------------------------------

$LOGFILENAME = @ARGV[0];
if ($LOGFILENAME eq "")
{
	print "Enter the log file to process: ";
	chop($LOGFILENAME = <STDIN>);
	die "No log file specified!\n" if ($LOGFILENAME eq "");
	-e $LOGFILENAME or die "Log file does not exist!\n";
}
$ACCTNUM = @ARGV[1];
$PROMPT = 0;
$BYACCT = !($ACCTNUM =~ /\./);
if ($ACCTNUM eq "")
{
	$PROMPT = 1;
}

# ------------------------------------------------------------------------------
if ($PROMPT)
{
	while (1)
	{
		print "Enter the account ID or IP address section: ";
		chop($ACCTNUM = <STDIN>);
		last if &valid_acct_or_ip($ACCTNUM);
		print "?INVALID: <$ACCTNUM> must be an integer less that 99999 or part of an IP address\n";
	}
}
unless (&valid_acct_or_ip($ACCTNUM))	# This checks command line arg and is not redundant
{
	print "?INVALID: <$ACCTNUM> must be an integer less that 99999 or part of an IP address\n";
	die;
}
print "\n";

if ($DEBUG)
{
	print "\n#-----\n";
	print "PARAMETERS: Log<$LOGFILENAME> ";
	if ($BYACCT) { print "acct<$ACCTNUM> "; } else { print "ip<$ACCTNUM>"; }
	print "\n#-----\n\n";
}

# ------------------------------------------------------------------------------
open(INFILE, "<".$LOGFILENAME) or die "Can't open the file <$LOGFILENAME> for input: $!\n";
if ($BYACCT)
{
	&process_users($ACCTNUM);
	print "Account <$ACCTNUM> has logged in under the following IP addresses:\n";
	printf "\t%-15s\t%s\n", "IP Address", "Logins";
	printf "\t%-15s\t%s\n", "---------------", "------";
	foreach $key (sort keys %HoH)
	{
		printf "\t%-15s\t%6d\n", $key, $HoH{$key};
	}
}
else
{
	&process_ips($ACCTNUM);
	print "The Accounts for IPs starting with <$ACCTNUM> are:\n";
	foreach $key (sort keys %HoH)
	{
		print "\t$key:\n";
		foreach $k2 (sort keys %{ $HoH{$key} })
		{
			print "\t\t$k2\t$HoH{$key}{$k2}\n";
		}
	}
}
close(INFILE);

exit 0;

# ------------------------------------------------------------------------------
sub process_users
{
	my $infile_lines = 0;
	my $ACCTNUM = @_[0];
	while (<INFILE>)
	{
		my $mo, $da, $year, $hms, $ignore, $tok, @rest;
		$infile_lines += 1;
		last if ($infile_lines > $MAXLINES);
		chop;
		next unless ($_ =~ /LogUserData\/4 got $ACCTNUM/);
		($mo, $da, $year, $hms, $ignore, $tok, @rest) = split(" ");
		$user = @rest[1];
		next unless ($user == $ACCTNUM);
		print "Accepting line <$_>\n" if $DEBUG;
		$ip = @rest[3];
		$ip =~ tr/,//d;
		if (defined($HoH{$ip}))
		{
			$HoH{$ip} += 1;
		}
		else
		{
			$HoH{$ip} = 1;
		}
	}
}

# ------------------------------------------------------------------------------
sub process_ips
{
	my $DEBUG_IPS = 0;
	my $infile_lines = 0;
	my $ACCTNUM = @_[0];
	while (<INFILE>)
	{
		my $mo, $da, $year, $hms, $ignore, $tok, @rest;
		$infile_lines += 1;
		last if ($infile_lines > $MAXLINES);
		chop;
		next unless ($_ =~ /LogUserData\/4 got [0-9]+ from $ACCTNUM/);
		($mo, $da, $year, $hms, $ignore, $tok, @rest) = split(" ");
		$ip = @rest[3];
		$ip =~ tr/,//d;
		print "Comparing $ip to $ACCTNUM at BOL\n" if ($DEBUG_IPS);
		next unless ($ip =~ /^\Q$ACCTNUM/);
		$user = @rest[1];
		if (defined($HoH{$ip}{$user}))
		{
			$HoH{$ip}{$user} += 1;
		}
		else
		{
			$HoH{$ip}{$user} = 1;
		}
	}
}

# ------------------------------------------------------------------------------
sub valid_acct_or_ip
{
	my $DEBUG_VALID_ACCT = 0;
	$chk = @_[0];
	print "#valid_acct_or_ip: checking <$chk>...\n" if $DEBUG_VALID_ACCT;
	if ($chk =~ /\./)
	{
		my @numlist = split('\.', $chk);
		foreach $val (@numlist)
		{
			print "chking part <$val>\n" if $DEBUG_VALID_ACCT;
			return 0 unless ($val =~ /[0-9]+/);
			return 0 if ($val < 0 || $val > 255);
		}
		return 1;
	}
	elsif ($chk =~ /[0-9]+/)
	{
		return ($chk < 99999);
	}
	return 0;
}

# ------------------------------------------------------------------------------
sub help_msg
{
	print "$0\t# Log search function\n";
	print "$0 [logfilename [acct | ip]]\n";
	print "Searches the given log file for all the IPs used by a specific account\n";
	print "or searches for all the accounts used by some IP address or IP prefix.\n";
	print "Example:\n";
	print "\tAn IP prefix of '192.33.2.' will find 192.33.2.87 and 192.33.2.255\n";
	print "\tAn IP prefix of '192.33.2' will find 192.33.2.87 and 192.33.23.87\n";
	print "\tAn IP prefix of '192.' will find all IP address beginning with 192.\n";
	exit 0;
}
