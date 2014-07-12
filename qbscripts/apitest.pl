#use strict;
use warnings;
use LWP::Simple;
use Getopt::Long;
use Data::Dumper::Names;
use Pithub;
use Data::Dumper;

my $git = Pithub::GitData->new(
    repo  => 'Meridian59_103',
    token => '27cd8d4bdcfabc6a330b2275761a38c92b194848',
    user  => 'Daenks',
);

#my $p = Pithub::PullRequests->new;

my $p = $git->
my $pullinfo = $p->get(user => 'Daenks', repo => 'Meridian59_103', pull_request_id => 352);
printf "Pull Request Number: %d Clone URL: %s Branch: %s Author: %s",$pullinfo->content->{number}, $pullinfo->content->{head}->{repo}->{clone_url}, $pullinfo->content->{head}->{ref}, $pullinfo->content->{head}->{user}->{login};