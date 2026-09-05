$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionDatasetBatch.h",
    "Source\Music\CompositionDatasetBatch.cpp",
    "Source\Tests\CompositionDatasetBatchTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 38 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDatasetBatch.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDatasetBatch.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionDatasetBatchTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionDatasetBatch\.cpp")
{
    throw "CompositionDatasetBatch.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionDatasetBatchTests")
{
    throw "CompositionDatasetBatchTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionDatasetBatchTests")
{
    throw "build-x64.ps1 does not execute CompositionDatasetBatchTests."
}

if ($Header -notmatch "struct CompositionDatasetBatch")
{
    throw "Composition dataset batch model is missing."
}

if ($Header -notmatch "globalMatrix")
{
    throw "Global matrix representation is missing."
}

if ($Header -notmatch "sectionMask")
{
    throw "Section mask representation is missing."
}

if ($Impl -notmatch "buildCompositionDatasetBatch")
{
    throw "Composition dataset batch builder is missing."
}

foreach ($Case in @(
    "testBatchShape",
    "testSectionMask",
    "testPaddingIsZero",
    "testDeterministicBatchEncoding",
    "testInvalidManifestIsRejected",
    "testEmptyDatasetProducesEmptyBatch"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required dataset-batch test is missing: $Case"
    }
}


if ($Impl -notmatch 'featureIndex\s*<\s*batch\.sectionFeatureWidth')
{
    throw "Section feature loop must use batch.sectionFeatureWidth."
}


if ($Impl -notmatch 'batch\.analysisValid = true')
{
    throw "Batch builder does not mark valid inputs as analyzed before empty-dataset handling."
}

if ($Impl -notmatch 'if \(dataset\.size\(\) == 0\)')
{
    throw "Batch builder does not explicitly handle the empty-dataset case."
}

Write-Host "Phase 38 composition-dataset-batch validation passed." -ForegroundColor Green
