$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionRuntimeInferenceService.h",
    "Source\Music\CompositionRuntimeInferenceService.cpp",
    "Source\Tests\CompositionRuntimeInferenceServiceTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 59 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionRuntimeInferenceService.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionRuntimeInferenceService.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionRuntimeInferenceServiceTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionRuntimeInferenceService\.cpp")
{
    throw "CompositionRuntimeInferenceService.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionRuntimeInferenceServiceTests")
{
    throw "CompositionRuntimeInferenceServiceTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionRuntimeInferenceServiceTests")
{
    throw "build-x64.ps1 does not execute CompositionRuntimeInferenceServiceTests."
}

if ($Header -notmatch "CompositionRuntimeInferenceService")
{
    throw "Runtime inference service is missing."
}

if ($Impl -notmatch "featureAdapter\.build")
{
    throw "Runtime inference service is not using the runtime feature adapter."
}

if ($Impl -notmatch "requestAdapter\.buildRequest")
{
    throw "Runtime inference service is not using the runtime request adapter."
}

if ($Impl -notmatch "pipeline\.infer")
{
    throw "Runtime inference service is not connected to the inference pipeline."
}

foreach ($Case in @(
    "testServiceBuildsFromPipeline",
    "testMusicalContextReachesNeuralInference",
    "testContextChangesReachInference",
    "testInferenceIsDeterministic",
    "testInvalidPipelineIsRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 59 test is missing: $Case"
    }
}

Write-Host "Phase 59 runtime inference service validation passed." -ForegroundColor Green
