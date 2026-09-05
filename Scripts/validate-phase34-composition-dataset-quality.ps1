$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionDatasetQuality.h",
    "Source\Music\CompositionDatasetQuality.cpp",
    "Source\Tests\CompositionDatasetQualityTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 34 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDatasetQuality.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDatasetQuality.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionDatasetQualityTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionDatasetQuality\.cpp")
{
    throw "CompositionDatasetQuality.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionDatasetQualityTests")
{
    throw "CompositionDatasetQualityTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionDatasetQualityTests")
{
    throw "build-x64.ps1 does not execute CompositionDatasetQualityTests."
}

if ($Header -notmatch "struct CompositionDatasetQuality")
{
    throw "Dataset quality model is missing."
}

if ($Impl -notmatch "assessCompositionDatasetQuality")
{
    throw "Dataset quality assessment is missing."
}

if ($Impl -notmatch "globalFeatureWidth")
{
    throw "Global feature-width validation is missing."
}

if ($Impl -notmatch "sectionFeatureWidth")
{
    throw "Section feature-width validation is missing."
}

foreach ($Case in @(
    "testQualityOfValidDataset",
    "testSchemaWidthsAreStable",
    "testEmptyDatasetQuality",
    "testInvalidDatasetIsRejected",
    "testFeatureWidthDriftIsRejected",
    "testSectionWidthDriftIsRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required dataset-quality test is missing: $Case"
    }
}

Write-Host "Phase 34 composition-dataset-quality validation passed." -ForegroundColor Green
