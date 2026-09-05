$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionAIConstraintAdapter.h",
    "Source\Music\CompositionAIConstraintAdapter.cpp",
    "Source\Tests\CompositionAIConstraintAdapterTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 51 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionAIConstraintAdapter.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionAIConstraintAdapter.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionAIConstraintAdapterTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionAIConstraintAdapter\.cpp")
{
    throw "CompositionAIConstraintAdapter.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionAIConstraintAdapterTests")
{
    throw "CompositionAIConstraintAdapterTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionAIConstraintAdapterTests")
{
    throw "build-x64.ps1 does not execute CompositionAIConstraintAdapterTests."
}

if ($Header -notmatch "struct CompositionAIConstraintProfile")
{
    throw "AI musical constraint profile is missing."
}

if ($Impl -notmatch "adaptAIResultToMusicalConstraints")
{
    throw "AI-to-musical constraint adapter is missing."
}

if ($Impl -notmatch "tensionDeltaTarget")
{
    throw "Tension constraint mapping is missing."
}

if ($Impl -notmatch "harmonyDegreeTarget")
{
    throw "Harmony constraint mapping is missing."
}

foreach ($Case in @(
    "testAdapterBuildsValidConstraintProfile",
    "testAllMusicalTargetsAreMapped",
    "testAdapterIsDeterministic",
    "testInvalidRequestIsRejected",
    "testInvalidPipelineIsRejected",
    "testTargetsRemainInsideNormalizedRange"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required AI-constraint test is missing: $Case"
    }
}

Write-Host "Phase 51 AI-constraint-adapter validation passed." -ForegroundColor Green
