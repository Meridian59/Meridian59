use strict;
use warnings;
use LWP::Simple;
use Getopt::Long;
use Data::Dumper;
use Pithub;

my $m = Pithub::Issues::Milestones->new;
my $result = $m->list(
    repo => 'Meridian59_103',
    user => 'Daenks',
);


while ( my $row = $result->next ) 
{
	printf "%s:%s\n", $row->{number},$row->{title};
}