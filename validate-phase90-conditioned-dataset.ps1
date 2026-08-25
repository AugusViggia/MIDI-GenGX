$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$RequiredFiles = @(
    "Source\Music\CompositionConditionedTrainingSample.h",
    "Source\Music\CompositionConditioningVocabulary.h",
    "Source\Music\CompositionConditionedTrainingDataset.h",
    "Source\Music\CompositionConditionedTrainingDatasetArtifact.h",
    "Source\Tests\CompositionConditionedTrainingDatasetTests.cpp"
)

foreach ($RelativePath in $RequiredFiles)
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 90 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Dataset = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedTrainingDataset.cpp")
$Vocabulary = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditioningVocabulary.cpp")
$Artifact = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedTrainingDatasetArtifact.h")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionConditionedTrainingDatasetTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionConditionedTrainingDataset\.cpp")
{
    throw "Phase 90 conditioned dataset implementation is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionConditionedTrainingDatasetTests")
{
    throw "Phase 90 test target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionConditionedTrainingDatasetTests")
{
    throw "build-x64.ps1 does not execute Phase 90 tests."
}

foreach ($Required in @(
    "findBySampleId",
    "buildCompositionConditioningVocabulary",
    "composerIndex",
    "styleIndex",
    "eraIndex",
    "instrumentationIndex",
    "buildCompositionConditionedTrainingDataset"
))
{
    if (($Dataset + $Vocabulary) -notmatch [regex]::Escape($Required))
    {
        throw "Phase 90 conditioned dataset component is missing: $Required"
    }
}

if ($Artifact -notmatch 'magic\s*=\s*0x4D474344')
{
    throw "Phase 90 conditioned dataset artifact magic is missing."
}

foreach ($Case in @(
    "testJoinBySampleId",
    "testVocabularyIsDeterministic",
    "testMissingMetadataRejectsTrainingDataset",
    "testUnverifiedMetadataRejectsTrainingDataset",
    "testConditionManifestIsDeterministic"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 90 test is missing: $Case"
    }
}


$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedTrainingDataset.h")
$ArtifactImpl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedTrainingDatasetArtifact.cpp")

if ($Header -notmatch 'CompositionSequenceMetadataCatalog\.h')
{
    throw "Phase 90 dataset contract does not include the metadata catalog type."
}

if ($ArtifactImpl -match 'magicValue\s*!=\s*magic\b' -or
    $ArtifactImpl -match 'versionValue\s*!=\s*version\b')
{
    throw "Phase 90 artifact implementation uses unqualified class constants."
}

if ($ArtifactImpl -match 'offset,\s*magic\s*\)' -or
    $ArtifactImpl -match 'offset,\s*version\s*\)')
{
    throw "Phase 90 artifact serialization uses unqualified class constants."
}


$ArtifactImpl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedTrainingDatasetArtifact.cpp")

if ($ArtifactImpl -match "!\s*writeU32\(")
{
    throw "Phase 90 artifact serializer treats void writeU32() as a boolean operation."
}


$ArtifactImpl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedTrainingDatasetArtifact.cpp")

if ($ArtifactImpl -notmatch "groupIndex >= 2")
{
    throw "Phase 90 artifact size accounting does not distinguish fixed-header counts from inline counts."
}

if ($ArtifactImpl -match "groups\)\s*\{\s*if\s*\(group->size\(\).*\)\s*return artifact;\s*\s*if\s*\(!safeAdd\(\s*totalBytes,\s*sizeof\(std::uint32_t\)")
{
    throw "Phase 90 artifact size accounting still allocates inline count bytes for composer/style groups."
}

Write-Host "Phase 90 conditioned dataset validation passed." -ForegroundColor Green
