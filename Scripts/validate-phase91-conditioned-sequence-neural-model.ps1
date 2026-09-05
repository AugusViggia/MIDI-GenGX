$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$RequiredFiles = @(
    "Source\Music\CompositionConditionedSequenceNeuralModel.h",
    "Source\Music\CompositionConditionedSequenceNeuralModel.cpp",
    "Source\Music\CompositionConditionedSequenceNeuralTrainer.h",
    "Source\Music\CompositionConditionedSequenceNeuralTrainer.cpp",
    "Source\Tests\CompositionConditionedSequenceNeuralTrainerTests.cpp"
)

foreach ($RelativePath in $RequiredFiles)
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 91 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Model = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedSequenceNeuralModel.cpp")
$Trainer = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedSequenceNeuralTrainer.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionConditionedSequenceNeuralTrainerTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionConditionedSequenceNeuralModel\.cpp" -or
    $CMake -notmatch "Source/Music/CompositionConditionedSequenceNeuralTrainer\.cpp")
{
    throw "Phase 91 conditioned neural sources are not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionConditionedSequenceNeuralTrainerTests")
{
    throw "Phase 91 test target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionConditionedSequenceNeuralTrainerTests")
{
    throw "build-x64.ps1 does not execute Phase 91 tests."
}

foreach ($Required in @(
    "composerEmbeddings",
    "styleEmbeddings",
    "eraEmbeddings",
    "instrumentationEmbeddings",
    "predictNextEvent",
    "conditionEmbeddingWidth"
))
{
    if ($Model -notmatch [regex]::Escape($Required))
    {
        throw "Phase 91 conditioning component is missing: $Required"
    }
}

foreach ($Required in @(
    "trainCompositionConditionedSequenceNeuralModel",
    "addEmbeddingGradient",
    "composerEmbeddings",
    "styleEmbeddings"
))
{
    if ($Trainer -notmatch [regex]::Escape($Required))
    {
        throw "Phase 91 conditioned trainer component is missing: $Required"
    }
}

foreach ($Case in @(
    "testModelInitialization",
    "testPredictionUsesCondition",
    "testTrainingReducesLoss",
    "testTrainingIsDeterministic"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 91 test is missing: $Case"
    }
}

Write-Host "Phase 91 conditioned sequence neural model validation passed." -ForegroundColor Green
