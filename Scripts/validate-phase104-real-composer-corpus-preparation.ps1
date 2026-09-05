$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionRealComposerCorpusPreparation.h",
    "Source\Music\CompositionRealComposerCorpusPreparation.cpp",
    "Source\Tests\CompositionRealComposerCorpusPreparationTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 104 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Implementation = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionRealComposerCorpusPreparation.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionRealComposerCorpusPreparationTests.cpp")

foreach ($Required in @(
    "CompositionRealComposerCorpusPreparation.h",
    "CompositionRealComposerCorpusPreparation.cpp"
))
{
    if ($CMake -notmatch [regex]::Escape("Source/Music/$Required"))
    {
        throw "Phase 104 source is not part of MIDI_GenGX_Music: $Required"
    }
}

if ($CMake -notmatch "MIDI_GenGX_CompositionRealComposerCorpusPreparationTests")
{
    throw "Phase 104 test target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionRealComposerCorpusPreparationTests")
{
    throw "build-x64.ps1 does not execute Phase 104 tests."
}

foreach ($Required in @(
    "prepareRealComposerCorpusFromRecords",
    "prepareRealComposerCorpusFromDirectory",
    "analyzeCompositionMidiSections",
    "analyzeCompositionMidiHarmony",
    "analyzeCompositionMidiMotifs",
    "buildCompositionMidiTrainingSequence",
    "buildCompositionConditionedTrainingDataset"
))
{
    if ($Implementation -notmatch [regex]::Escape($Required))
    {
        throw "Phase 104 preparation component is missing: $Required"
    }
}

foreach ($Case in @(
    "testRealComposerPreparationUsesFullMusicalEnrichment",
    "testMissingMetadataIsRejected",
    "testInvalidMusicalRecordIsRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 104 test is missing: $Case"
    }
}

Write-Host "Phase 104 real composer corpus preparation validation passed." -ForegroundColor Green
