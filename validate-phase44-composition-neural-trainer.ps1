$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionNeuralTrainer.h",
    "Source\Music\CompositionNeuralTrainer.cpp",
    "Source\Tests\CompositionNeuralTrainerTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 44 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionNeuralTrainer.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionNeuralTrainer.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionNeuralTrainerTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionNeuralTrainer\.cpp")
{
    throw "CompositionNeuralTrainer.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionNeuralTrainerTests")
{
    throw "CompositionNeuralTrainerTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionNeuralTrainerTests")
{
    throw "build-x64.ps1 does not execute CompositionNeuralTrainerTests."
}

if ($Header -notmatch "CompositionNeuralTrainingConfig")
{
    throw "Neural training configuration is missing."
}

if ($Impl -notmatch "trainCompositionNeuralModel")
{
    throw "Neural training implementation is missing."
}

if ($Impl -notmatch "trainingIndices")
{
    throw "Training-only fitting is missing."
}

if ($Impl -notmatch "outputBias")
{
    throw "Training update target is missing."
}

if ($Impl -notmatch "hiddenGradient")
{
    throw "Hidden-layer backpropagation is missing."
}

if ($Impl -notmatch "inputWeights")
{
    throw "Input-weight backpropagation is missing."
}

if ($Impl -notmatch "outputWeights")
{
    throw "Output-weight backpropagation is missing."
}

if ($Impl -notmatch "gradientClip")
{
    throw "Gradient clipping contract is missing."
}

if ($Impl -notmatch "CompositionNeuralOptimizer::Adam")
{
    throw "Adam optimizer path is missing."
}

if ($Impl -notmatch "CompositionNeuralOptimizer::SGD")
{
    throw "SGD optimizer path is missing."
}

if ($Impl -notmatch "applyAdam")
{
    throw "Adam parameter update implementation is missing."
}

foreach ($Case in @(
    "testTrainingProducesValidResult",
    "testTrainingChangesModelParameters",
    "testTrainingIsDeterministic",
    "testLossIsStableOrImproves",
    "testValidationAndTestDataAreNotUsed",
    "testInvalidConfigurationIsRejected",
    "testEmptyPreparedDataIsRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required neural-trainer test is missing: $Case"
    }
}


if ($Tests -match '(?m)^\s*CompositionModel\s+neuralModel')
{
    throw "Trainer tests must use CompositionNeuralModel, not the statistical CompositionModel."
}

Write-Host "Phase 44 composition-neural-trainer validation passed." -ForegroundColor Green
