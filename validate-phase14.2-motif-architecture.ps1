$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\Motif.h",
    "Source\Music\Motif.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 14.2 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
if ($CMake -notmatch "Source/Music/Motif\.cpp")
{
    throw "Motif.cpp is not part of MIDI_GenGX_Music."
}

$Motif = Get-Content -Raw (
    Join-Path $Root "Source\Music\Motif.cpp"
)

foreach ($Symbol in @(
    "extractMotif",
    "applyMotif",
    "Motif::isValid"
))
{
    if ($Motif -notmatch [regex]::Escape($Symbol))
    {
        throw "$Symbol implementation is missing."
    }
}

$Tests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\MusicalEngineTests.cpp"
)

if ($Tests -notmatch "testMotifRoundTrip")
{
    throw "Motif round-trip test is missing."
}

Write-Host "Phase 14.2 motif architecture validation passed." -ForegroundColor Green
