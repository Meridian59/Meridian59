use strict;
use warnings;
use LWP::Simple;
use Getopt::Long;
use Git::Repository;
use Pithub;

my $localrepo = Git::Repository->new();

#simple commands (leave output as is)
#$localrepo->run( checkout => 'master' );

#complex commands, allows storage of output
#Git::Repository::Command 

my @commandargs = ( "master" );
my $command = Git::Repository::Command->new('checkout',@commandargs);

#Grabs handles to the input and output of the command
my $out = $command->stdout();
my $error = $command->stderr();

print "out:" . <$out>;
print "err:" . <$error>;

$command->close();