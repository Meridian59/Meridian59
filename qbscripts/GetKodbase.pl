use Net::FTP;

my $kodpath = "C:/Meridian59-quickbuilds/kod";
my $host = "10.0.50.220";

chdir $kodpath;

my $ftp = Net::FTP->new($host, Debug => 0)
	or die "Cannot connect to $host: $@";
  
$ftp->login("buildserver",'betafreak!@#')
	or die "Cannot login to $host", $ftp->message;
  
$ftp->cwd("/download")
	or die "Cannot change working directory ", $ftp->message;
	
$ftp->get("kodbase.txt")
	or die "get failed ", $ftp->message;

$ftp->quit;

print "GETKODBASE: obtained $kodpath/kodbase.txt via FTP from $host.\n";

	
