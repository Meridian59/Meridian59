use Getopt::Long;
use Archive::Zip qw( :ERROR_CODES :CONSTANTS );
use File::Path qw(make_path remove_tree);

my $build;
my $workingdir = "C:/Users/Administrator/Desktop/server104";
my $zip;

GetOptions("build=s" => \$build);

remove_tree("$workingdir/channel",
            "$workingdir/download",
			"$workingdir/loadkod",
			"$workingdir/memmap",
			"$workingdir/rooms",
			"$workingdir/rsc",
			"$workingdir/savegame");

unlink "$workingdir/blakserv.cfg";
unlink "$workingdir/blakserv.exe";
unlink "$workingdir/blakston.khd";
unlink "$workingdir/kodbase.txt";
unlink "$workingdir/libmysql.dll";
unlink "$workingdir/packages.txt";

chdir "$workingdir";
$zip=Archive::Zip->new ( "server-$build.zip" );
$zip->extractTree;