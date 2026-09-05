$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionInferencePipeline.h",
    "Source\Music\CompositionInferencePipeline.cpp",
    "Source\Tests\CompositionInferencePipelineTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 50 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionInferencePipeline.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionInferencePipeline.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionInferencePipelineTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionInferencePipeline\.cpp")
{
    throw "CompositionInferencePipeline.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionInferencePipelineTests")
{
    throw "CompositionInferencePipelineTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionInferencePipelineTests")
{
    throw "build-x64.ps1 does not execute CompositionInferencePipelineTests."
}

if ($Header -notmatch "struct CompositionInferencePipeline")
{
    throw "Inference pipeline model is missing."
}

if ($Header -notmatch "CompositionInferenceRequest")
{
    throw "Inference request contract is missing."
}

if ($Impl -notmatch "buildCompositionInferencePipeline")
{
    throw "Inference pipeline builder is missing."
}

if ($Impl -notmatch "predictNextSection")
{
    throw "Inference pipeline is not connected to neural prediction."
}

foreach ($Case in @(
    "testPipelineBuildsFromTrainedModel",
    "testInferenceProducesValidPrediction",
    "testInferenceIsDeterministic",
    "testInvalidRequestIsRejected",
    "testDimensionMismatchIsRejected",
    "testInvalidModelDoesNotCreateReadyPipeline"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required inference-pipeline test is missing: $Case"
    }
}

Write-Host "Phase 50 composition-inference-pipeline validation passed." -ForegroundColor Green
