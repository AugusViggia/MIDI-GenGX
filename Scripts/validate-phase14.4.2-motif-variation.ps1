$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$Composer = Get-Content -Raw (
    Join-Path $Root "Source\Music\MotifPhraseComposer.cpp"
)

$Tests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\MusicalEngineTests.cpp"
)

if ($Composer -notmatch "repetition\s*>=\s*100")
{
    throw "Exact repetition control is not enforced explicitly."
}

if ($Composer -notmatch "100\s*-\s*repetition")
{
    throw "Repetition is not converted into a deterministic development level."
}

if ($Composer -notmatch "combinedDevelopment")
{
    throw "Variation/repetition development is not centralized."
}

if ($Tests -notmatch "variation/repetition controls deterministically affect motif development")
{
    throw "Deterministic variation/repetition regression test is missing."
}

Write-Host "Phase 14.4.2 variation/repetition validation passed." -ForegroundColor Green
