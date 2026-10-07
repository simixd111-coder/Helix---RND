[CmdletBinding()]
param(
    [string]$BuildDir = "build-test",
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug"
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
$buildPath = Join-Path $repoRoot $BuildDir
$cachePath = Join-Path $buildPath "CMakeCache.txt"

if (-not (Test-Path -LiteralPath $cachePath)) {
    throw "Build directory '$BuildDir' is not configured. Configure CMake with tests, software backend, and headless demo enabled."
}

foreach ($requiredOption in @(
    "^HX_BUILD_TESTS:BOOL=ON$",
    "^HX_BACKEND_SOFTWARE:BOOL=ON$",
    "^HX_BUILD_HEADLESS_DEMO:BOOL=ON$"
)) {
    if (-not (Select-String -LiteralPath $cachePath -Pattern $requiredOption -Quiet)) {
        throw "Build directory '$BuildDir' must enable tests, software rendering, and the headless demo."
    }
}

$metadataChecks = @(
    @{ Path = "CMakeLists.txt"; Pattern = 'VERSION 2\.0\.0' },
    @{ Path = "include/helix.h"; Pattern = '^#define HX_VERSION_STRING "2\.0\.0"$' },
    @{ Path = "packaging/nuget/HelixRND/HelixRND.csproj"; Pattern = '<Version>2\.0\.0</Version>' },
    @{ Path = "packaging/chocolatey/helix-rnd.nuspec"; Pattern = '<version>2\.0\.0</version>' },
    @{ Path = "packaging/chocolatey/tools/chocolateyInstall.ps1"; Pattern = '^\$version = ''2\.0\.0''$' }
)
foreach ($check in $metadataChecks) {
    $filePath = Join-Path $repoRoot $check.Path
    if (-not (Select-String -LiteralPath $filePath -Pattern $check.Pattern -Quiet)) {
        throw "Version metadata mismatch in '$($check.Path)'."
    }
}

foreach ($workflow in @(".github/workflows/ci.yml", ".github/workflows/release.yml")) {
    if (Select-String -LiteralPath (Join-Path $repoRoot $workflow) -Pattern 'macos-latest' -Quiet) {
        throw "macOS must remain outside the 2.0 CI and release matrix ('$workflow')."
    }
}

Write-Host "Building Helix RND 2.0.0 ($Configuration)..."
& cmake --build $buildPath --config $Configuration --parallel
if ($LASTEXITCODE -ne 0) {
    throw "Build failed with exit code $LASTEXITCODE."
}

Write-Host "Running the complete CTest suite..."
& ctest --test-dir $buildPath -C $Configuration --output-on-failure
if ($LASTEXITCODE -ne 0) {
    throw "CTest failed with exit code $LASTEXITCODE."
}

Write-Host "Helix RND 2.0.0 verification passed."
