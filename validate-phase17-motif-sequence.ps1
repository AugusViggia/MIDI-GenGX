$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$Header = Get-Content -Raw (
    Join-Path $Root "Source\Music\MotifDevelopment.h"
)

$Implementation = Get-Content -Raw (
    Join-Path $Root "Source\Music\MotifDevelopment.cpp"
)

$PlanHeader = Get-Content -Raw (
    Join-Path $Root "Source\Music\PhraseDevelopmentPlan.h"
)

$Plan = Get-Content -Raw (
    Join-Path $Root "Source\Music\PhraseDevelopmentPlan.cpp"
)

$Composer = Get-Content -Raw (
    Join-Path $Root "Source\Music\MotifPhraseComposer.cpp"
)

$Tests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\MusicalEngineTests.cpp"
)

if ($Header -notmatch "Motif\s+sequence\s*\(")
{
    throw "MotifDevelopment::sequence declaration is missing."
}

if ($Implementation -notmatch "MotifDevelopment::sequence")
{
    throw "MotifDevelopment::sequence implementation is missing."
}

if ($PlanHeader -notmatch "sequenceRepetitions" -or
    $PlanHeader -notmatch "sequenceStepSemitones")
{
    throw "PhraseDevelopmentPlan lacks sequence parameters."
}

if ($Plan -notmatch "sequenceRepetitions" -or
    $Plan -notmatch "sequenceStepSemitones")
{
    throw "PhraseDevelopmentPlan does not assign sequence parameters."
}

if ($Composer -notmatch "MotifDevelopment::sequence")
{
    throw "MotifPhraseComposer does not consume sequence development."
}

if ($Composer -notmatch "1\.0\s*/")
{
    throw "Sequence is not normalized back into its phrase slot."
}

if ($Tests -notmatch "testMotifSequenceDevelopment")
{
    throw "Motif sequence regression tests are missing."
}

Write-Host "Phase 17 motif-sequence validation passed." -ForegroundColor Green
