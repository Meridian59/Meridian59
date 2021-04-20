<#
    .SYNOPSIS
    Converted the Post Build Batch for Meridian 59 to Powershell and exteneded it with some neat tweeks

    .DESCRIPTION
    Converted the Post Build Batch for Meridian 59 to Powershell and exteneded it with some neat tweeks
    Link to Post Build Batch File: https://github.com/Meridian59/Meridian59/wiki/Build-Instructions
    All credits to the Post Build Batch file and steps go to Andrew Kirmse

    .PARAMETER ClientResourceFolder
    Specifies the Folder where we copy the official images, music etc. from for the Classic Client

    .PARAMETER BuildType
    Specifies what kind of Build you want to trigger
    DEBUG:    Triggers a Debug Build over the Make File
    RELEASE:  Triggers a Release Build over the Make File and creates Zip Files of the Server and Client at the End which can be deployed

    .PARAMETER ServerConfigTemplatePath
    OPTIONAL Parameter! If passed it specifies the location of a Server Config File Template you have created with your custom settings,
    so that its copied after the Build and you don't always have to transfer it manually to your server folder

    .EXAMPLE
    Debug Build
    ./compile.ps1 -ClientResourceFolder C:\temp\oldClient\Resources -BuildType DEBUG
    
    Release Build
    ./compile.ps1 -ClientResourceFolder C:\temp\oldClient\Resources -BuildType RELEASE

    Release Build with Server Config Template passed
    ./compile.ps1 -ClientResourceFolder C:\temp\oldClient\Resources -BuildType Release -ServerConfigTemplatePath "C:\Temp\blakserv_template.cfg"

    .NOTES
    Run this from the Developer PowerShell for VS 2017 or 2019

#>

Param
(
  [Parameter(Mandatory=$true)][String] $ClientResourceFolder,
  [Parameter(Mandatory=$true)][ValidateSet("DEBUG","RELEASE")][String] $BuildType,
  [String] $ServerConfigTemplatePath 
)


Write-Host "#####################################" -ForegroundColor Cyan
Write-Host "Thorbenn's Meridian 59 Compile Script" -ForegroundColor Cyan
Write-Host "#####################################" -ForegroundColor Cyan

#Compile the Code according to the Mode passed as parameter
switch($BuildType)
{
  'DEBUG' 
  {
    Write-Host "Compiling Server and Client in Debug Mode..." -ForegroundColor Yellow
    nmake DEBUG=1
  }
  
  'RELEASE' 
  {
    Write-Host "Compiling Server and Client in Release Mode..." -ForegroundColor Yellow
    nmake RELEASE=1
  }
}


Write-Host "Cleaning Clients .rsc Files..." -ForegroundColor Yellow
$items = Get-ChildItem -Path .\run\localclient\resource -Filter *.rsc
foreach($oldRSC in $items)
{
  $name = $oldRSC.FullName
  Write-Host "Removing: $name"
  Remove-Item -Path $name
}


Write-Host "Copying RSC Files to Server Folder..." -ForegroundColor Yellow
$items = Get-ChildItem -Path .\kod -Filter *.rsc
foreach($item in $items)
{
  $name = $item.FullName
  Write-Host "Copying: $name"
  Copy-Item -Path $name -Destination .\run\server\rsc\
}



Write-Host "Build RSB file..." -ForegroundColor Yellow
.\bin\rscmerge.exe -o .\run\localclient\resource\rsc0000.rsb .\run\server\rsc\*.rsc
if($? -eq $true)
{
  Write-Host "RSB created successfully!" -ForegroundColor Green
}

Write-Host "Copying Room Files to Client Folder..." -ForegroundColor Yellow
$items = Get-ChildItem -Path .\resource\rooms -Filter *.roo
foreach($item in $items)
{
  $name = $item.FullName
  Write-Host "Copying: $name to .\run\localclient\resource\"
  Copy-Item -Path $name -Destination .\run\localclient\resource\
}

Write-Host "Copying Rooms Folder to Server Folder..." -ForegroundColor Yellow
if((Test-Path -Path ".\run\server\rooms\") -ne $true)
{
    New-Item -ItemType Directory -Path .\run\server\rooms\
}

$items = Get-ChildItem -Path .\resource\rooms -Filter *.roo
foreach($item in $items)
{
  $name = $item.FullName
  Write-Host "Copying: $name to .\run\server\rooms\"
  Copy-Item -Path $name -Destination .\run\server\rooms\
}

Write-Host "Copying the required .dll files to the Client Folder..." -ForegroundColor Yellow
switch($BuildType)
{
  'DEBUG' 
  {
    Copy-Item -Path .\module\*\debug -Include *.dll -Destination .\run\localclient\resource\
  }
  
  'RELEASE' 
  {
    Copy-Item -Path .\module\*\release -Include *.dll -Destination .\run\localclient\resource\
  }
}


Write-Host "Create Kod Folder in Server Directory if it does not exist..." -ForegroundColor Yellow
if((Test-Path -Path .\run\server\kod) -eq $false)
{
    New-Item -ItemType Directory -Path .\run\server\kod
}

Write-Host "Create savegame Folder in Server Directory if it does not exist..." -ForegroundColor Yellow
if((Test-Path -Path .\run\server\savegame) -eq $false)
{
    New-Item -ItemType Directory -Path .\run\server\savegame
}

#Copying Client Images and Stuff from specified Resource Folder passed as Parameter
Write-Host "Copying Graphics, Sound etc..." -ForegroundColor Yellow
$items = Get-ChildItem -Path $ClientResourceFolder -Filter *.bgf
foreach($item in $items)
{
  $name = $item.FullName
  Write-Host "Copying: $name to .\run\localclient\resource\"
  Copy-Item -Path $name -Destination .\run\localclient\resource\
}

$items = Get-ChildItem -Path $ClientResourceFolder -Filter *.wav
foreach($item in $items)
{
  $name = $item.FullName
  Write-Host "Copying: $name to .\run\localclient\resource\"
  Copy-Item -Path $name -Destination .\run\localclient\resource\
}

$items = Get-ChildItem -Path $ClientResourceFolder -Filter *.xmi
foreach($item in $items)
{
  $name = $item.FullName
  Write-Host "Copying: $name to .\run\localclient\resource\"
  Copy-Item -Path $name -Destination .\run\localclient\resource\
}

$items = Get-ChildItem -Path $ClientResourceFolder -Filter *.mp3
foreach($item in $items)
{
  $name = $item.FullName
  Write-Host "Copying: $name to .\run\localclient\resource\"
  Copy-Item -Path $name -Destination .\run\localclient\resource\
}

$items = Get-ChildItem -Path $ClientResourceFolder -Filter *.bsf
foreach($item in $items)
{
  $name = $item.FullName
  Write-Host "Copying: $name to .\run\localclient\resource\"
  Copy-Item -Path $name -Destination .\run\localclient\resource\
}

$items = Get-ChildItem -Path $ClientResourceFolder -Filter *.mid
foreach($item in $items)
{
  $name = $item.FullName
  Write-Host "Copying: $name to .\run\localclient\resource\"
  Copy-Item -Path $name -Destination .\run\localclient\resource\
}

$items = Get-ChildItem -Path $ClientResourceFolder -Filter *.ogg
foreach($item in $items)
{
  $name = $item.FullName
  Write-Host "Copying: $name to .\run\localclient\resource\"
  Copy-Item -Path $name -Destination .\run\localclient\resource\
}

#check if the cfg template parameter was passed, if so copy the server config template to the server folder
if( ($null -ne $ServerConfigTemplatePath) -and ($ServerConfigTemplatePath -ne ""))
{
  Write-Host "Copying blacksev.cfg Template to Server Folder..."
  Copy-Item -Path $ServerConfigTemplatePath -Destination .\run\server\blakserv.cfg
}


#if this is a release build compress the Server and client to a Zip File so that it can be deployed
if($BuildType -eq "RELEASE")
{
    #Get the Date for the Folder Structure and Zips
    $date = (Get-Date).ToString("dd-MM-yyyy")
    #Where we will place our Release Artifacts
    $releaseFolder = ".\run\releaseBuilds\$date"

    #Create a Release Folder if it does not exist
    if((Test-Path -Path $releaseFolder ) -eq $false)
    {
        New-Item -ItemType Directory -Path $releaseFolder
    }

    #Copy the Server to the Release Folder
    Copy-Item -Recurse -Path .\run\server -Destination "$releaseFolder\server"
    #Copy the Client to the Release Folder
    Copy-Item -Recurse -Path .\run\localclient -Destination ".\$releaseFolder\classic_client"

    #Remove the .gitignore Files from the client and server folder
    $items = Get-ChildItem -Recurse -Path $releaseFolder -Filter .gitignore
    foreach($item in $items)
    {
        Remove-Item -Path $item.FullName
    }
    
    #Compress the Client
    Write-Host "Compressing Client to Zip..." -ForegroundColor Yellow
    $clientPath = $releaseFolder + "\" + $date + "_ClassicClient.zip"
    Compress-Archive -Path "$releaseFolder\classic_client" -DestinationPath $clientPath -CompressionLevel Optimal

    #Compress the Server
    Write-Host "Compressing Server to Zip..." -ForegroundColor Yellow
    $serverPath = $releaseFolder + "\" + $date + "_Server.zip"
    Compress-Archive -Path "$releaseFolder\server" -DestinationPath $serverPath -CompressionLevel Optimal
}
