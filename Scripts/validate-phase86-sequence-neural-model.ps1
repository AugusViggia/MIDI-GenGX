$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionSequenceNeuralModel.h",
    "Source\Music\CompositionSequenceNeuralModel.cpp",
    "Source\Music\CompositionSequenceNeuralTrainer.h",
    "Source\Music\CompositionSequenceNeuralTrainer.cpp",
    "Source\Tests\CompositionSequenceNeuralTrainerTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 86 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Model = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionSequenceNeuralModel.cpp")
$Trainer = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionSequenceNeuralTrainer.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionSequenceNeuralTrainerTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionSequenceNeuralModel\.cpp")
{
    throw "CompositionSequenceNeuralModel.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "Source/Music/CompositionSequenceNeuralTrainer\.cpp")
{
    throw "CompositionSequenceNeuralTrainer.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionSequenceNeuralTrainerTests")
{
    throw "CompositionSequenceNeuralTrainerTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionSequenceNeuralTrainerTests")
{
    throw "build-x64.ps1 does not execute CompositionSequenceNeuralTrainerTests."
}

foreach ($Required in @(
    "recurrentWeights",
    "predictNextEvent",
    "std::tanh"
))
{
    if ($Model -notmatch [regex]::Escape($Required))
    {
        throw "Phase 86 recurrent model component is missing: $Required"
    }
}

foreach ($Required in @(
    "trainWindow",
    "dhNext",
    "recurrentWeights",
    "gradientClip",
    "applyGradients"
))
{
    if ($Trainer -notmatch [regex]::Escape($Required))
    {
        throw "Phase 86 sequence trainer component is missing: $Required"
    }
}

foreach ($Case in @(
    "testModelInitialization",
    "testTrainingReducesLoss",
    "testPredictionIsValid",
    "testInvalidInputRejected",
    "testTrainingIsDeterministic"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 86 test is missing: $Case"
    }
}

Write-Host "Phase 86 sequence neural model validation passed." -ForegroundColor Green
