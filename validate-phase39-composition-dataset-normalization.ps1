$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionDatasetNormalization.h",
    "Source\Music\CompositionDatasetNormalization.cpp",
    "Source\Tests\CompositionDatasetNormalizationTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 39 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDatasetNormalization.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDatasetNormalization.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionDatasetNormalizationTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionDatasetNormalization\.cpp")
{
    throw "CompositionDatasetNormalization.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionDatasetNormalizationTests")
{
    throw "CompositionDatasetNormalizationTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionDatasetNormalizationTests")
{
    throw "build-x64.ps1 does not execute CompositionDatasetNormalizationTests."
}

if ($Header -notmatch "struct CompositionDatasetNormalization")
{
    throw "Dataset normalization model is missing."
}

if ($Impl -notmatch "training samples")
{
    throw "Training-only normalization contract is missing."
}

if ($Impl -notmatch "fitCompositionDatasetNormalization")
{
    throw "Normalization fitting is missing."
}

if ($Impl -notmatch "applyCompositionDatasetNormalization")
{
    throw "Normalization application is missing."
}

foreach ($Case in @(
    "testTrainingOnlyFit",
    "testNoDataLeakageIntoStatistics",
    "testNormalizationProducesFiniteValues",
    "testPaddingRemainsZeroAndMasked",
    "testConstantFeaturesUseStableUnitScale",
    "testInvalidInputsAreRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required dataset-normalization test is missing: $Case"
    }
}

Write-Host "Phase 39 composition-dataset-normalization validation passed." -ForegroundColor Green
