# Set the folder paths
$sourceFolder = "../../resource/rooms" # Change this to your source folder path
$destinationFolder = "../../run/localclient/resource" # Change this to your destination folder path

# Create a FileSystemWatcher object to watch the source folder
$watcher = New-Object System.IO.FileSystemWatcher
$watcher.Path = $sourceFolder
$watcher.IncludeSubdirectories = $false
$watcher.EnableRaisingEvents = $true

# Define the filter and types of changes to watch for
$watcher.Filter = "*.roo" # Monitor all files, change this to something like "*.txt" to monitor only text files
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
