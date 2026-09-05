$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionMidiTrainingEvaluationService.h",
    "Source\Music\CompositionMidiTrainingEvaluationService.cpp",
    "Source\Tests\CompositionMidiTrainingEvaluationServiceTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 80 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiTrainingEvaluationService.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiTrainingEvaluationServiceTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionMidiTrainingEvaluationService\.cpp")
{
    throw "CompositionMidiTrainingEvaluationService.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionMidiTrainingEvaluationServiceTests")
{
    throw "CompositionMidiTrainingEvaluationServiceTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionMidiTrainingEvaluationServiceTests")
{
    throw "build-x64.ps1 does not execute CompositionMidiTrainingEvaluationServiceTests."
}

foreach ($Required in @(
    "trainCompositionNeuralModelFromMidiCorpus",
    "evaluateCompositionNeuralModel",
    "evaluateCompositionBaselineModel",
    "evaluateCompositionNeuralMusicalQuality"
))
{
    if ($Impl -notmatch $Required)
    {
        throw "Phase 80 evaluation stage is missing: $Required"
    }
}

if ($Impl -notmatch "neuralBeatsBaselineOnTest")
{
    throw "Phase 80 baseline comparison boundary is missing."
}

foreach ($Case in @(
    "testEndToEndEvaluation",
    "testEvaluationSplitsAreSeparated",
    "testMetricsAreDeterministic",
    "testInvalidCorpusIsRejected",
    "testBaselineComparisonIsWellDefined"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 80 test is missing: $Case"
    }
}

Write-Host "Phase 80 MIDI training evaluation validation passed." -ForegroundColor Green
