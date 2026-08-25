$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionRuntimeInferenceAdapter.h",
    "Source\Music\CompositionRuntimeInferenceAdapter.cpp",
    "Source\Tests\CompositionRuntimeInferenceAdapterTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 58 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionRuntimeInferenceAdapter.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionRuntimeInferenceAdapter.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionRuntimeInferenceAdapterTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionRuntimeInferenceAdapter\.cpp")
{
    throw "CompositionRuntimeInferenceAdapter.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionRuntimeInferenceAdapterTests")
{
    throw "CompositionRuntimeInferenceAdapterTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionRuntimeInferenceAdapterTests")
{
    throw "build-x64.ps1 does not execute CompositionRuntimeInferenceTests."
}

if ($Header -notmatch "CompositionRuntimeInferenceAdapter")
{
    throw "Runtime inference adapter is missing."
}

if ($Impl -notmatch "CompositionInferenceRequest")
{
    throw "Runtime inference request construction is missing."
}

if ($Impl -notmatch "features\.globalFeatures")
{
    throw "Global runtime features are not copied into the inference request."
}

if ($Impl -notmatch "features\.sectionFeatures")
{
    throw "Section runtime features are not copied into the inference request."
}

foreach ($Case in @(
    "testValidFeaturesBecomeValidRequest",
    "testFeatureValuesArePreserved",
    "testInvalidFeaturesAreRejected",
    "testContractMismatchIsRejected",
    "testOutputRequestIsDeterministic",
    "testRequestDimensionsMatchContract"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 58 test is missing: $Case"
    }
}

Write-Host "Phase 58 runtime inference adapter validation passed." -ForegroundColor Green
