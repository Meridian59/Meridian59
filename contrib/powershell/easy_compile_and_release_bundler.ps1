<#
    .SYNOPSIS
    Converted the Post Build Batch for Meridian 59 to PowerShell and extended it with some goodies to ease the Deployment

    .DESCRIPTION
    Converted the Post Build Batch for Meridian 59 to PowerShell and extended it with some goodies to ease the Deployment
    Link to Post Build Batch File: https://github.com/Meridian59/Meridian59/wiki/Build-Instructions
    All credits to the Post Build Batch file and steps go to Andrew Kirmse

    .PARAMETER ClientResourceFolder
    Specifies the Folder where we copy the official Images, Music etc. from for the Classic Client

    .PARAMETER BuildType
    Specifies what kind of Build you want to trigger
    DEBUG:    Triggers a Debug Build over the Make File
    RELEASE:  Triggers a Release Build over the Make File and creates Zip Files of the Server and Client which can be deployed

    .PARAMETER ServerConfigTemplatePath
    OPTIONAL Parameter! If passed, it specifies the location of a Server Configuration File Template you have created with your custom settings,
    so that its copied after the Build and you don't always have to transfer it manually to your server folder

    .EXAMPLE
    Debug Build
    ./easy_compile_and_release_bundler.ps1 -ClientResourceFolder C:\temp\oldClient\Resources -BuildType DEBUG
    
    Release Build
    ./easy_compile_and_release_bundler.ps1 -ClientResourceFolder C:\temp\oldClient\Resources -BuildType RELEASE

    Release Build with Server Config Template passed
    ./easy_compile_and_release_bundler.ps1 -ClientResourceFolder C:\temp\oldClient\Resources -BuildType Release -ServerConfigTemplatePath "C:\Temp\blakserv_example_template.cfg"

    .NOTES
    Run this Script from the Developer PowerShell for VS 2017, 2019 or 2022

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


#Check if we are in the Root Directory of the Repository
Write-Host "Checking Working Directory..." -ForegroundColor Yellow
$workingDirectory = (Get-Item $PSScriptRoot).Parent.Parent.FullName

#If we are not in the Working Directory, switch to Working Directory
if( (Get-Location).Path -eq $workingDirectory )
{
  Write-Host "Already in correct Working Directory: $workingDirectory" -ForegroundColor Green
}
else
{  
  #Change Working Directory to the Root Folder of the Repository
  Write-Host "Changing Working Directory to: $workingDirectory" -ForegroundColor Yellow
  Set-Location $workingDirectory
}

#Check that the makefile exists in the Working Directory, otherwise abort script!
if( (Test-Path -Path "$workingDirectory\makefile") -eq $false)
{
  Write-Host "makefile not found! Aborting Script..." -ForegroundColor Red
  exit 1
}

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


Write-Host "Create Kod Folder in Server Directory, if it does not exist..." -ForegroundColor Yellow
if((Test-Path -Path .\run\server\kod) -eq $false)
{
    New-Item -ItemType Directory -Path .\run\server\kod
}

Write-Host "Create savegame Folder in Server Directory, if it does not exist..." -ForegroundColor Yellow
if((Test-Path -Path .\run\server\savegame) -eq $false)
{
    New-Item -ItemType Directory -Path .\run\server\savegame
}

#Copying Client Images and Stuff from the specified Resource Folder
Write-Host "Copying Graphics, Sounds etc..." -ForegroundColor Yellow
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

#check if the cfg Template Parameter was passed, if so copy the Server Configuration Template to the Server Folder
if( ($null -ne $ServerConfigTemplatePath) -and ($ServerConfigTemplatePath -ne ""))
{
  Write-Host "Copying blacksev.cfg Template to Server Folder..."
  Copy-Item -Path $ServerConfigTemplatePath -Destination .\run\server\blakserv.cfg
}

#if this is a Debug Build, copy the runLocalClient.ps1 script to .\run\localclient
if($BuildType -eq "DEBUG")
{
  Write-Host "Copying runLocalClient.ps1 to .\run\localclient Folder..."
  Copy-Item -Path .\contrib\powershell\runLocalClient.ps1 -Destination .\run\localclient\runLocalClient.ps1
}

#if this is a Release Build, compress the Server and Client to a Zip File, so that it can be deployed more easily
if($BuildType -eq "RELEASE")
{
    #Get the Date for the Folder Structure and Zips
    $date = (Get-Date).ToString("dd-MM-yyyy-hhmm")
    #Where we will place our Release Artifacts
    $releaseFolder = ".\run\releaseBuilds\$date"

    #Create a Release Folder, if it does not exist
    if((Test-Path -Path $releaseFolder ) -eq $false)
    {
        New-Item -ItemType Directory -Path $releaseFolder
    }

    #Copy the Server to the Release Folder
    Copy-Item -Recurse -Path .\run\server -Destination "$releaseFolder\server"
    #Copy the Client to the Release Folder
    Copy-Item -Recurse -Path .\run\localclient -Destination ".\$releaseFolder\classic_client"

    #Remove the .gitignore Files from the Client and Server Folder
    $items = Get-ChildItem -Recurse -Path $releaseFolder -Filter .gitignore
    foreach($item in $items)
    {
      Remove-Item -Path $item.FullName
    }
    
    #Compress the Client
    Write-Host "Compressing Client to Zip..." -ForegroundColor Yellow
    $clientPath = $releaseFolder + "\" + $date + "_ClassicClient.zip"
    Compress-Archive -Path "$releaseFolder\classic_client" -DestinationPath $clientPath -CompressionLevel Optimal

    #Remove classic_client Folder after Zipping
    if($? -eq $true)
    {
      if(Test-Path -Path "$releaseFolder\classic_client")
      {
        Write-Host "Removing Folder $releaseFolder\classic_client" -ForegroundColor Yellow
        Remove-Item -Recurse -Force -Path "$releaseFolder\classic_client"
      }
    }

    #Compress the Server
    Write-Host "Compressing Server to Zip..." -ForegroundColor Yellow
    $serverPath = $releaseFolder + "\" + $date + "_Server.zip"
    Compress-Archive -Path "$releaseFolder\server" -DestinationPath $serverPath -CompressionLevel Optimal

    #Remove server Folder after Zipping
    if($? -eq $true)
    {
      if(Test-Path -Path "$releaseFolder\server")
      {
        Write-Host "Removing Folder $releaseFolder\server" -ForegroundColor Yellow
        Remove-Item -Recurse -Force -Path "$releaseFolder\server"
      }
    }
}
