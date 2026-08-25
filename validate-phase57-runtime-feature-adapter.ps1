$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionRuntimeFeatureAdapter.h",
    "Source\Music\CompositionRuntimeFeatureAdapter.cpp",
    "Source\Tests\CompositionRuntimeFeatureAdapterTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 57 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionRuntimeFeatureAdapter.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionRuntimeFeatureAdapter.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionRuntimeFeatureAdapterTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionRuntimeFeatureAdapter\.cpp")
{
    throw "CompositionRuntimeFeatureAdapter.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionRuntimeFeatureAdapterTests")
{
    throw "CompositionRuntimeFeatureAdapterTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionRuntimeFeatureAdapterTests")
{
    throw "build-x64.ps1 does not execute CompositionRuntimeFeatureAdapterTests."
}

if ($Header -notmatch "CompositionRuntimeFeatures")
{
    throw "Runtime feature representation is missing."
}

if ($Impl -notmatch "CompositionDatasetSchema::globalFeatureCount")
{
    throw "Runtime adapter is not coupled to global dataset schema width."
}

if ($Impl -notmatch "CompositionDatasetSchema::sectionFeatureCount")
{
    throw "Runtime adapter is not coupled to section dataset schema width."
}

if ($Impl -notmatch "TensionDeltaNormalized")
{
    throw "Runtime section delta handling is missing."
}

foreach ($Case in @(
    "testRuntimeFeatureShape",
    "testRuntimeFeaturesAreNormalized",
    "testContextChangesPropagate",
    "testUnavailableRuntimeKnowledgeStaysNeutral",
    "testExtremeContextRemainsSafe",
    "testAdapterIsDeterministic"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 57 test is missing: $Case"
    }
}

Write-Host "Phase 57 runtime feature adapter validation passed." -ForegroundColor Green
