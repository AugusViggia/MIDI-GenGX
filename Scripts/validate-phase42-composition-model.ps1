$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionModel.h",
    "Source\Music\CompositionModel.cpp",
    "Source\Tests\CompositionModelTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 42 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionModel.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionModel.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionModelTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionModel\.cpp")
{
    throw "CompositionModel.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionModelTests")
{
    throw "CompositionModelTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionModelTests")
{
    throw "build-x64.ps1 does not execute CompositionModelTests."
}

if ($Header -notmatch "struct CompositionModel")
{
    throw "Composition model is missing."
}

if ($Header -notmatch "predictNextSection")
{
    throw "Model inference interface is missing."
}

if ($Impl -notmatch "trainCompositionBaselineModel")
{
    throw "Model training implementation is missing."
}

if ($Impl -notmatch "trainingIndices")
{
    throw "Training-only model fitting is missing."
}

foreach ($Case in @(
    "testTrainingProducesValidModel",
    "testPredictionShapeAndFiniteness",
    "testValidationAndTestDataCannotTrainModel",
    "testPredictionRejectsInvalidContext",
    "testUntrainedModelCannotPredict",
    "testEmptyPreparedDataCannotTrainModel"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required composition-model test is missing: $Case"
    }
}

Write-Host "Phase 42 composition-model validation passed." -ForegroundColor Green
