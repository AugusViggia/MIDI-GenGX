$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionConditionedSequenceNeuralEvaluator.h",
    "Source\Music\CompositionConditionedSequenceNeuralEvaluator.cpp",
    "Source\Tests\CompositionConditionedSequenceNeuralEvaluatorTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 107 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Service = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedCorpusTrainingService.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionConditionedSequenceNeuralEvaluatorTests.cpp")

foreach ($Required in @(
    "CompositionConditionedSequenceNeuralEvaluator.cpp",
    "CompositionConditionedSequenceNeuralEvaluator.h"
))
{
    if ($CMake -notmatch [regex]::Escape("Source/Music/$Required"))
    {
        throw "Phase 107 evaluator is not in MIDI_GenGX_Music: $Required"
    }
}

if ($CMake -notmatch "MIDI_GenGX_CompositionConditionedSequenceNeuralEvaluatorTests")
{
    throw "Phase 107 evaluator test target is missing."
}

foreach ($Required in @(
    "evaluateCompositionConditionedSequenceNeuralModel",
    "validationEvaluation",
    "testEvaluation",
    "deserializeCompositionConditionedSequenceNeuralModel"
))
{
    if ($Service -notmatch [regex]::Escape($Required))
    {
        throw "Phase 107 corpus training integration is missing: $Required"
    }
}

foreach ($Required in @(
    "testEvaluatorProducesFiniteLoss",
    "testInvalidDatasetFailsClosed"
))
{
    if ($Tests -notmatch $Required)
    {
        throw "Required Phase 107 test is missing: $Required"
    }
}

Write-Host "Phase 107 neural evaluation validation passed." -ForegroundColor Green
