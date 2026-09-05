$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionModelEvaluation.h",
    "Source\Music\CompositionModelEvaluation.cpp",
    "Source\Tests\CompositionModelEvaluationTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 47 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionModelEvaluation.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionModelEvaluation.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionModelEvaluationTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionModelEvaluation\.cpp")
{
    throw "CompositionModelEvaluation.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionModelEvaluationTests")
{
    throw "CompositionModelEvaluationTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionModelEvaluationTests")
{
    throw "build-x64.ps1 does not execute CompositionModelEvaluationTests."
}

if ($Header -notmatch "struct CompositionModelEvaluationResult")
{
    throw "Model evaluation result is missing."
}

if ($Impl -notmatch "meanSquaredError")
{
    throw "MSE metric is missing."
}

if ($Impl -notmatch "meanAbsoluteError")
{
    throw "MAE metric is missing."
}

if ($Impl -notmatch "validationIndices")
{
    throw "Validation-set evaluation path is missing."
}

if ($Impl -notmatch "testIndices")
{
    throw "Test-set evaluation path is missing."
}

foreach ($Case in @(
    "testBaselineValidationEvaluation",
    "testNeuralTestEvaluation",
    "testValidationAndTestUseDifferentSplits",
    "testEvaluationIsDeterministic",
    "testInvalidModelIsRejected",
    "testEmptyEvaluationSetIsRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required model-evaluation test is missing: $Case"
    }
}

Write-Host "Phase 47 composition-model-evaluation validation passed." -ForegroundColor Green
