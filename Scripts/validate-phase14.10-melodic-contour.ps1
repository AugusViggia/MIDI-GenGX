$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\MelodicContour.h",
    "Source\Music\MelodicContour.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 14.10 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")

if ($CMake -notmatch "Source/Music/MelodicContour\.cpp")
{
    throw "MelodicContour.cpp is not part of MIDI_GenGX_Music."
}

$Engine = Get-Content -Raw (
    Join-Path $Root "Source\Music\MusicalEngine.cpp"
)

if ($Engine -match "double\s+contourShape\s*\(")
{
    throw "Contour policy is still embedded in MusicalEngine.cpp."
}

foreach ($Symbol in @(
    "MelodicContour::registerShape",
    "MelodicContour::endpointRegisterIndex"
))
{
    if ($Engine -notmatch [regex]::Escape($Symbol))
    {
        throw "$Symbol is not used by MusicalEngine."
    }
}

$Contour = Get-Content -Raw (
    Join-Path $Root "Source\Music\MelodicContour.cpp"
)

foreach ($Symbol in @(
    "MelodicContour::registerShape",
    "MelodicContour::endpointRegisterIndex"
))
{
    if ($Contour -notmatch [regex]::Escape($Symbol))
    {
        throw "$Symbol implementation is missing."
    }
}

$Tests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\MusicalEngineTests.cpp"
)

if ($Tests -notmatch "testMelodicContourPolicy")
{
    throw "Melodic contour regression tests are missing."
}

Write-Host "Phase 14.10 melodic contour architecture validation passed." -ForegroundColor Green
