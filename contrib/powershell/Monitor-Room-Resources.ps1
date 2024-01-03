<#
    .DESCRIPTION
    Monitors `/resource/rooms` for new or updated `.roo` files, and when found, copies them to `run/localclient/resource`.

    .EXAMPLE
    From the project root:
    ./contrib/powershell/Monitor-Room-Resources.ps1

    .NOTES
    Saves time and clicks when making and testing frequent room file changes.
#>

# Get the absolute path of the directory where the script is located
$scriptDirectory = Split-Path -Parent -Path $MyInvocation.MyCommand.Definition

# Construct the absolute paths for source and destination folders
$sourceFolder = Join-Path -Path $scriptDirectory -ChildPath "..\..\resource\rooms"
$destinationFolder = Join-Path -Path $scriptDirectory -ChildPath "..\..\run\localclient\resource"

# Normalize the paths (resolves any "..")
$sourceFolder = [System.IO.Path]::GetFullPath($sourceFolder)
$destinationFolder = [System.IO.Path]::GetFullPath($destinationFolder)

if (-not (Test-Path -Path $sourceFolder)) {
  Write-Host "Directory $sourceFolder does not exist. Exiting script."
  exit
}

# Create a FileSystemWatcher object to watch the source folder
$watcher = New-Object System.IO.FileSystemWatcher
$watcher.Path = $sourceFolder
$watcher.IncludeSubdirectories = $false
$watcher.EnableRaisingEvents = $true

# Define the filter and types of changes to watch for
$watcher.Filter = "*.roo" # Monitor room files, change this to something like "*.txt" to monitor only text files
$watcher.NotifyFilter = [System.IO.NotifyFilters]'FileName, LastWrite'

# Define the action to take when a file is changed, created, or deleted
$action = {
  param($source, $e)

  $changeType = $e.ChangeType
  $fileName = $e.FullPath
  $destFile = Join-Path $destinationFolder (Split-Path $fileName -Leaf)

  Write-Host "File $fileName was $changeType at $(Get-Date) - copying to $destFile"

  # Copy the file to the destination folder
  Copy-Item $fileName -Destination $destFile -Force
}

# Attach event handlers
Register-ObjectEvent $watcher "Created" -Action $action
Register-ObjectEvent $watcher "Changed" -Action $action
Register-ObjectEvent $watcher "Deleted" -Action $action

# Write a message to indicate the script is running
Write-Host "Watching for changes to files in $sourceFolder..."

# Prevent the script from exiting immediately
while ($true) {
  Start-Sleep -Seconds 2
}
