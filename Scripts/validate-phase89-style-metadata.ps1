$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$RequiredFiles = @(
    "Source\Music\CompositionSequenceMetadata.h",
    "Source\Music\CompositionSequenceMetadata.cpp",
    "Source\Music\CompositionSequenceMetadataCatalog.h",
    "Source\Music\CompositionSequenceMetadataCatalog.cpp",
    "Source\Music\CompositionSequenceMetadataArtifact.h",
    "Source\Music\CompositionSequenceMetadataArtifact.cpp",
    "Source\Tests\CompositionSequenceMetadataTests.cpp"
)

foreach ($RelativePath in $RequiredFiles)
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 89 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Metadata = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionSequenceMetadata.h")
$Catalog = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionSequenceMetadataCatalog.cpp")
$Artifact = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionSequenceMetadataArtifact.h")
$ArtifactImpl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionSequenceMetadataArtifact.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionSequenceMetadataTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionSequenceMetadata\.cpp" -or
    $CMake -notmatch "Source/Music/CompositionSequenceMetadataArtifact\.cpp")
{
    throw "Phase 89 metadata sources are not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionSequenceMetadataTests")
{
    throw "Phase 89 metadata test target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionSequenceMetadataTests")
{
    throw "build-x64.ps1 does not execute Phase 89 metadata tests."
}

foreach ($Required in @(
    "composerId",
    "workId",
    "movementId",
    "styleId",
    "eraId",
    "instrumentationId"
))
{
    if ($Metadata -notmatch $Required)
    {
        throw "Phase 89 metadata field is missing: $Required"
    }
}

if ($Catalog -notmatch "findBySampleId" -or
    $Catalog -notmatch "std::sort")
{
    throw "Phase 89 metadata catalog lookup/order is missing."
}

if ($Artifact -notmatch 'magic\s*=\s*0x4D47544D')
{
    throw "Phase 89 metadata artifact magic is missing."
}

if ($ArtifactImpl -notmatch "writeString" -or
    $ArtifactImpl -notmatch "readString")
{
    throw "Phase 89 metadata serialization boundary is missing."
}

foreach ($Case in @(
    "testMetadataValidity",
    "testCatalogSortsAndRejectsDuplicates",
    "testCatalogArtifactRoundTrip",
    "testUnverifiedMetadataIsNotTrainingReady",
    "testSampleLookup"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 89 test is missing: $Case"
    }
}

Write-Host "Phase 89 style metadata validation passed." -ForegroundColor Green
