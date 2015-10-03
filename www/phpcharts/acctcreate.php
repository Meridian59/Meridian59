<?php
/* Meridian 59 Server Account Creation Script
 * Written by Daenks (daenks@daenks.org) (c) 2013
 * You may use this software as is with no guarantee or warranty
 * You may distribute this software. You may modify it to meet your needs.
 * Do not remove this header block.
 */


function display_form()
{?>
<STYLE type=text/css>
TD {
	FONT-SIZE: SMALL; FONT-FAMILY: Verdana,Arial,Helvetica; color: #FFFFFF; background: #000000; 
}
Body {
	FONT-SIZE: SMALL; FONT-FAMILY: Verdana,Arial,Helvetica; color: #FFFFFF; background: #000000; 
}
</STYLE>
<FORM ACTION="<?php echo $_SERVER['PHP_SELF']; ?>" METHOD="post">
<TABLE>
<TR><TD align=left>Username:</TD></TR>
<TR><TD><INPUT TYPE="text" NAME="username" size=16></TD></TR>
<TR><TD align=left>Password:</TD></TR>
<TR><TD><INPUT TYPE="password" NAME="password1" size=16></TD></TR>
<TR><TD align=left>Confirm Password:</TD></TR>
<TR><TD><INPUT TYPE="password" NAME="password2" size=16></TD></TR>
<TR><TD><INPUT TYPE="submit" NAME="submit" TEXT="Create"></TD></TR>
</TABLE>
</FORM>
<?php
}

function validate_input()
{	
	$error = false;
	if (!(strlen($_POST['username']) > 3))
	{
		echo "<B><font color=red>Username must be at least 4 characters</font></B><BR>";
		$error = true;
	}
	if (!(strlen($_POST['password1']) > 5))
	{
		echo "<B><font color=red>Password must be at least 6 characters</font></B><BR>";
		$error = true;
	}
	if (!($_POST['password1'] == $_POST['password2']))
	{
		echo "<B><font color=red>Password fields must match</font></B><BR>";
		$error = true;
	}
	if (!preg_match('/[a-z]/',strtolower($_POST['username'])))
	{
		echo "<B><font color=red>Username must contain at least one letter</font></B><BR>";
		$error = true;
	}
	if ($error)
		display_form();
	else
		check_account_exists();
}

function check_account_exists()
{
	$username = strtolower(trim($_POST['username']));
	$command = "show account $username\r\n";

	$sock = fsockopen("127.0.0.1",9996,$errornumber, $errorstring, 30);
	if (!$sock)
		die("Error $errornumber: $errorstring");
	
	fputs($sock, $command);
	sleep(1);
	$result = fread($sock,1000);
	fclose($sock);

	if (strpos($result,"Cannot find account"))
		create_account();
	else
	{
		echo "<B><font color=red>That account name is taken, please try again.</font></B><BR>";
		display_form();
	}
}

function create_account()
{
	$username = strtolower(trim($_POST['username']));
	$password = trim($_POST['password1']);
	$command = "create account user $username $password\r\n";
	
	$sock = fsockopen("127.0.0.1",9996,$errornumber, $errorstring, 30);
	if (!$sock)
		die("Error $errornumber: $errorstring");
	
	fputs($sock, $command);
	sleep(1);
	$result = fread($sock,1000);

	$lines = explode("ACCOUNT",$result);
	$accountID = trim(trim($lines[1]),".");
	
	$command = "create user $accountID\r\n";
	
	fputs($sock, $command);
	sleep(1);
	$result = fread($sock, 1000);
	
	fputs($sock, $command);
	sleep(1);
	$result = fread($sock, 1000);
	
	fputs($sock, $command);
	sleep(1);
	$result = fread($sock, 1000);
	
	fputs($sock, $command);
	sleep(1);
	$result = fread($sock, 1000);
	echo "<B><font color=blue>Account $username created with four character slots.</font></B><BR>";
}
?>
<HTML>
<HEAD>
<TITLE>Meridian 59 - Server 103 - Account Creator</TITLE>
</HEAD>
<BODY>
<?php
if(!isset($_POST['submit']))
{
	display_form();
}
else
{
	validate_input();	
}
?>


</BODY>
</HTML>
