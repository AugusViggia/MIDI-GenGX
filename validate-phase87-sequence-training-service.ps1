$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionSequenceNeuralModelArtifact.h",
    "Source\Music\CompositionSequenceNeuralModelArtifact.cpp",
    "Source\Music\CompositionSequenceNeuralTrainingService.h",
    "Source\Music\CompositionSequenceNeuralTrainingService.cpp",
    "Source\Tests\CompositionSequenceNeuralTrainingServiceTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 87 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Artifact = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionSequenceNeuralModelArtifact.cpp")
$Service = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionSequenceNeuralTrainingService.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionSequenceNeuralTrainingServiceTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionSequenceNeuralModelArtifact\.cpp")
{
    throw "Sequence model artifact implementation is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "Source/Music/CompositionSequenceNeuralTrainingService\.cpp")
{
    throw "Sequence neural training service is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionSequenceNeuralTrainingServiceTests")
{
    throw "CompositionSequenceNeuralTrainingServiceTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionSequenceNeuralTrainingServiceTests")
{
    throw "build-x64.ps1 does not execute the Phase 87 tests."
}

if ($Artifact -notmatch "MGSN")
{
    throw "Phase 87 sequence model artifact identifier is missing."
}

foreach ($Required in @(
    "serializeCompositionSequenceNeuralModel",
    "deserializeCompositionSequenceNeuralModel",
    "trainCompositionSequenceNeuralModel"
))
{
    if (($Artifact + $Service) -notmatch $Required)
    {
        throw "Phase 87 training/artifact stage is missing: $Required"
    }
}

foreach ($Case in @(
    "testCorpusArtifactTrainingProducesModelArtifact",
    "testTrainedArtifactRoundTripPreservesPredictions",
    "testInvalidCorpusRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 87 test is missing: $Case"
    }
}

Write-Host "Phase 87 sequence training service validation passed." -ForegroundColor Green
