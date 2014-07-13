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
my $servernumber = "104";
my $patchinfopath = "C:/wamp/www/$servernumber/patchinfo.txt";
my $clientpackagepath = "C:/wamp/www/$servernumber/clientpatch";

GetOptions ("build=s" => \$build,
			"clientMinVersion=s" => \$clientMinVersion,
			"servernumber=s" => \$servernumber,
#			"serverpackagepath=s" => $serverpackagepath,
			"patchinfopath=s" => \$patchinfopath,
			"clientpackagepath=s" => \$clientpackagepath);


			my $serverpackagepath = "C:/packages/server-$build";
my $zip = Archive::Zip->new();

#create serverpackagepath
print "Creating server path and subdirs\n";
make_path("$serverpackagepath",
          "$serverpackagepath/channel",
		  "$serverpackagepath/savegame", 
		  "$serverpackagepath/download");
		  
#create clientpackagepath
print "Creating client path and subdirs\n";
make_path("$clientpackagepath/resource",
          "$clientpackagepath/mail",
		  "$clientpackagepath/download",
		  "$clientpackagepath/ads",
		  "$clientpackagepath/help");

#copy server files from build
print "Copying server files....\n";
copy("./run/server/blakserv.exe",$serverpackagepath);
copy("./lib/libmysql.dll",$serverpackagepath);
copy("./kod/kodbase.txt",$serverpackagepath);
copy("./kod/include/blakston.khd",$serverpackagepath);
dircopy("./resource/rooms","$serverpackagepath/rooms");
dircopy("./run/server/rsc","$serverpackagepath/rsc");
dircopy("./run/server/memmap","$serverpackagepath/memmap");
dircopy("./run/server/loadkod","$serverpackagepath/loadkod");

#copy client files from build
print "Copying client files.....\n";
copy("./run/localclient/meridian.exe",$clientpackagepath);
copy("./run/localclient/m59bind.exe",$clientpackagepath);
copy("./run/localclient/wavplay.dll",$clientpackagepath);
copy("./run/localclient/wrap.dll",$clientpackagepath);
copy("./run/localclient/zlib1.dll",$clientpackagepath);
copy("./module/admin/debug/admin.dll",$clientpackagepath . "/resource");
copy("./module/char/debug/char.dll",$clientpackagepath . "/resource");
copy("./module/chess/debug/chess.dll",$clientpackagepath . "/resource");
copy("./module/dm/debug/dm.dll",$clientpackagepath . "/resource");
copy("./module/intro/debug/intro.dll",$clientpackagepath . "/resource");
copy(".//module/mailnews/debug/mailnews.dll",$clientpackagepath . "/resource");
copy("./module/merintr/debug/merintr.dll",$clientpackagepath . "/resource");

#diff the two branches and come up with changed .roo .bgf .mp3 etc

#generate motd file???
#foreach change something something???

#generate blakserv.cfg
#caclculate a port number based on server number?????
print "Writing blakserv.cfg\n";
open my $OUT,">$serverpackagepath/blakserv.cfg";
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
PackagePath          /$servernumber/
DownloadReason       <An update of Meridian files is available and required.>

[Console]            
Administrator        Administrator
Caption              <$servernumber>

[MySQL]
Enabled              Yes
Username             root
Password             <>
Host                 localhost
Database             meridian104";
close $OUT;

#generate rsb file
print "Generating rsb file:\n.\\bin\\rscmerge.exe -o $clientpackagepath/resource/rsc0000.rsb ./run/server/rsc/*.rsc\n";
my $RC = system(".\\bin\\rscmerge.exe -o $clientpackagepath/resource/rsc0000.rsb ./run/server/rsc/*.rsc");
if ($RC != 0)
{
	die "rscmerge failed to generate client.rsb file!";
}

#generate patchlistinfo
print "Generating Patch JSON: $patchinfopath\n";
my $output = `C:/qbscripts/PatchListGenerator.exe --client=$clientpackagepath --outfile=$patchinfopath`;

#creates a zip file for easy FTP to 
print "Generating pack archive: C:/packages/server-$build.zip\n";
$zip = Archive::Zip->new();
$zip->addTree( $serverpackagepath );
unless ( $zip->writeToFileNamed("C:/packages/server-$build.zip") == AZ_OK ) 
{
	die 'write error';
}