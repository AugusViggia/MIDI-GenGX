$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionTransitionProfile.h",
    "Source\Music\CompositionTransitionProfile.cpp",
    "Source\Tests\CompositionTransitionProfileTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 30 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionTransitionProfile.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionTransitionProfile.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionTransitionProfileTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionTransitionProfile\.cpp")
{
    throw "CompositionTransitionProfile.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionTransitionProfileTests")
{
    throw "CompositionTransitionProfileTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionTransitionProfileTests")
{
    throw "build-x64.ps1 does not execute CompositionTransitionProfileTests."
}

if ($Header -notmatch "enum class TensionTransitionKind")
{
    throw "Tension transition vocabulary is missing."
}

if ($Impl -notmatch "analyzeCompositionTransitions")
{
    throw "Composition transition analysis is missing."
}

if ($Header -notmatch "bool analysisValid")
{
    throw "Transition profile analysis-state flag is missing."
}

if ($Header -notmatch "std::size_t sectionCount")
{
    throw "Transition profile section-count state is missing."
}

if ($Impl -notmatch "profile\.analysisValid = true")
{
    throw "Valid transition analysis does not mark completion."
}

foreach ($Case in @(
    "testRisingAndFallingTransitions",
    "testFlatTransition",
    "testPeakDetection",
    "testHarmonicTransitionPreservation",
    "testInvalidGraphProducesInvalidProfile",
    "testSingleSectionComposition"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required transition-profile test is missing: $Case"
    }
}

Write-Host "Phase 30 composition-transition-profile validation passed." -ForegroundColor Green
