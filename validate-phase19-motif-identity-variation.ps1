$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$Header = Get-Content -Raw (Join-Path $Root "Source\Music\MotifDevelopment.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\MotifDevelopment.cpp")
$PlanHeader = Get-Content -Raw (Join-Path $Root "Source\Music\PhraseDevelopmentPlan.h")
$Plan = Get-Content -Raw (Join-Path $Root "Source\Music\PhraseDevelopmentPlan.cpp")
$Composer = Get-Content -Raw (Join-Path $Root "Source\Music\MotifPhraseComposer.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\MusicalEngineTests.cpp")

if ($Header -notmatch "Motif\s+varyIntervals\s*\(")
{
    throw "MotifDevelopment::varyIntervals declaration is missing."
}

if ($Impl -notmatch "MotifDevelopment::varyIntervals")
{
    throw "MotifDevelopment::varyIntervals implementation is missing."
}

if ($PlanHeader -notmatch "motifVariationAmount")
{
    throw "PhraseDevelopmentPlan lacks explicit motif variation amount."
}

if ($Plan -notmatch "motifVariationAmount")
{
    throw "PhraseDevelopmentPlan does not assign motif variation."
}

if ($Composer -notmatch "MotifDevelopment::varyIntervals")
{
    throw "MotifPhraseComposer does not consume motif identity variation."
}

if ($Tests -notmatch "testMotifIdentityVariation")
{
    throw "Motif identity variation regression coverage is missing."
}

Write-Host "Phase 19 motif-identity variation validation passed." -ForegroundColor Green
