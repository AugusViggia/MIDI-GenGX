$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\MelodicMotionGuidance.h",
    "Source\Music\MelodicMotionGuidance.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 14.12 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
if ($CMake -notmatch "Source/Music/MelodicMotionGuidance\.cpp")
{
    throw "MelodicMotionGuidance.cpp is not part of MIDI_GenGX_Music."
}

$Composer = Get-Content -Raw (Join-Path $Root "Source\Music\MotifPhraseComposer.cpp")
foreach ($Symbol in @(
    "chooseMelodicMotionPitch",
    "context.parameters.complexity",
    "harmonyEvent->tension"
))
{
    if ($Composer -notmatch [regex]::Escape($Symbol))
    {
        throw "$Symbol is not integrated into MotifPhraseComposer."
    }
}

$Guidance = Get-Content -Raw (Join-Path $Root "Source\Music\MelodicMotionGuidance.cpp")
if ($Guidance -notmatch "MelodicMotion::analyzeInterval" -or
    $Guidance -notmatch "MelodicMotion::scoreInterval")
{
    throw "MelodicMotionGuidance does not consume MelodicMotion."
}

$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\MusicalEngineTests.cpp")
if ($Tests -notmatch "testMelodicMotionComposerIntegration")
{
    throw "Melodic motion integration regression test is missing."
}

Write-Host "Phase 14.12 melodic-motion integration validation passed." -ForegroundColor Green
