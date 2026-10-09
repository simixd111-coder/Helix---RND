$ErrorActionPreference = 'Stop'

$packageName = 'helix-rnd'
$version = '3.0.0'
$releaseUrl = "https://github.com/simixd111-coder/Helix---RND/releases/download/v$version"
$zipName = "helix-rnd-$version-windows-x64.zip"
$url64 = "$releaseUrl/$zipName"
$checksumFile = Join-Path $env:TEMP "$packageName-$version.sha256"
Get-ChocolateyWebFile -PackageName $packageName -FileFullPath $checksumFile -Url "$url64.sha256"
$checksumLine = Get-Content -LiteralPath $checksumFile -Raw
if ($checksumLine -notmatch '^(?<hash>[A-Fa-f0-9]{64})\s+') {
    throw "The release checksum file for $zipName is invalid."
}
$checksum64 = $Matches.hash
Remove-Item -LiteralPath $checksumFile -Force
$checksumType64 = 'sha256'

$toolsDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$installDir = Join-Path $env:ChocolateyInstall "lib\$packageName"
$includeDir = Join-Path $installDir "include"
$libDir = Join-Path $installDir "lib"
$binDir = Join-Path $installDir "bin"

$packageArgs = @{
    PackageName    = $packageName
    Url64          = $url64
    Checksum64     = $checksum64
    ChecksumType64 = $checksumType64
    UnzipLocation  = $installDir
}

Install-ChocolateyZipPackage @packageArgs

# Create directory structure
New-Item -ItemType Directory -Force -Path $includeDir
New-Item -ItemType Directory -Force -Path $libDir
New-Item -ItemType Directory -Force -Path $binDir

# Move files to proper locations (adjust based on actual zip structure)
$extractedDir = Get-ChildItem -Path $installDir -Directory | Where-Object { $_.Name -like 'helix-rnd-*' } | Select-Object -First 1

if ($extractedDir) {
    # Copy headers
    if (Test-Path (Join-Path $extractedDir.FullName 'include')) {
        Copy-Item -Path (Join-Path $extractedDir.FullName 'include\*') -Destination $includeDir -Recurse -Force
    }
    
    # Copy libraries
    if (Test-Path (Join-Path $extractedDir.FullName 'lib')) {
        Copy-Item -Path (Join-Path $extractedDir.FullName 'lib\*') -Destination $libDir -Recurse -Force
    }
    
    # Copy binaries
    if (Test-Path (Join-Path $extractedDir.FullName 'bin')) {
        Copy-Item -Path (Join-Path $extractedDir.FullName 'bin\*') -Destination $binDir -Recurse -Force
    }
}

# Add to PATH if binaries exist
if (Test-Path $binDir) {
    Install-ChocolateyPath -PathToInstall $binDir -PathType 'Machine'
}

# Write version info
$versionInfo = @{
    Version = $version
    InstallDate = Get-Date -Format 'yyyy-MM-dd'
    InstallDir = $installDir
}
$versionInfo | ConvertTo-Json | Out-File (Join-Path $installDir 'version.json')

Write-Host "Helix RND $version installed successfully to $installDir" -ForegroundColor Green
