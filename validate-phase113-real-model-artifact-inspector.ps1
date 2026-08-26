$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$Implementation = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedSequenceNeuralModelArtifactFileInspector.cpp")
$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionConditionedSequenceNeuralModelArtifactFileInspectorTests.cpp")

foreach ($Required in @(
    "inspectCompositionConditionedSequenceNeuralModelArtifactFile",
    "artifactVersion",
    "composerSummary",
    "styleSummary",
    "eraSummary",
    "instrumentationSummary"
))
{
    if ($Implementation -notmatch [regex]::Escape($Required))
    {
        throw "Phase 113 artifact inspector component is missing: $Required"
    }
}

if ($Implementation -notmatch "CompositionConditionedSequenceNeuralModelRuntimeLoader")
{
    throw "Phase 113 inspector is not routed through the Phase 112 runtime loader."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionConditionedSequenceNeuralModelArtifactFileInspectorTests")
{
    throw "Phase 113 artifact inspector test target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionConditionedSequenceNeuralModelArtifactFileInspectorTests")
{
    throw "build-x64.ps1 does not execute Phase 113 artifact inspector tests."
}

foreach ($Required in @(
    "testArtifactFileRoundTrip",
    "testMissingArtifactFailsClosed"
))
{
    if ($Tests -notmatch $Required)
    {
        throw "Required Phase 113 test is missing: $Required"
    }
}

Write-Host "Phase 113 real model artifact inspector validation passed." -ForegroundColor Green
