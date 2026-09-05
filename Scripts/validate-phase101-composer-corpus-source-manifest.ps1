$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionComposerCorpusSourceManifest.h",
    "Source\Music\CompositionComposerCorpusSourceManifest.cpp",
    "Source\Tests\CompositionComposerCorpusSourceManifestTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 101 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Implementation = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionComposerCorpusSourceManifest.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionComposerCorpusSourceManifestTests.cpp")

foreach ($Required in @(
    "CompositionComposerCorpusSourceManifest.h",
    "CompositionComposerCorpusSourceManifest.cpp"
))
{
    if ($CMake -notmatch [regex]::Escape("Source/Music/$Required"))
    {
        throw "Phase 101 source is not part of MIDI_GenGX_Music: $Required"
    }
}

if ($CMake -notmatch "MIDI_GenGX_CompositionComposerCorpusSourceManifestTests")
{
    throw "Phase 101 test target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionComposerCorpusSourceManifestTests")
{
    throw "build-x64.ps1 does not execute Phase 101 tests."
}

foreach ($Required in @(
    "buildCompositionComposerCorpusSourceManifest",
    "relativeMidiPath",
    "contentHash",
    "byteSize",
    "sampleId"
))
{
    if ($Implementation -notmatch [regex]::Escape($Required))
    {
        throw "Phase 101 source manifest component is missing: $Required"
    }
}

foreach ($Case in @(
    "testRealDirectoryMapsExactlyToCatalog",
    "testMissingFileFailsClosed",
    "testUnexpectedMidiFailsClosed",
    "testNonRecursiveModeRespectsRootBoundary"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 101 test is missing: $Case"
    }
}


if ($Implementation -notmatch "lessPath")
{
    throw "Phase 101 source manifest does not use deterministic case-insensitive relative-path ordering."
}

Write-Host "Phase 101 composer corpus source manifest validation passed." -ForegroundColor Green
