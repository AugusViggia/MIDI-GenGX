$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionDatasetSample.h",
    "Source\Music\CompositionDatasetSample.cpp",
    "Source\Tests\CompositionDatasetSampleTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 32 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDatasetSample.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDatasetSample.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionDatasetSampleTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionDatasetSample\.cpp")
{
    throw "CompositionDatasetSample.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionDatasetSampleTests")
{
    throw "CompositionDatasetSampleTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionDatasetSampleTests")
{
    throw "build-x64.ps1 does not execute CompositionDatasetSampleTests."
}

if ($Header -notmatch "struct CompositionDatasetSample")
{
    throw "Composition dataset sample model is missing."
}

if ($Header -notmatch "schemaVersion = 1")
{
    throw "Dataset schema version is missing."
}

if ($Impl -notmatch "buildCompositionDatasetSample")
{
    throw "Composition dataset sample builder is missing."
}

if ($Impl -notmatch "globalFeatures")
{
    throw "Global feature vector is missing."
}

if ($Impl -notmatch "sectionFeatures")
{
    throw "Section feature vectors are missing."
}

foreach ($Case in @(
    "testSchemaShape",
    "testFeaturesAreNormalized",
    "testDeterministicFeatureOrder",
    "testFirstSectionHasZeroIncomingDelta",
    "testInvalidSnapshotProducesInvalidSample",
    "testEmptySampleIdIsRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required dataset-sample test is missing: $Case"
    }
}

Write-Host "Phase 32 composition-dataset-sample validation passed." -ForegroundColor Green
