$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\MelodicTendency.h",
    "Source\Music\MelodicTendency.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 14.14 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
if ($CMake -notmatch "Source/Music/MelodicTendency\.cpp")
{
    throw "MelodicTendency.cpp is not part of MIDI_GenGX_Music."
}

$Tendency = Get-Content -Raw (
    Join-Path $Root "Source\Music\MelodicTendency.cpp"
)

foreach ($Symbol in @(
    "analyzeTendencyTone",
    "scoreTendencyResolution"
))
{
    if ($Tendency -notmatch [regex]::Escape($Symbol))
    {
        throw "$Symbol implementation is missing."
    }
}

$Guidance = Get-Content -Raw (
    Join-Path $Root "Source\Music\MelodicMotionGuidance.cpp"
)

if ($Guidance -notmatch "analyzeTendencyTone" -or
    $Guidance -notmatch "scoreTendencyResolution")
{
    throw "Tendency logic is not integrated into melodic pitch selection."
}

$Composer = Get-Content -Raw (
    Join-Path $Root "Source\Music\MotifPhraseComposer.cpp"
)

if ($Composer -notmatch "chooseMelodicMotionPitchWithTendency")
{
    throw "Phrase composer does not use tendency-aware pitch selection."
}

$Tests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\MusicalEngineTests.cpp"
)

if ($Tests -notmatch "testMelodicTendencyModel")
{
    throw "Tendency-tone regression coverage is missing."
}

if ($Tests -notmatch "chromatic leading tone is detected outside natural minor scale")
{
    throw "Chromatic leading-tone regression coverage is missing."
}

if ($Tendency -notmatch "Tendency analysis intentionally happens")
{
    throw "Leading-tone detection must precede scale-membership rejection."
}

Write-Host "Phase 14.14 tendency-tone validation passed." -ForegroundColor Green
