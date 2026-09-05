$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionDatasetSchema.h",
    "Source\Music\CompositionDatasetSchema.cpp",
    "Source\Tests\CompositionDatasetSchemaTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 36 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDatasetSchema.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDatasetSchema.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionDatasetSchemaTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionDatasetSchema\.cpp")
{
    throw "CompositionDatasetSchema.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionDatasetSchemaTests")
{
    throw "CompositionDatasetSchemaTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionDatasetSchemaTests")
{
    throw "build-x64.ps1 does not execute CompositionDatasetSchemaTests."
}

if ($Header -notmatch "struct CompositionDatasetSchema")
{
    throw "Dataset schema model is missing."
}

if ($Header -notmatch "globalFeatureCount = 13")
{
    throw "Global schema width contract is missing."
}

if ($Header -notmatch "sectionFeatureCount = 6")
{
    throw "Section schema width contract is missing."
}

if ($Impl -notmatch "validateSample")
{
    throw "Schema sample validation is missing."
}

foreach ($Case in @(
    "testSchemaConstants",
    "testFeatureIndicesAreStable",
    "testFeatureNamesAreStable",
    "testValidSampleMatchesSchema",
    "testGlobalWidthDriftIsRejected",
    "testSectionWidthDriftIsRejected",
    "testSchemaVersionContractIsStable"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required dataset-schema test is missing: $Case"
    }
}

Write-Host "Phase 36 composition-dataset-schema validation passed." -ForegroundColor Green
