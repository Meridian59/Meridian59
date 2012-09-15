main();

sub main
{
    local $i;

    while (<>)
    {
	($account, $password) = /create auto (.*) (.*)/;

	$email = $account;
	$account =~ s/_/ /;

	open(F,">mail.txt") || die "Can't open mail.txt for writing: $!\n";
	print F "Welcome to the Beta test for Meridan 59!  Your username and password are\n";
	print F "included below. Please keep this e-mail in case you forget your password.\n";
	print F "You will find information about how to download the Beta version of the\n";
	print F "software at:\n";
	print F "\n";
	print F "http://www.otherrealms.com/beta\n";
	print F "\n";
	print F "Username: $account\n";
	print F "Password: $password\n";
	print F "\n";
	print F "If you have any problems, please e-mail info\@terranova.com.\n";
	print F "\n";
	print F "Christopher Kirmse\n";
	print F "Archetype Interactive\n";
	close(F);
	print STDOUT ">> sending mail to $email\n";
      system("blat mail.txt -s \"Meridian 59 beta\" -t $email -f info\@terranova.com\n");
	unlink("mail.txt");
    }

}




