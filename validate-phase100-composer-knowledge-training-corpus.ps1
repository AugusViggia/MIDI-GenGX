$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionComposerKnowledgeTrainingCorpus.h",
    "Source\Music\CompositionComposerKnowledgeTrainingCorpus.cpp",
    "Source\Tests\CompositionComposerKnowledgeTrainingCorpusTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 100 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Implementation = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionComposerKnowledgeTrainingCorpus.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionComposerKnowledgeTrainingCorpusTests.cpp")

foreach ($Required in @(
    "CompositionComposerKnowledgeTrainingCorpus.h",
    "CompositionComposerKnowledgeTrainingCorpus.cpp"
))
{
    if ($CMake -notmatch [regex]::Escape("Source/Music/$Required"))
    {
        throw "Phase 100 source is not part of MIDI_GenGX_Music: $Required"
    }
}

if ($CMake -notmatch "MIDI_GenGX_CompositionComposerKnowledgeTrainingCorpusTests")
{
    throw "Phase 100 test target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionComposerKnowledgeTrainingCorpusTests")
{
    throw "build-x64.ps1 does not execute Phase 100 tests."
}

foreach ($Required in @(
    "buildCompositionComposerKnowledgeTrainingCorpus",
    "trainingSamples",
    "validationSamples",
    "testSamples",
    "manifest",
    "partition"
))
{
    if ($Implementation -notmatch [regex]::Escape($Required))
    {
        throw "Phase 100 training corpus component is missing: $Required"
    }
}

foreach ($Case in @(
    "testBuildProducesReadyCorpus",
    "testSampleIdentityIsPreserved",
    "testManifestMismatchFailsClosed",
    "testInvalidPartitionFailsClosed"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 100 test is missing: $Case"
    }
}


if ($Implementation -notmatch 'corpus\.valid\s*=\s*true' -or
    $Implementation -notmatch 'if \(!corpus\.isValid\(\)\)')
{
    throw "Phase 100 corpus builder does not establish validity before final structural validation."
}

Write-Host "Phase 100 composer knowledge training corpus validation passed." -ForegroundColor Green
