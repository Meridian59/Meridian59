#CheckHaveBranch.pl

#use strict;
use warnings;
use LWP::Simple;
use Getopt::Long;
use Pithub;

my $branch;

GetOptions("branch=s", => \$branch);

$output = `git checkout $branch 2>&1 1>nul`;
if ($output !~ /did not match any file/)
{
	print "1\n";
}
else
{
	print "0\n";
}