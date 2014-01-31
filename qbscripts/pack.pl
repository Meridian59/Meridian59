use strict;
use warnings;
use LWP::Simple;
use Getopt::Long;
use File::Path qw(make_path remove_tree);
use File::Copy;
use File::Copy::Recursive qw(fcopy rcopy dircopy fmove rmove dirmove);
use Archive::Zip qw( :ERROR_CODES :CONSTANTS );

my @changelists;
my $build;
my $clientMinVersion;

GetOptions ("change=s" => \@changelists,
            "build=s" => \$build,
			"clientMinVersion=s" => \$clientMinVersion);

my $packagepath = "C:/packages/server-$build";

#create package path
make_path("$packagepath",
          "$packagepath/channel",
		  "$packagepath/savegame", 
		  "$packagepath/download",);

#copy files from build
print "Copying blakserv, libmysql, kodbase, blakston.khd, rooms, rsc, memmap, loadkod\n";
copy("./run/server/blakserv.exe",$packagepath);
copy("./lib/libmysql.dll",$packagepath);
copy("./kod/kodbase.txt",$packagepath);
copy("./kod/include/blakston.khd",$packagepath);
dircopy("./resource/rooms","$packagepath/rooms");
dircopy("./run/server/rsc","$packagepath/rsc");
dircopy("./run/server/memmap","$packagepath/memmap");
dircopy("./run/server/loadkod","$packagepath/loadkod");

#generate blakserv.cfg
print "Writing blakserv.cfg\n";
open my $OUT,">$packagepath/blakserv.cfg";
print $OUT "[Path]               
Bof                  loadkod\\
Memmap               memmap\\
Rsc                  rsc\\
Rooms                rooms\\
Motd                 .\\
Channel              channel\\
LoadSave             savegame\\
Forms                .\\
Kodbase              .\\
PackageFile          .\\

[Channel]            
DebugDisk            Yes
ErrorDisk            Yes
LogDisk              Yes
Flush                Yes
GodDisk              Yes

[Constants]          
Enabled   Yes
Filename  .\\blakston.khd

[Login]              
MinVersion           $clientMinVersion

[Socket]             
Port                 5904
MaintenancePort      9992
MaintenanceMask      127.0.0.1;10.0.50.221

[Update]             
ClientMachine        meridian103.daenks.org
ClientFilename       client$build"."exe.zip
PackageMachine       meridian103.daenks.org
PackagePath          /104/
DownloadReason       <An update of Meridian files is available and required.>

[Console]            
Administrator        Administrator
Caption              <104>

[MySQL]
Enabled              Yes
Username             root
Password             <>
Host                 localhost
Database             meridian104";
close $OUT;

#generate rsb file
print "Generating rsb file\n";
my $RC = system(".\\bin\\rscmerge.exe -o $packagepath/rsc0000.rsb ./run/server/rsc/*.rsc");
if ($RC != 0)
{
	die "rscmerge failed to generate client.rsb file!";
}

#generate client$build.zip from .rsb file
print "Zipping rsb file\n";
my $zip = Archive::Zip->new();
$zip->addFile("$packagepath/rsc0000.rsb","rsc0000.rsb");
unless ( $zip->writeToFileNamed("$packagepath/download/client$build.zip") == AZ_OK ) 
{
	die "unable to write $packagepath/download/client$build.zip";
}
unlink "$packagepath/rsc0000.rsb";

#generate client$build"."exe.zip
print "Zipping meridian.exe\n";
$zip = Archive::Zip->new();
$zip->addFile("./run/localclient/meridian.exe","meridian.exe");
unless ( $zip->writeToFileNamed("$packagepath/download/client$build"."exe.zip") == AZ_OK ) 
{
	die "unable to write $packagepath/download/client$build"."exe.zip";
}


#generate packages.txt
print "Generating packages file\n";
open $OUT,">$packagepath/packages.txt";
print $OUT "client$build.zip 10000 0";
close $OUT;

#creates a zip file for easy FTP to 
print "Generating pack archive\n";
$zip = Archive::Zip->new();
$zip->addTree( $packagepath );
unless ( $zip->writeToFileNamed("C:/packages/server-$build.zip") == AZ_OK ) 
{
	die 'write error';
}