$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionNeuralModelRuntimeLoader.h",
    "Source\Music\CompositionNeuralModelRuntimeLoader.cpp",
    "Source\Tests\CompositionNeuralModelRuntimeLoaderTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 61 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionNeuralModelRuntimeLoader.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionNeuralModelRuntimeLoader.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionNeuralModelRuntimeLoaderTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionNeuralModelRuntimeLoader\.cpp")
{
    throw "CompositionNeuralModelRuntimeLoader.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionNeuralModelRuntimeLoaderTests")
{
    throw "CompositionNeuralModelRuntimeLoaderTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionNeuralModelRuntimeLoaderTests")
{
    throw "build-x64.ps1 does not execute CompositionNeuralModelRuntimeLoaderTests."
}

if ($Header -notmatch "CompositionNeuralModelRuntimeLoader")
{
    throw "Runtime model loader is missing."
}

if ($Impl -notmatch "deserializeCompositionNeuralModel")
{
    throw "Runtime model loader is not connected to artifact deserialization."
}

if ($Impl -notmatch "CompositionNeuralModel candidate")
{
    throw "Runtime model loader does not protect the active model during load."
}

foreach ($Case in @(
    "testEmptyLoaderIsInvalid",
    "testValidArtifactLoads",
    "testLoadedModelMatchesSource",
    "testLoadedModelPreservesInference",
    "testInvalidArtifactDoesNotMutateLoadedModel",
    "testClearUnloadsModel",
    "testRepeatedLoadReplacesModelAtomically"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 61 test is missing: $Case"
    }
}

Write-Host "Phase 61 neural model runtime loader validation passed." -ForegroundColor Green
