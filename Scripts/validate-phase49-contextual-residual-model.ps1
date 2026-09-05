$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionNeuralModel.h",
    "Source\Music\CompositionNeuralModel.cpp",
    "Source\Music\CompositionNeuralTrainer.cpp",
    "Source\Tests\CompositionNeuralModelTests.cpp",
    "Source\Tests\CompositionNeuralTrainerTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 49 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Model = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionNeuralModel.cpp")
$ModelHeader = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionNeuralModel.h")
$Trainer = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionNeuralTrainer.cpp")
$ModelTests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionNeuralModelTests.cpp")
$TrainerTests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionNeuralTrainerTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionNeuralModel\.cpp")
{
    throw "CompositionNeuralModel.cpp is missing from the music target."
}

if ($ModelHeader -notmatch "contextResidualWeights")
{
    throw "Context residual parameter storage is missing."
}

if ($ModelHeader -notmatch "Learned direct context-to-output residual path")
{
    throw "Residual architecture contract comment is missing."
}

if ($Model -notmatch "contextResidualWeights")
{
    throw "Residual weights are not used by inference."
}

if ($Model -notmatch "assign\(\s*contract\.sectionInputWidth")
{
    throw "Residual matrix initialization is missing."
}

if ($Trainer -notmatch "residualGradient")
{
    throw "Residual gradient path is missing."
}

if ($Trainer -notmatch "adam\.residualM")
{
    throw "Residual Adam state is missing."
}

if ($Trainer -notmatch "model\.contextResidualWeights")
{
    throw "Residual parameters are not optimized."
}

foreach ($Case in @(
    "testContextResidualShape",
    "testTrainingChangesModelParameters"
))
{
    if ($ModelTests -notmatch $Case -and $TrainerTests -notmatch $Case)
    {
        throw "Required residual regression test is missing: $Case"
    }
}


if ($ModelTests -notmatch "contextResidualWeights")
{
    throw "Model tests do not cover residual parameter initialization."
}

if ($TrainerTests -notmatch "contextResidualWeights")
{
    throw "Trainer tests do not cover residual parameter determinism/update."
}

Write-Host "Phase 49 contextual residual model validation passed." -ForegroundColor Green
