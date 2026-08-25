$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedSequenceNeuralModelArtifact.h")
$Implementation = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedSequenceNeuralModelArtifact.cpp")
$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionConditionedSequenceNeuralModelArtifactTests.cpp")

if ($Header -notmatch "version = 2")
{
    throw "Phase 110 conditioned model artifact version was not advanced."
}

foreach ($Required in @(
    "appendString",
    "readString",
    "stringTableBytes",
    "model.vocabulary.composers",
    "vocabulary.composers[index]",
    "vocabulary.styles[index]",
    "vocabulary.eras[index]",
    "vocabulary.instrumentations[index]"
))
{
    if ($Implementation -notmatch [regex]::Escape($Required))
    {
        throw "Phase 110 artifact vocabulary persistence is missing: $Required"
    }
}

if ($Implementation -match 'category_"')
{
    throw "Phase 110 still reconstructs fake category_N vocabulary names."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionConditionedSequenceNeuralModelArtifactTests")
{
    throw "Phase 110 artifact test target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionConditionedSequenceNeuralModelArtifactTests")
{
    throw "build-x64.ps1 does not execute Phase 110 artifact tests."
}

foreach ($Required in @(
    "testVocabularyRoundTrip",
    "testArtifactIsDeterministic",
    "testTruncatedArtifactFails"
))
{
    if ($Tests -notmatch $Required)
    {
        throw "Required Phase 110 test is missing: $Required"
    }
}


if ($Implementation -notmatch "array<std::vector<std::string>, 4>")
{
    throw "Phase 110 vocabulary serialization does not use a fixed four-category container."
}


if ($Implementation -notmatch "const auto stringOffset" -or
    $Implementation -notmatch "std::size_t parameterOffset")
{
    throw "Phase 110 deserializer does not keep parameter and vocabulary regions separate."
}

if ($Tests -notmatch "testParametersAndVocabularyUseDistinctRegions")
{
    throw "Phase 110 region-separation regression test is missing."
}

Write-Host "Phase 110 conditioned model artifact vocabulary validation passed." -ForegroundColor Green
