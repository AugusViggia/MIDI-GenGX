$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionMidiDatasetBuilder.h",
    "Source\Music\CompositionMidiDatasetBuilder.cpp",
    "Source\Tests\CompositionMidiDatasetBuilderTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 76 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiDatasetBuilder.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiDatasetBuilderTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionMidiDatasetBuilder\.cpp")
{
    throw "CompositionMidiDatasetBuilder.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionMidiDatasetBuilderTests")
{
    throw "CompositionMidiDatasetBuilderTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionMidiDatasetBuilderTests")
{
    throw "build-x64.ps1 does not execute CompositionMidiDatasetBuilderTests."
}

foreach ($Required in @(
    "analyzeCompositionMidiCorpus",
    "analyzeCompositionMidiSections",
    "analyzeCompositionMidiHarmony",
    "analyzeCompositionMidiMotifs",
    "buildSample"
))
{
    if ($Impl -notmatch $Required)
    {
        throw "MIDI dataset builder is missing pipeline stage: $Required"
    }
}

if ($Impl -notmatch "findById")
{
    throw "Dataset duplicate-id protection is missing."
}

if ($Impl -notmatch "std::sort")
{
    throw "Deterministic dataset ordering is missing."
}

foreach ($Case in @(
    "testSingleMidiRecordBuildsDataset",
    "testMultipleMidiRecordsProduceSortedDataset",
    "testDuplicateIdsAreRejected",
    "testInvalidRecordsAreRejected",
    "testFeatureWidthsMatchSchema",
    "testBuildIsDeterministic"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 76 test is missing: $Case"
    }
}


if ($Impl -notmatch "std::set<std::string>")
{
    throw "MIDI dataset builder duplicate detection is not independent of the unsorted dataset vector."
}

if ($Impl -match "dataset\.findById\(")
{
    throw "MIDI dataset builder still uses findById() before final dataset sorting."
}

Write-Host "Phase 76 MIDI dataset builder validation passed." -ForegroundColor Green
