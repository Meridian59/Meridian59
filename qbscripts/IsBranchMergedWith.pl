#IsBranchMergedWith.pl
#--destination=branch to merge into
#--source=branch to merge from
#returns 1 if the source branch has been merged into destionation

#use strict;
use warnings;
use LWP::Simple;
use Getopt::Long;
use Pithub;


my $source;
my $destination;

GetOptions("source=s", => \$source,
           "destination=s", => \$destination);

$output = `git checkout $destination 2>&1 1>nul`;
$output = `git branch --merged`;

if ($output =~ m/$source/)
{
	print "1\n";
}
else
{
	print "0\n";
}