param([string]$Configuration = 'Release')
$ErrorActionPreference = 'Stop'
if ($Configuration -ne 'Release') { throw 'Shareable packages must use Release.' }
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
Push-Location $projectRoot
try {
    & (Join-Path $PSScriptRoot 'build_msvc.bat') Release StageGame
    if ($LASTEXITCODE -ne 0) { throw 'Game build failed. See compiler output.' }
    $gameDirectory = Join-Path $projectRoot 'dist/Game/Release'
    $zip = Join-Path $projectRoot 'dist/Game-windows-x64.zip'
    Compress-Archive -Path (Join-Path $gameDirectory '*') -DestinationPath $zip -Force
    Write-Host "Game folder: $gameDirectory"
    Write-Host "Share: $zip"
} finally { Pop-Location }
