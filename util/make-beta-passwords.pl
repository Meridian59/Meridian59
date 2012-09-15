main();

sub main
{
    while (<>)
    {
	chop;
	foreach $i (split(' ',$_))
	{
	    print "create auto $i ",&generate_password(),"\n";
	}
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
