use Net::Telnet ();
use File::Copy;

$SIG{CHLD} = 'IGNORE';

print "Starting blakserv...\n";
chdir "C:/Users/Administrator/Desktop/server104";

system(1,"C:/Users/Administrator/Desktop/server104/blakserv.exe");

print "Waiting 5 seconds for blakserv to start\n";
sleep 5;

print "Connecting to server 104\n";
$t = new Net::Telnet (Timeout => 10);
$t->port(9992);
$t->open("localhost");
print "Setting Daenks' password to \"1\"\n";
$t->print("set account password 1 1");
sleep(1);
print "Sending recreateall\n";
$t->print("send o 0 RecreateAll");