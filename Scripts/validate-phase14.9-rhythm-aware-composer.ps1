$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\RhythmPlan.h",
    "Source\Music\RhythmPlan.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 14.9 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
if ($CMake -notmatch "Source/Music/RhythmPlan\.cpp")
{
    throw "RhythmPlan.cpp is not part of MIDI_GenGX_Music."
}

$Rhythm = Get-Content -Raw (
    Join-Path $Root "Source\Music\RhythmPlan.cpp"
)

foreach ($Symbol in @(
    "planRhythm",
    "applyRhythmPlan",
    "RhythmPlan::isValid"
))
{
    if ($Rhythm -notmatch [regex]::Escape($Symbol))
    {
        throw "$Symbol implementation is missing."
    }
}

$Composer = Get-Content -Raw (
    Join-Path $Root "Source\Music\MotifPhraseComposer.cpp"
)

if ($Composer -notmatch "planRhythm")
{
    throw "MotifPhraseComposer does not consume RhythmPlan."
}

if ($Composer -notmatch "applyRhythmPlan")
{
    throw "MotifPhraseComposer does not apply rhythmic development."
}

$Tests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\MusicalEngineTests.cpp"
)

if ($Tests -notmatch "testRhythmPlanningAndDevelopment")
{
    throw "Rhythm planning regression test is missing."
}

Write-Host "Phase 14.9 rhythm-aware composer validation passed." -ForegroundColor Green
