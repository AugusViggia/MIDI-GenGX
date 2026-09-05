$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\MotifOccurrenceGraph.h",
    "Source\Music\MotifOccurrenceGraph.cpp",
    "Source\Tests\MotifOccurrenceGraphTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 23 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\MotifOccurrenceGraph.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\MotifOccurrenceGraph.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\MotifOccurrenceGraphTests.cpp")

if ($CMake -notmatch "Source/Music/MotifOccurrenceGraph\.cpp")
{
    throw "MotifOccurrenceGraph.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_MotifOccurrenceGraphTests")
{
    throw "MotifOccurrenceGraphTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_MotifOccurrenceGraphTests")
{
    throw "build-x64.ps1 does not execute MotifOccurrenceGraphTests."
}

if ($Header -notmatch "struct MotifOccurrenceGraph")
{
    throw "Motif occurrence graph model is missing."
}

if ($Impl -notmatch "buildMotifOccurrenceGraph")
{
    throw "Motif occurrence graph builder is missing."
}

foreach ($Case in @(
    "testOccurrenceNodes",
    "testRelationshipEdges",
    "testInvalidMotifsAreExcluded",
    "testSparseSourcePhraseMetadata"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required occurrence-graph test is missing: $Case"
    }
}

Write-Host "Phase 23 motif-occurrence graph validation passed." -ForegroundColor Green
