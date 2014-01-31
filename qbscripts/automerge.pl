#mergechangedbranches
#create branch from buildnumber
#merge branches from previous script

use strict;
use warnings;
use LWP::Simple;
use Getopt::Long;

my @changelists;
my $build;

GetOptions ("change=s" => \@changelists,
            "build=s" => \$build);

system("git checkout master");

system("git checkout -b $build master");
	
foreach my $changelist (@changelists)
{
	my $RC = system("git merge $build-$changelist");	
	if ($RC != 0)
	{
		die "Merge of branch $build-$changelist failed";
	}
}