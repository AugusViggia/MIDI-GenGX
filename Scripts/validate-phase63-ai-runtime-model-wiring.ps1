$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Plugin\AIRuntimeGeneration.h",
    "Source\Plugin\AIRuntimeGeneration.cpp",
    "Source\Tests\AIRuntimeGenerationModelTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 63 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Plugin\AIRuntimeGeneration.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Plugin\AIRuntimeGeneration.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\AIRuntimeGenerationModelTests.cpp")

if ($CMake -notmatch "MIDI_GenGX_AIRuntimeGenerationModelTests")
{
    throw "AIRuntimeGenerationModelTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_AIRuntimeGenerationModelTests")
{
    throw "build-x64.ps1 does not execute AIRuntimeGenerationModelTests."
}

if ($Header -notmatch "CompositionNeuralModelArtifact")
{
    throw "AI runtime is not aware of the model artifact."
}

if ($Header -notmatch "loadModelArtifact")
{
    throw "AI runtime model-loading API is missing."
}

if ($Impl -notmatch "CompositionAIModelRuntimeProvider")
{
    throw "AI runtime is not connected to the persistent model provider."
}

if ($Impl -notmatch "modelProvider")
{
    throw "AI runtime does not retain a loaded runtime provider."
}

if ($Impl -notmatch "currentProvider")
{
    throw "Explicit provider path is missing."
}

foreach ($Case in @(
    "testModelLoadMakesAIRuntimeReady",
    "testLoadedModelIsUsedWhenEnabled",
    "testDisabledRuntimeStillUsesBaseline",
    "testInvalidReplacementPreservesLoadedModel",
    "testClearModelRemovesRuntimeModel",
    "testExplicitProviderStillHasPriority"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 63 test is missing: $Case"
    }
}

Write-Host "Phase 63 AI runtime model wiring validation passed." -ForegroundColor Green
