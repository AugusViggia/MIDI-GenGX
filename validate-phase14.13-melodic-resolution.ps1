$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\MelodicResolution.h",
    "Source\Music\MelodicResolution.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 14.13 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
if ($CMake -notmatch "Source/Music/MelodicResolution\.cpp")
{
    throw "MelodicResolution.cpp is not part of MIDI_GenGX_Music."
}

$Guidance = Get-Content -Raw (
    Join-Path $Root "Source\Music\MelodicMotionGuidance.cpp"
)

if ($Guidance -notmatch "MelodicResolution::scoreResolution")
{
    throw "MelodicResolution is not integrated into motion-aware pitch choice."
}

$Resolution = Get-Content -Raw (
    Join-Path $Root "Source\Music\MelodicResolution.cpp"
)

foreach ($Symbol in @(
    "MelodicResolution::isLeap",
    "MelodicResolution::preferredResolutionDirection",
    "MelodicResolution::scoreResolution"
))
{
    if ($Resolution -notmatch [regex]::Escape($Symbol))
    {
        throw "$Symbol implementation is missing."
    }
}

$Tests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\MusicalEngineTests.cpp"
)

if ($Tests -notmatch "testMelodicResolutionModel")
{
    throw "Melodic resolution regression coverage is missing."
}

Write-Host "Phase 14.13 melodic-resolution validation passed." -ForegroundColor Green
