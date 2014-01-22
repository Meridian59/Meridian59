use Getopt::Long;
use Net::Telnet ();
use Net::FTP;

my @changelists;
my $build;
my $clientMinVersion;
my $shutdowndelay = 10;

GetOptions ("change=s" => \@changelists,
            "build=s" => \$build,
			"clientMinVersion=s" => \$clientMinVersion);

my $host = "10.0.50.220";
my $port = 9992;

print "Connecting to server 104\n";
$t = new Net::Telnet (Timeout => 10);
$t->port($port);
$t->open($host);
print "Warn of impending shutdown\n";
sleep 1;
$t->print("send o 0 AdminSystemMessage string 8 Buildserver shutting down blakserv in order to apply build $build in $shutdowndelay seconds");
sleep $shutdowndelay;
#shutdown the things
$t->print("terminate nosave");

print "Transferring C:/packages/server-$build.zip to server\n";
$ftp = Net::FTP->new($host, Debug => 0)
  or die "Cannot connect to $host: $@";

$ftp->login("buildserver",'betafreak!@#')
  or die "Cannot login to $host", $ftp->message;

$ftp->binary;
$ftp->put("C:/packages/server-$build.zip")
  or die "get failed ", $ftp->message;
$ftp->quit;
