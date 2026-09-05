$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionMidiCorpusTrainingService.h",
    "Source\Music\CompositionMidiCorpusTrainingService.cpp",
    "Source\Tests\CompositionMidiCorpusTrainingServiceTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 79 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiCorpusTrainingService.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiCorpusTrainingServiceTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionMidiCorpusTrainingService\.cpp")
{
    throw "CompositionMidiCorpusTrainingService.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionMidiCorpusTrainingServiceTests")
{
    throw "CompositionMidiCorpusTrainingServiceTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionMidiCorpusTrainingServiceTests")
{
    throw "build-x64.ps1 does not execute CompositionMidiCorpusTrainingServiceTests."
}

foreach ($Required in @(
    "buildCompositionMidiTrainingPipeline",
    "trainCompositionNeuralArtifact",
    "result.neuralTraining",
    "CompositionMidiCorpusTrainingResult"
))
{
    if ($Impl -notmatch $Required)
    {
        throw "Phase 79 orchestration stage is missing: $Required"
    }
}

foreach ($Case in @(
    "testRealMidiCorpusTrains",
    "testTrainingProducesNonConstantLoss",
    "testInvalidCorpusIsRejected",
    "testTrainingIsDeterministic"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 79 test is missing: $Case"
    }
}

Write-Host "Phase 79 MIDI corpus neural training validation passed." -ForegroundColor Green
