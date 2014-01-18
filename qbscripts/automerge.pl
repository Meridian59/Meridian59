use strict;
use warnings;
use LWP::Simple;
use Getopt::Long;

my $outdir="C:/patchfiles";
#my $changelist=124;
my @changelists;
my $build;

GetOptions ("change=s" => \@changelists,
            "build=s" => \$build);

foreach my $changelist (@changelists)
{
	my $baseurl="https://github.com/Daenks/Meridian59_103/pull/$changelist.patch";

	my $content = get $baseurl;
	die "Couldn't get $baseurl" unless defined $content;

	my $patchfilename=$outdir."/$build\_$changelist.patch";
	open my $OUT,">",$patchfilename;
	print $OUT $content;
	close $OUT;

	my $output =`git checkout -b $changelist 2>&1`;
	if ($output !~ /Switched to a new branch/)
	{
		die "Couldn't create branch: $output";
	}
	#git am --3way --ignore-space-change --keep-cr $patchfilename
	$output = `git am --3way --ignore-space-change --keep-cr $patchfilename 2>&1`;
	my $RC = system("git am --3way --ignore-space-change --keep-cr $patchfilename");
	if ($RC != 0)
	{
		system("git am --abort");
		die "Failed Error Code: $RC\n";
	}
	print "Success Merging $changelist\n";
}