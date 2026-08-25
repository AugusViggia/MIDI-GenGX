$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\MelodicMotion.h",
    "Source\Music\MelodicMotion.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 14.11 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")

if ($CMake -notmatch "Source/Music/MelodicMotion\.cpp")
{
    throw "MelodicMotion.cpp is not part of MIDI_GenGX_Music."
}

$Motion = Get-Content -Raw (
    Join-Path $Root "Source\Music\MelodicMotion.cpp"
)

foreach ($Symbol in @(
    "MelodicMotion::analyzeInterval",
    "MelodicMotion::preferredMaximumLeap",
    "MelodicMotion::scoreInterval"
))
{
    if ($Motion -notmatch [regex]::Escape($Symbol))
    {
        throw "$Symbol implementation is missing."
    }
}

$Tests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\MusicalEngineTests.cpp"
)

foreach ($TestName in @(
    "testMelodicContourPolicy",
    "testMelodicMotionModel"
))
{
    if ($Tests -notmatch $TestName)
    {
        throw "$TestName regression coverage is missing."
    }
}

Write-Host "Phase 14.11 melodic-motion validation passed." -ForegroundColor Green
