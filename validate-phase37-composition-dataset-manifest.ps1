$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionDatasetManifest.h",
    "Source\Music\CompositionDatasetManifest.cpp",
    "Source\Tests\CompositionDatasetManifestTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 37 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDatasetManifest.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDatasetManifest.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionDatasetManifestTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionDatasetManifest\.cpp")
{
    throw "CompositionDatasetManifest.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionDatasetManifestTests")
{
    throw "CompositionDatasetManifestTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionDatasetManifestTests")
{
    throw "build-x64.ps1 does not execute CompositionDatasetManifestTests."
}

if ($Header -notmatch "struct CompositionDatasetManifest")
{
    throw "Dataset manifest model is missing."
}

if ($Impl -notmatch "if \(!dataset\.samples\.empty\(\)")
{
    throw "Manifest builder does not explicitly handle the empty-dataset schema case."
}

if ($Impl -notmatch "calculateDatasetSignature")
{
    throw "Dataset signature calculation is missing."
}

if ($Impl -notmatch "schemaVersion")
{
    throw "Manifest schema version is missing."
}

foreach ($Case in @(
    "testManifestValidity",
    "testStableSignature",
    "testSampleChangeChangesSignature",
    "testInvalidQualityProducesInvalidManifest",
    "testEmptyDatasetManifest",
    "testSchemaWidthsAreCarried"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required dataset-manifest test is missing: $Case"
    }
}


$Motif = Get-Content -Raw (Join-Path $Root "Source\Music\Motif.cpp")
$EngineTests = Get-Content -Raw (Join-Path $Root "Source\Tests\MusicalEngineTests.cpp")
$ManifestImpl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDatasetManifest.cpp")
$SchemaHeader = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDatasetSchema.h")
$PartitionHeader = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionDatasetPartition.h")

if ($Motif -match 'static_cast<void>\(midiPitch\)')
{
    throw "Motif.cpp still contains the unused-parameter suppression patch."
}

if ($EngineTests -match 'static_cast<void>\(composerPhrase\)')
{
    throw "MusicalEngineTests still contains the dead-result suppression patch."
}

if ($ManifestImpl -match '#include <array>')
{
    throw "CompositionDatasetManifest.cpp still contains the unused array include."
}

if ($SchemaHeader -match '#include <string>')
{
    throw "CompositionDatasetSchema.h still contains an unused string include."
}

if ($PartitionHeader -match '#include "CompositionDatasetQuality.h"')
{
    throw "CompositionDatasetPartition.h still uses the unnecessary transitive dependency."
}

Write-Host "Phase 37 composition-dataset-manifest validation passed." -ForegroundColor Green
