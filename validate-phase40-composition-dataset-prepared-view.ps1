$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionDatasetPreparedView.h",
    "Source\Music\CompositionDatasetPreparedView.cpp",
    "Source\Tests\CompositionDatasetPreparedViewTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 40 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDatasetPreparedView.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDatasetPreparedView.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionDatasetPreparedViewTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionDatasetPreparedView\.cpp")
{
    throw "CompositionDatasetPreparedView.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionDatasetPreparedViewTests")
{
    throw "CompositionDatasetPreparedViewTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionDatasetPreparedViewTests")
{
    throw "build-x64.ps1 does not execute CompositionDatasetPreparedViewTests."
}

if ($Header -notmatch "struct CompositionDatasetPreparedView")
{
    throw "Prepared learning-view model is missing."
}

if ($Impl -notmatch "fitCompositionDatasetNormalization")
{
    throw "Prepared view does not fit normalization."
}

if ($Impl -notmatch "applyCompositionDatasetNormalization")
{
    throw "Prepared view does not apply normalization."
}

if ($Impl -notmatch "partition")
{
    throw "Prepared view does not preserve dataset partition metadata."
}

foreach ($Case in @(
    "testPreparationIsValid",
    "testNormalizationIsFittedBeforeLearningViewIsExposed",
    "testPartitionIndicesRemainAligned",
    "testPreparedDataIsDeterministic",
    "testInvalidManifestIsRejected",
    "testInvalidPartitionIsRejected",
    "testEmptyDatasetIsAValidPreparedState"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required prepared-view test is missing: $Case"
    }
}

Write-Host "Phase 40 composition-dataset-prepared-view validation passed." -ForegroundColor Green
