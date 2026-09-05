$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionDatasetPartition.h",
    "Source\Music\CompositionDatasetPartition.cpp",
    "Source\Tests\CompositionDatasetPartitionTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 35 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDatasetPartition.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDatasetPartition.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionDatasetPartitionTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionDatasetPartition\.cpp")
{
    throw "CompositionDatasetPartition.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionDatasetPartitionTests")
{
    throw "CompositionDatasetPartitionTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionDatasetPartitionTests")
{
    throw "build-x64.ps1 does not execute CompositionDatasetPartitionTests."
}

if ($Header -notmatch "struct CompositionDatasetPartition")
{
    throw "Dataset partition model is missing."
}

if ($Impl -notmatch "stableHash")
{
    throw "Stable partition hashing is missing."
}

if ($Impl -notmatch "buildCompositionDatasetPartition")
{
    throw "Dataset partition builder is missing."
}

if ($Impl -notmatch "RankedSample")
{
    throw "Deterministic ranked partition strategy is missing."
}

if ($Impl -notmatch "sampleCount >= activeSplits")
{
    throw "Small-dataset split population policy is missing."
}

foreach ($Case in @(
    "testPartitionCoversDatasetExactlyOnce",
    "testPartitionIsDeterministic",
    "testPartitionIsIndependentOfInputConstructionOrder",
    "testInvalidRatiosAreRejected",
    "testInvalidDatasetIsRejected",
    "testPartitionRatiosProduceExpectedCounts",
    "testSmallDatasetKeepsRequestedSplitsPopulated",
    "testZeroRatioDisablesSplit"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required dataset-partition test is missing: $Case"
    }
}

Write-Host "Phase 35 composition-dataset-partition validation passed." -ForegroundColor Green
