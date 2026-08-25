$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionComposerKnowledgeCorpusManifest.h",
    "Source\Music\CompositionComposerKnowledgeCorpusManifest.cpp",
    "Source\Tests\CompositionComposerKnowledgeCorpusManifestTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 99 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Implementation = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionComposerKnowledgeCorpusManifest.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionComposerKnowledgeCorpusManifestTests.cpp")

foreach ($Required in @(
    "CompositionComposerKnowledgeCorpusManifest.h",
    "CompositionComposerKnowledgeCorpusManifest.cpp"
))
{
    if ($CMake -notmatch [regex]::Escape("Source/Music/$Required"))
    {
        throw "Phase 99 source is not part of MIDI_GenGX_Music: $Required"
    }
}

if ($CMake -notmatch "MIDI_GenGX_CompositionComposerKnowledgeCorpusManifestTests")
{
    throw "Phase 99 test target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionComposerKnowledgeCorpusManifestTests")
{
    throw "build-x64.ps1 does not execute Phase 99 tests."
}

foreach ($Required in @(
    "buildCompositionComposerKnowledgeCorpusManifest",
    "corpusId",
    "corpusVersion",
    "trainingCount",
    "validationCount",
    "testCount",
    "workCount"
))
{
    if ($Implementation -notmatch [regex]::Escape($Required))
    {
        throw "Phase 99 corpus manifest component is missing: $Required"
    }
}

foreach ($Case in @(
    "testManifestIsValid",
    "testManifestPreservesComposerIdentity",
    "testInvalidInputsFailClosed",
    "testUnverifiedCatalogCannotProduceVerifiedManifest"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 99 test is missing: $Case"
    }
}

Write-Host "Phase 99 composer knowledge corpus manifest validation passed." -ForegroundColor Green
