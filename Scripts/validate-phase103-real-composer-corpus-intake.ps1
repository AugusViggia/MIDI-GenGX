$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionRealComposerCorpusIntake.h",
    "Source\Music\CompositionRealComposerCorpusIntake.cpp",
    "Source\Tests\CompositionRealComposerCorpusIntakeTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 103 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Implementation = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionRealComposerCorpusIntake.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionRealComposerCorpusIntakeTests.cpp")

foreach ($Required in @(
    "CompositionRealComposerCorpusIntake.h",
    "CompositionRealComposerCorpusIntake.cpp"
))
{
    if ($CMake -notmatch [regex]::Escape("Source/Music/$Required"))
    {
        throw "Phase 103 source is not part of MIDI_GenGX_Music: $Required"
    }
}

if ($CMake -notmatch "MIDI_GenGX_CompositionRealComposerCorpusIntakeTests")
{
    throw "Phase 103 test target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionRealComposerCorpusIntakeTests")
{
    throw "build-x64.ps1 does not execute Phase 103 tests."
}

foreach ($Required in @(
    "inspectRealComposerCorpusDirectory",
    "canEnterFirstComposerTraining",
    "composerId",
    "sampleId",
    "issues"
))
{
    if ($Implementation -notmatch [regex]::Escape($Required))
    {
        throw "Phase 103 intake component is missing: $Required"
    }
}

foreach ($Case in @(
    "testRealComposerIntakeMatchesCatalog",
    "testMissingComposerFileFailsClosed",
    "testUnexpectedComposerFileFailsClosed",
    "testUnknownComposerFailsClosed"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 103 test is missing: $Case"
    }
}

Write-Host "Phase 103 real composer corpus intake validation passed." -ForegroundColor Green
