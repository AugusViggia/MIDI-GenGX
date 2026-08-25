$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionNeuralModelArtifact.h",
    "Source\Music\CompositionNeuralModelArtifact.cpp",
    "Source\Tests\CompositionNeuralModelArtifactTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 60 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionNeuralModelArtifact.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionNeuralModelArtifact.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionNeuralModelArtifactTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionNeuralModelArtifact\.cpp")
{
    throw "CompositionNeuralModelArtifact.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionNeuralModelArtifactTests")
{
    throw "CompositionNeuralModelArtifactTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionNeuralModelArtifactTests")
{
    throw "build-x64.ps1 does not execute CompositionNeuralModelArtifactTests."
}

if ($Header -notmatch "serializeCompositionNeuralModel")
{
    throw "Neural model serialization API is missing."
}

if ($Header -notmatch "deserializeCompositionNeuralModel")
{
    throw "Neural model deserialization API is missing."
}

if ($Impl -notmatch "CompositionNeuralModelArtifact::magic")
{
    throw "Artifact magic validation is missing."
}

if ($Impl -notmatch "CompositionNeuralModelArtifact::version")
{
    throw "Artifact version validation is missing."
}

if ($Impl -notmatch "contextResidualWeights")
{
    throw "Artifact serialization does not include residual model parameters."
}

foreach ($Case in @(
    "testValidSerialization",
    "testSerializedParameterCountMatchesModelShape",
    "testRoundTripPreservesModel",
    "testRoundTripPreservesPrediction",
    "testCorruptedHeaderIsRejected",
    "testTruncatedPayloadIsRejected",
    "testInvalidTargetModelIsNotSerialized",
    "testDeterministicSerialization"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 60 test is missing: $Case"
    }
}


if ($Impl -match '(?<!CompositionNeuralModelArtifact::)\bmagic\b')
{
    throw "Unqualified artifact magic reference detected."
}

if ($Impl -match '(?<!CompositionNeuralModelArtifact::)\bversion\b')
{
    throw "Unqualified artifact version reference detected."
}


if ($Impl -notmatch '\(contract\.globalInputWidth\s*\+\s*contract\.sectionInputWidth\)')
{
    throw "Neural artifact loader does not reconstruct the complete model input width."
}

Write-Host "Phase 60 neural model artifact validation passed." -ForegroundColor Green
