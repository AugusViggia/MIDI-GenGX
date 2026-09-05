$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\MotifRecurrenceProfile.h",
    "Source\Music\MotifRecurrenceProfile.cpp",
    "Source\Tests\MotifRecurrenceProfileTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 24 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\MotifRecurrenceProfile.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\MotifRecurrenceProfile.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\MotifRecurrenceProfileTests.cpp")

if ($CMake -notmatch "Source/Music/MotifRecurrenceProfile\.cpp")
{
    throw "MotifRecurrenceProfile.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_MotifRecurrenceProfileTests")
{
    throw "MotifRecurrenceProfileTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_MotifRecurrenceProfileTests")
{
    throw "build-x64.ps1 does not execute MotifRecurrenceProfileTests."
}

if ($Header -notmatch "struct MotifRecurrenceFamily")
{
    throw "Motif recurrence family model is missing."
}

if ($Impl -notmatch "analyzeMotifRecurrence")
{
    throw "Motif recurrence analysis is missing."
}

if ($Header -notmatch "analysisValid")
{
    throw "Recurrence profile analysis-state flag is missing."
}

if ($Impl -notmatch "profile\.analysisValid = true")
{
    throw "Valid graph does not explicitly mark the recurrence analysis complete."
}

$GraphHeader = Get-Content -Raw (
    Join-Path $Root "Source\Music\MotifOccurrenceGraph.h"
)

$GraphImpl = Get-Content -Raw (
    Join-Path $Root "Source\Music\MotifOccurrenceGraph.cpp"
)

$GraphTests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\MotifOccurrenceGraphTests.cpp"
)

if ($GraphHeader -notmatch "bool analysisValid")
{
    throw "Motif occurrence graph lacks explicit construction state."
}

if ($GraphImpl -notmatch "if \(!analysisValid\)")
{
    throw "Motif occurrence graph validity does not require construction state."
}

if ($GraphImpl -notmatch "graph\.analysisValid = true")
{
    throw "Motif occurrence graph builder does not mark successful construction."
}

if ($GraphTests -notmatch "testGraphConstructionState")
{
    throw "Graph construction-state regression test is missing."
}

foreach ($Case in @(
    "testFamiliesGroupCanonicalIdentity",
    "testTransformationCounts",
    "testUnrelatedMotifsRemainSeparate",
    "testInvalidGraphProducesEmptyProfile",
    "testValidEmptyGraphProducesValidEmptyProfile"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required recurrence-profile test is missing: $Case"
    }
}

Write-Host "Phase 24 motif-recurrence validation passed." -ForegroundColor Green
