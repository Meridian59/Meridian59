# Crazy Andrew's Blakston server fake billing system.
#
# Accepts connections from Blakserv and handles a simple text string protocol
# for certain events (user login/logout, server startup).

$message = "";

$port = 4949;

$AF_INET = 2;
$SOCK_STREAM = 1;

$sockaddr = 'S n a4 x8';

($name, $aliases, $proto) = getprotobyname('tcp');
if ($port !~ /^\d+/) 
{
    ($name, $aliases, $port) = getservbyport($port, 'tcp');
}

print "Listening on port $port\n";

$this = pack($sockaddr, $AF_INET, $port, "\0\0\0\0");

# select(NS); $| = 1; select(stdout);

socket(S, $AF_INET, $SOCK_STREAM, $proto) || die "socket: $!";
bind(S, $this) || die "bind: $!";
listen(S, 5) || die "listen: $!";

# select(S); $| = 1; select(stdout);

for ($con = 1; ; $con++) 
{
    ($addr = accept(NS, S)) || die $!;

    ($af, $port, $inetaddr) = unpack($sockaddr, $addr);
    @inetaddr = unpack('C4', $inetaddr);
    print "Got connection #$con (port $port) from IP address @inetaddr\n";

    $command = "";

    while (1) 
    {
	$retval = recv(NS, $buf, 16384, 0);
	if (!$buf)
	{
	    last;
	}

	$command = $command.$buf;
#	print "command from connection $con: $command\n";

	if (($user, $server, $junk, $remainder) = ($command =~ /^LOGIN\t(\S+)\t(\d+)(.*)\n(.*)/))
	{
	    print "Got LOGIN command, user = $user, server number = $server\n";

	    send(NS, "LOGIN\tYES\t".$user."\t".$server."\t".$message."\n", 0);
	    $command = $remainder;
	}
	elsif (($user, $server, $junk, $remainder) = ($command =~ /^LOGOUT\t(\S+)\t(\d+)(.*)\n(.*)/))
	{
	    print "Got LOGOUT command, user = $user, server number = $server\n";
	    $command = $remainder;
	}
	elsif (($server, $junk, $remainder) = ($command =~ /^STARTUP\t(\d+)(.*)\n(.*)/))
	{
	    print "Got STARTUP command, server number = $server\n";

	    sleep(30);
	    send(NS, "STARTUP\t".$server."\n", 0);
	    $command = $remainder;
	}
	elsif (($junk, $remainder) = ($command =~ /^(.*)\n(.*)/))
	{
	    print "Got unknown command $junk\n";
	    $command = $remainder;
	}
    }
    print "Closing connection #$con\n";
    close(NS);
}
