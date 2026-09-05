$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Tests\AIGenerationFunctionalTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 55 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\AIGenerationFunctionalTests.cpp")

if ($CMake -notmatch "MIDI_GenGX_AIGenerationFunctionalTests")
{
    throw "AIGenerationFunctionalTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_AIGenerationFunctionalTests")
{
    throw "build-x64.ps1 does not execute AIGenerationFunctionalTests."
}

foreach ($Case in @(
    "testRoles",
    "testKeysAndScales",
    "testSeedDeterminism",
    "testDifferentSeedsRemainFunctional",
    "testAIOnVsOff",
    "testGuidanceExtremesStaySafe",
    "testTightRegisterIsSafe",
    "testInvalidAIRequestDoesNotGenerate",
    "testDisabledAIBypassesFeatureValidation",
    "testMusicalContextConstraintsRemainAuthoritative"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 55 functional test is missing: $Case"
    }
}

if ($Tests -notmatch "generateWithAIGuidance")
{
    throw "Functional tests do not exercise AI-guided MusicalEngine generation."
}

if ($Tests -notmatch "buildCompositionAIGenerationCoordinator")
{
    throw "Functional tests do not exercise the complete AI generation coordinator."
}

Write-Host "Phase 55 AI generation functional validation passed." -ForegroundColor Green
