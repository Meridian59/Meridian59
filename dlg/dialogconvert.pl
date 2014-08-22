use strict;
use warnings;

my $infile = @ARGV[0];
my $outfile = 'npcdlg.txt';

open my $info, $infile or die "Could not open $infile: $!";
open my $fh, '>>', $outfile or die "Could not open file '$outfile' $!";

while( my $line = <$info>)  
{
	my @values = split('\t',$line);
	my $command1 = "send o 0 speechlibtrigger who class $values[0]";
	my $command2 = "send o 0 speechlibquote who class $values[0] quote q $values[3]\n";
	#print "0:$values[0]\n1:$values[1]\n2:$values[2]\n3:$values[3]\n----\n";
	if (length $values[1])
	{
		$command1 = $command1 . " mood int $values[1]";
	}
	if (length $values[2])
	{
		$command1 = $command1 . " trigger q $values[2]";
	}
	print $fh "$command1\n$command2";
    #last if $. == 200;
}

close $infile;
close $outfile;