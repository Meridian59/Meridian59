main();

sub main
{
    local $i;

    if (@ARGV < 1)
    {
	print "Usage:\n";
	print "genpass <# of passwords to generate>\n";
	return;
    }

    srand(time|$$);

    for ($i=0;$i<$ARGV[0];$i++)
    {
	print &generate_password();
	print "\n";
    }
}

sub generate_password
{
    local ($curpass,$password_chars,$i);

    @password_chars = ('a'..'h','m'..'n','p'..'z','A'..'H','M'..'N','P'..'Z','1'..'9');

    $curpass = "";

    for ($i=0;$i<10;$i++)
    {
	$curpass .= $password_chars[int(rand(@password_chars))];
    }

    $curpass;
}
