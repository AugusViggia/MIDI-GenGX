$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionMidiTrainingCorpusArtifact.h",
    "Source\Music\CompositionMidiTrainingCorpusArtifact.cpp",
    "Source\Tests\CompositionMidiTrainingCorpusArtifactTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 84 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiTrainingCorpusArtifact.cpp")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiTrainingCorpusArtifact.h")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiTrainingCorpusArtifactTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionMidiTrainingCorpusArtifact\.cpp")
{
    throw "CompositionMidiTrainingCorpusArtifact.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionMidiTrainingCorpusArtifactTests")
{
    throw "CompositionMidiTrainingCorpusArtifactTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionMidiTrainingCorpusArtifactTests")
{
    throw "build-x64.ps1 does not execute CompositionMidiTrainingCorpusArtifactTests."
}

if ($Header -notmatch 'magic\s*=\s*0x4D47534D')
{
    throw "Phase 84 artifact magic is missing."
}

foreach ($Required in @(
    "serializeCompositionMidiTrainingSequences",
    "deserializeCompositionMidiTrainingSequences",
    "safeMultiply",
    "safeAdd"
))
{
    if ($Impl -notmatch $Required)
    {
        throw "Phase 84 artifact safety component is missing: $Required"
    }
}

if ($Impl -notmatch "sampleId")
{
    throw "Phase 84 artifact does not preserve sequence identity."
}

foreach ($Case in @(
    "testArtifactRoundTrip",
    "testSerializationIsDeterministic",
    "testInvalidSequencesAreRejected",
    "testCorruptedArtifactIsRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 84 test is missing: $Case"
    }
}

Write-Host "Phase 84 sequence training corpus validation passed." -ForegroundColor Green
