<?php
class Settings
{
	public $ServerName = "103";
	public $PatchInfoUrl = "http://ww1.openmeridian.org/103/patchinfo.txt";
	public $ClientFolder = "C:\\Program Files\\Open Meridian\\Meridian 103";
	public $PatchBaseUrl = "http://ww1.openmeridian.org/103/clientpatch";
	public $Guid = "5AD1FB01-A84A-47D1-85B8-5F85FB0C201E";
	public $FullInstallUrl = "";
	public $AccountCreationUrl = "http://ww1.openmeridian.org/103/acctcreate.php";
	public $default = true;
	
	function __construct($Server) 
	{
       switch ($Server)
	   {
			case "3B89295C-F19C-46C3-8B8A-2F51F2C3A8C9": //server 1
			case 1:
				$ServerName = "Korea 1";
				$PatchInfoUrl = "http://m59.iptime.org/1/patchinfo.txt";
				$ClientFolder = "C:\\Program Files\\Open Meridian\\Meridian 1";
				$PatchBaseUrl = "http://m59.iptime.org/1/clientpatch";
				$Guid = "3B89295C-F19C-46C3-8B8A-2F51F2C3A8C9";
				$FullInstallUrl = "";
				$AccountCreationUrl = "http://ww1.openmeridian.org/1/acctcreate.php";
				$default = false;
				break;
			default:
			case "5AD1FB01-A84A-47D1-85B8-5F85FB0C201E": //server 103
			case 103:
				$ServerName = "103";
				$PatchInfoUrl = "http://ww1.openmeridian.org/103/patchinfo.txt";
				$ClientFolder = "C:\\Program Files\\Open Meridian\\Meridian 103";
				$PatchBaseUrl = "http://ww1.openmeridian.org/103/clientpatch";
				$Guid = "5AD1FB01-A84A-47D1-85B8-5F85FB0C201E";
				$FullInstallUrl = "";
				$AccountCreationUrl = "http://ww1.openmeridian.org/103/acctcreate.php";
				$default = true;
				break;
			case "EACFDF63-65A6-46C3-AC99-1C5BAB07EDEB": //server 104
			case 104:
				$ServerName = "104";
				$PatchInfoUrl = "http://ww1.openmeridian.org/104/patchinfo.txt";
				$ClientFolder = "C:\\Program Files\\Open Meridian\\Meridian 104";
				$PatchBaseUrl = "http://ww1.openmeridian.org/104/clientpatch";
				$Guid = "EACFDF63-65A6-46C3-AC99-1C5BAB07EDEB";
				$FullInstallUrl = "";
				$AccountCreationUrl = "http://ww1.openmeridian.org/104/acctcreate.php";
				$default = false;
				break;
	   }
    }
}

//$AllSettings = Array (new Settings(1),new Settings(103),new Settings(104));

print(json_encode( new Settings(1)));


?>
