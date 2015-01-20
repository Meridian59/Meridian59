<?php
class Settings
{
	public $ServerName;
	public $PatchInfoUrl;
	public $ClientFolder;
	public $PatchBaseUrl;
	public $Guid;
	public $FullInstallUrl;
	public $AccountCreationUrl;
	public $default = true;
	
	function __construct($Server = 103) //default to 103
	{
       switch ($Server)
	   {
			case "3B89295C-F19C-46C3-8B8A-2F51F2C3A8C9": //server 1
			case 1:
				$this->ServerName = "Korea 1";
				$this->PatchInfoUrl = "http://m59.iptime.org/1/patchinfo.txt";
				$this->ClientFolder = "C:\\Program Files\\Open Meridian\\Meridian 1";
				$this->PatchBaseUrl = "http://m59.iptime.org/1/clientpatch";
				$this->Guid = "3B89295C-F19C-46C3-8B8A-2F51F2C3A8C9";
				$this->FullInstallUrl = "";
				$this->AccountCreationUrl = "http://ww1.openmeridian.org/1/acctcreate.php";
				$this->default = false;
				break;
			case "5AD1FB01-A84A-47D1-85B8-5F85FB0C201E": //server 103
			case 103:
				$this->ServerName = "103";
				$this->PatchInfoUrl = "http://ww1.openmeridian.org/103/patchinfo.txt";
				$this->ClientFolder = "C:\\Program Files\\Open Meridian\\Meridian 103";
				$this->PatchBaseUrl = "http://ww1.openmeridian.org/103/clientpatch";
				$this->Guid = "5AD1FB01-A84A-47D1-85B8-5F85FB0C201E";
				$this->FullInstallUrl = "";
				$this->AccountCreationUrl = "http://ww1.openmeridian.org/103/acctcreate.php";
				$this->default = true;
				break;
			case "EACFDF63-65A6-46C3-AC99-1C5BAB07EDEB": //server 104
			case 104:
				$this->ServerName = "104";
				$this->PatchInfoUrl = "http://ww1.openmeridian.org/104/patchinfo.txt";
				$this->ClientFolder = "C:\\Program Files\\Open Meridian\\Meridian 104";
				$this->PatchBaseUrl = "http://ww1.openmeridian.org/104/clientpatch";
				$this->Guid = "EACFDF63-65A6-46C3-AC99-1C5BAB07EDEB";
				$this->FullInstallUrl = "";
				$this->AccountCreationUrl = "http://ww1.openmeridian.org/104/acctcreate.php";
				$this->default = false;
				break;
	   }
    }
	
	function ToJson()
	{
		return json_encode($this, SON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES );
	}
	
	function AllToJson()
	{
		return json_encode( Array (new Settings(1),new Settings(103),new Settings(104)), JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES );
	}
}

$settings = new Settings();
if ( isset( $_GET['debug'] ) && !empty( $_GET['debug'] ) )
  echo "<pre>";

print($settings->AllToJson());

if ( isset( $_GET['debug'] ) && !empty( $_GET['debug'] ) )
  echo "</pre>";

?>
