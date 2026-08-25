$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionComposerKnowledgeCorpusAssembly.h",
    "Source\Music\CompositionComposerKnowledgeCorpusAssembly.cpp",
    "Source\Tests\CompositionComposerKnowledgeCorpusAssemblyTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 102 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Implementation = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionComposerKnowledgeCorpusAssembly.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionComposerKnowledgeCorpusAssemblyTests.cpp")

foreach ($Required in @(
    "CompositionComposerKnowledgeCorpusAssembly.h",
    "CompositionComposerKnowledgeCorpusAssembly.cpp"
))
{
    if ($CMake -notmatch [regex]::Escape("Source/Music/$Required"))
    {
        throw "Phase 102 source is not part of MIDI_GenGX_Music: $Required"
    }
}

if ($CMake -notmatch "MIDI_GenGX_CompositionComposerKnowledgeCorpusAssemblyTests")
{
    throw "Phase 102 test target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionComposerKnowledgeCorpusAssemblyTests")
{
    throw "build-x64.ps1 does not execute Phase 102 tests."
}

foreach ($Required in @(
    "assembleCompositionComposerKnowledgeCorpus",
    "sourceManifest",
    "trainingCorpus",
    "sourceIntegrityVerified",
    "sampleId"
))
{
    if ($Implementation -notmatch [regex]::Escape($Required))
    {
        throw "Phase 102 assembly component is missing: $Required"
    }
}

foreach ($Case in @(
    "testRealSourceAndTrainingCorpusAssemble",
    "testMissingTrainingSampleFailsClosed",
    "testUnknownSourceSampleFailsClosed"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 102 test is missing: $Case"
    }
}

Write-Host "Phase 102 composer knowledge corpus assembly validation passed." -ForegroundColor Green
