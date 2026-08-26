$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ProjectRoot

$VCTools = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231"
$WindowsKit = "C:\Program Files (x86)\Windows Kits\10"
$WindowsKitVersion = "10.0.26100.0"

$MsvcBinX64 = Join-Path $VCTools "bin\Hostx64\x64"
$MsvcLibX64 = Join-Path $VCTools "lib\x64"
$WindowsUmX64 = Join-Path $WindowsKit "Lib\$WindowsKitVersion\um\x64"
$WindowsUcrtX64 = Join-Path $WindowsKit "Lib\$WindowsKitVersion\ucrt\x64"

$BuildDir = Join-Path $ProjectRoot "out\build\x64-Debug"
$ArtifactPath = Join-Path $BuildDir "MIDI_GenGX_artefacts\Debug\VST3\MIDI-GenGX.vst3"
$UserVst3Dir = Join-Path $env:LOCALAPPDATA "Programs\Common\VST3"
$UserPluginPath = Join-Path $UserVst3Dir "MIDI-GenGX.vst3"

foreach ($RequiredPath in @(
    $ProjectRoot,
    $BuildDir,
    $VCTools,
    $MsvcBinX64,
    $MsvcLibX64,
    $WindowsUmX64,
    $WindowsUcrtX64
))
{
    if (-not (Test-Path $RequiredPath))
    {
        throw "Required x64 build path not found: $RequiredPath"
    }
}

$env:PATH = "$MsvcBinX64;$env:PATH"
$env:LIB = "$MsvcLibX64;$WindowsUcrtX64;$WindowsUmX64"
$env:LIBPATH = "$MsvcLibX64"
$env:VCToolsInstallDir = "$VCTools\"

Write-Host "FAST x64 VST3 build..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI-GenGX.vst3" `
    --parallel

if ($LASTEXITCODE -ne 0)
{
    throw "Fast x64 VST3 build failed."
}

if (-not (Test-Path $ArtifactPath))
{
    throw "VST3 artifact not found: $ArtifactPath"
}

if (Test-Path $UserPluginPath)
{
    try
    {
        Remove-Item $UserPluginPath -Recurse -Force -ErrorAction Stop
    }
    catch
    {
        throw "MIDI-GenGX.vst3 is locked. Close Ableton Live and retry."
    }
}

New-Item -ItemType Directory -Force -Path $UserVst3Dir | Out-Null

Copy-Item `
    -Path $ArtifactPath `
    -Destination $UserPluginPath `
    -Recurse `
    -Force

if (-not (Test-Path $UserPluginPath))
{
    throw "VST3 installation failed: $UserPluginPath"
}

Write-Host "FAST X64 VST3 BUILD + INSTALL PASSED" -ForegroundColor Green
