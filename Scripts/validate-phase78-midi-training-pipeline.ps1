$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionMidiTrainingPipeline.h",
    "Source\Music\CompositionMidiTrainingPipeline.cpp",
    "Source\Tests\CompositionMidiTrainingPipelineTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 78 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiTrainingPipeline.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiTrainingPipelineTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionMidiTrainingPipeline\.cpp")
{
    throw "CompositionMidiTrainingPipeline.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionMidiTrainingPipelineTests")
{
    throw "CompositionMidiTrainingPipelineTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionMidiTrainingPipelineTests")
{
    throw "build-x64.ps1 does not execute CompositionMidiTrainingPipelineTests."
}

foreach ($Required in @(
    "loadCompositionMidiCorpusDirectory",
    "buildCompositionDatasetFromMidiCorpus",
    "assessCompositionDatasetQuality",
    "buildCompositionDatasetPartition",
    "buildCompositionDatasetManifest",
    "prepareCompositionDatasetForLearning",
    "serializeCompositionTrainingCorpus"
))
{
    if ($Impl -notmatch $Required)
    {
        throw "Phase 78 pipeline stage is missing: $Required"
    }
}

if ($Impl -notmatch "CompositionMidiTrainingPipelineResult")
{
    throw "Phase 78 result boundary is missing."
}

foreach ($Case in @(
    "testEndToEndTrainingPipeline",
    "testSmallCorpusStillGetsRequestedSplits",
    "testInvalidDirectoryFailsPipeline",
    "testEmptyDirectoryIsRejectedAtTrainingBoundary",
    "testPipelineIsDeterministic"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 78 test is missing: $Case"
    }
}


$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiTrainingPipeline.h")

foreach ($RequiredInclude in @(
    'CompositionMidiCorpusDirectoryLoader\.h',
    'CompositionMidiDatasetBuilder\.h',
    'CompositionDatasetManifest\.h',
    'CompositionDatasetPartition\.h',
    'CompositionDatasetQuality\.h'
))
{
    if ($Header -notmatch '#include\s+"' + $RequiredInclude + '"')
    {
        throw "Phase 78 pipeline header is missing required type dependency: $RequiredInclude"
    }
}


$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiTrainingPipelineTests.cpp")

if ($Tests -notmatch "beat\s*<\s*64")
{
    throw "Phase 78 end-to-end fixture does not contain 64 musical beats."
}

if ($Tests -notmatch "0,\s*4,\s*7")
{
    throw "Phase 78 end-to-end fixture does not provide triadic harmonic evidence."
}

if ($Tests -notmatch "0x83" -or
    $Tests -notmatch "0x60")
{
    throw "Phase 78 end-to-end fixture does not encode a real quarter-note duration."
}

if ($Tests -notmatch "Hold the complete triad for one quarter note")
{
    throw "Phase 78 end-to-end fixture does not document its MIDI timing contract."
}


$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiTrainingPipelineTests.cpp")

if ($Tests -match "track\.pop_back\(\)")
{
    throw "Phase 78 fixture must not remove final MIDI note-off bytes."
}

if ($Tests -notmatch "The final chord already ends at the correct timestamp")
{
    throw "Phase 78 fixture does not document final MIDI event handling."
}


if ($Impl -notmatch "result\.corpus\.records\.empty\(\)")
{
    throw "Phase 78 training boundary does not explicitly reject an empty MIDI corpus."
}

Write-Host "Phase 78 MIDI training pipeline validation passed." -ForegroundColor Green
