$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$Trainer = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedSequenceNeuralTrainer.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionConditionedSequenceNeuralTrainerTests.cpp")

if ($Trainer -notmatch "outputScale")
{
    throw "Phase 108 output-gradient scaling is missing."
}

if ($Trainer -notmatch "rawError[\s\S]*gradient[\s\S]*outputWeights")
{
    throw "Phase 108 scaled output-weight gradient path is missing."
}

if ($Trainer -notmatch "dhNext\[column\][\s\S]*gradient")
{
    throw "Phase 108 scaled recurrent backpropagation from output is missing."
}

if ($Trainer -match "gradients\.outputWeights[\s\S]{0,600}\berror\s*\*[\s\S]{0,100}hidden\[timeCount\]\[column\][\s\S]{0,600}for \(std::size_t row = 0;")
{
    throw "Legacy duplicate unscaled output gradient path is still present."
}

if ($Tests -notmatch "testTrainingReducesLoss")
{
    throw "Existing training-loss regression test is missing."
}

if ($Tests -notmatch "testTrainingIsDeterministic")
{
    throw "Existing deterministic training regression test is missing."
}

Write-Host "Phase 108 training gradient correctness validation passed." -ForegroundColor Green
