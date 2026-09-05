$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionTrainingCorpusArtifact.h",
    "Source\Music\CompositionTrainingCorpusArtifact.cpp",
    "Source\Tests\CompositionTrainingCorpusArtifactTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 68 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionTrainingCorpusArtifact.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionTrainingCorpusArtifactTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionTrainingCorpusArtifact\.cpp")
{
    throw "CompositionTrainingCorpusArtifact.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionTrainingCorpusArtifactTests")
{
    throw "CompositionTrainingCorpusArtifactTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionTrainingCorpusArtifactTests")
{
    throw "build-x64.ps1 does not execute CompositionTrainingCorpusArtifactTests."
}

if ($Impl -notmatch "CompositionDataset::schemaVersion")
{
    throw "Training corpus artifact is not schema-versioned."
}

if ($Impl -notmatch "CompositionDatasetBatch::batchVersion")
{
    throw "Training corpus artifact is not batch-versioned."
}

if ($Impl -notmatch "globalMatrix")
{
    throw "Training corpus artifact does not store global features."
}

if ($Impl -notmatch "sectionMatrix")
{
    throw "Training corpus artifact does not store section features."
}

if ($Impl -notmatch "sectionMask")
{
    throw "Training corpus artifact does not store section masks."
}

foreach ($Case in @(
    "testValidCorpusArtifact",
    "testCorpusRoundTripPreservesMatrices",
    "testCorpusShapeIsPreserved",
    "testCorruptedCorpusArtifactIsRejected",
    "testTruncatedCorpusArtifactIsRejected",
    "testDeterministicCorpusArtifact",
    "testInvalidPreparedViewIsRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 68 test is missing: $Case"
    }
}


foreach ($Line in ($Impl -split "`r?`n"))
{
    $Trimmed = $Line.Trim()

    if ($Trimmed -match '(?<!CompositionTrainingCorpusArtifact::)\bmagic\b')
    {
        throw "Unqualified training corpus artifact magic reference detected."
    }

    if ($Trimmed -match '(?<!CompositionTrainingCorpusArtifact::)\bversion\b')
    {
        throw "Unqualified training corpus artifact version reference detected."
    }
}

Write-Host "Phase 68 training corpus artifact validation passed." -ForegroundColor Green
