$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\MusicalEngine.h",
    "Source\Music\MusicalEngine.cpp",
    "Source\Tests\MusicalEngineAIGuidanceTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 53 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\MusicalEngine.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\MusicalEngine.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\MusicalEngineAIGuidanceTests.cpp")

if ($CMake -notmatch "MIDI_GenGX_MusicalEngineAIGuidanceTests")
{
    throw "MusicalEngineAIGuidanceTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_MusicalEngineAIGuidanceTests")
{
    throw "build-x64.ps1 does not execute MusicalEngineAIGuidanceTests."
}

if ($Header -notmatch "generateWithAIGuidance")
{
    throw "AI-guided MusicalEngine entry point is missing."
}

if ($Impl -notmatch "generateWithAIGuidance")
{
    throw "AI-guided MusicalEngine implementation is missing."
}

if ($Impl -notmatch "guidedContext")
{
    throw "AI guidance is not applied through a guided context."
}

if ($Tests -notmatch "testDefaultGenerationIsUnchanged")
{
    throw "Default-generation regression test is missing."
}

foreach ($Case in @(
    "testValidAIGuidanceProducesPhrase",
    "testInvalidGuidanceFallsBackSafely",
    "testDefaultGenerationIsUnchanged",
    "testAIGuidanceIsDeterministic",
    "testExplicitContextConstraintsRemainAuthoritative"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 53 test is missing: $Case"
    }
}


if ($Impl -match "harmonicComplexity")
{
    throw "MusicalEngine AI guidance still references the removed harmonicComplexity parameter."
}

if ($Tests -match "harmonicComplexity")
{
    throw "AI guidance tests still reference the removed harmonicComplexity parameter."
}

if ($Impl -notmatch "parameters\.complexity")
{
    throw "AI harmony guidance must map through the existing complexity parameter."
}

Write-Host "Phase 53 MusicalEngine AI guidance validation passed." -ForegroundColor Green
