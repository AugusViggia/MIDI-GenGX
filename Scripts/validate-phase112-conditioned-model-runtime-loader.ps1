$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedSequenceNeuralModelRuntimeLoader.h")
$Implementation = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedSequenceNeuralModelRuntimeLoader.cpp")
$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionConditionedSequenceNeuralModelRuntimeLoaderTests.cpp")

foreach ($Required in @(
    "load(",
    "embeddedBytes",
    "deserializeCompositionConditionedSequenceNeuralModel",
    "artifact.isValid()",
    "model.isValid()"
))
{
    if ($Implementation -notmatch [regex]::Escape($Required))
    {
        throw "Phase 112 runtime loader component is missing: $Required"
    }
}

if ($Header -notmatch "CompositionConditionedSequenceNeuralModelRuntimeLoader")
{
    throw "Phase 112 runtime loader class is missing."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionConditionedSequenceNeuralModelRuntimeLoaderTests")
{
    throw "Phase 112 runtime loader test target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionConditionedSequenceNeuralModelRuntimeLoaderTests")
{
    throw "build-x64.ps1 does not execute Phase 112 runtime loader tests."
}

foreach ($Required in @(
    "testEmbeddedVectorLoad",
    "testRawResourceLoad",
    "testInvalidResourceFailsClosed",
    "testNullResourceFailsClosed"
))
{
    if ($Tests -notmatch $Required)
    {
        throw "Required Phase 112 test is missing: $Required"
    }
}

Write-Host "Phase 112 conditioned model runtime loader validation passed." -ForegroundColor Green
