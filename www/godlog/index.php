<HEAD>
<TITLE>Server 103 God Log</TITLE>
</HEAD>
<BODY>
<STRONG>Server 103 God Log</STRONG>
<P>
<?php
$handle = @fopen("C:/Meridian59-master/run/server/channel/god.txt", "r");
if ($handle) {
    while (($buffer = fgets($handle, 4096)) !== false) 
	{
		if (strpos($buffer, 'account') !== FALSE)
			echo '--------->Line Omitted due to containing login information<BR>';
        else
			echo $buffer. "<BR>";
    }
    if (!feof($handle)) {
        echo "Error: unexpected fgets() fail\n";
    }
    fclose($handle);
}
?>
</P>
</BODY>