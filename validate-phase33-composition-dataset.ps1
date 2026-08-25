$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionDataset.h",
    "Source\Music\CompositionDataset.cpp",
    "Source\Tests\CompositionDatasetTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 33 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDataset.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDataset.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionDatasetTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionDataset\.cpp")
{
    throw "CompositionDataset.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionDatasetTests")
{
    throw "CompositionDatasetTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionDatasetTests")
{
    throw "build-x64.ps1 does not execute CompositionDatasetTests."
}

if ($Header -notmatch "struct CompositionDataset")
{
    throw "Composition dataset model is missing."
}

if ($Header -notmatch "schemaVersion = 1")
{
    throw "Dataset schema version is missing."
}

if ($Impl -notmatch "buildCompositionDataset")
{
    throw "Composition dataset builder is missing."
}

if ($Impl -notmatch "lower_bound")
{
    throw "Dataset lookup is not deterministic ordered lookup."
}

foreach ($Case in @(
    "testDatasetValidityAndOrdering",
    "testStableLookup",
    "testDuplicateIdsAreCollapsedDeterministically",
    "testDatasetStatistics",
    "testInvalidSamplesAreExcluded",
    "testEmptyDatasetIsValid"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required dataset test is missing: $Case"
    }
}

Write-Host "Phase 33 composition-dataset validation passed." -ForegroundColor Green
