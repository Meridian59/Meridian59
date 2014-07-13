use Net::Telnet ();
use File::Copy;

chdir "C:/Users/Administrator/Desktop/server104";
system("runas /profile /user:dstone \"C:/Users/Administrator/Desktop/server104/blakserv.exe\"");

print "Waiting 30 seconds for blakserv to start\n";
sleep 30;

print "Connecting to server 104\n";
$t = new Net::Telnet (Timeout => 10);
$t->port($port);
$t->open($host);
print "Setting Daenks' password to \"1\"\n";
$t->print("\rset account password 1 1");
sleep(1);
print "Sending recreateall\n";
$t->print("\rsend o 0 RecreateAll");
sleep(1);