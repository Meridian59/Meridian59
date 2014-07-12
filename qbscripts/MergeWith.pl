#MergeWith
#--destination=branch to merge into
#--source=branch to merge from

#use strict;
use warnings;
use LWP::Simple;
use Getopt::Long;
use Pithub;


my $source;
my $destination;

GetOptions("source=s", => \$source,
           "destination=s", => \$destination);

$output = `git checkout $destination`;

$output = `git merge $source`;