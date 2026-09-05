$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionMidiTrainingSequence.h",
    "Source\Music\CompositionMidiTrainingSequence.cpp",
    "Source\Tests\CompositionMidiTrainingSequenceTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 83 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiTrainingSequence.cpp")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiTrainingSequence.h")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiTrainingSequenceTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionMidiTrainingSequence\.cpp")
{
    throw "CompositionMidiTrainingSequence.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionMidiTrainingSequenceTests")
{
    throw "CompositionMidiTrainingSequenceTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionMidiTrainingSequenceTests")
{
    throw "build-x64.ps1 does not execute CompositionMidiTrainingSequenceTests."
}

if ($Header -notmatch "featureCount\s*=\s*20")
{
    throw "Phase 83 event schema width is not 20."
}

foreach ($Required in @(
    "midiNote",
    "velocity",
    "deltaTicks",
    "durationBeats",
    "sectionProgress",
    "encodeRole",
    "encodeHarmonyQuality",
    "previousPitch"
))
{
    if ($Impl -notmatch $Required)
    {
        throw "Phase 83 musical representation component is missing: $Required"
    }
}

if ($Impl -notmatch "section.tension")
{
    throw "Section tension is not represented in the event schema."
}

if ($Impl -notmatch "harmonic.confidence")
{
    throw "Harmonic confidence is not represented in the event schema."
}

foreach ($Case in @(
    "testSequenceIsValid",
    "testFeatureWidthIsStable",
    "testAllFeaturesAreBounded",
    "testSequenceIsDeterministic",
    "testInvalidMidiIsRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 83 test is missing: $Case"
    }
}


$SectionHeader = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiSection.h")

if ($Impl -match "section\.tensionDeltaFromPrevious")
{
    throw "Phase 83 references nonexistent CompositionMidiSection::tensionDeltaFromPrevious."
}

if ($SectionHeader -notmatch "\bint\s+tension\s*=\s*0")
{
    throw "Phase 83 validator cannot confirm the supported MIDI section tension field."
}


$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiTrainingSequenceTests.cpp")

if ($Tests -match "!sequence\.valid")
{
    throw "Phase 83 tests use a nonexistent public `valid` field; use CompositionMidiTrainingSequence::isValid()."
}

if ($Tests -notmatch "!\s*sequence\.isValid\(\)")
{
    throw "Phase 83 invalid-input test does not use isValid()."
}

Write-Host "Phase 83 musical representation validation passed." -ForegroundColor Green
