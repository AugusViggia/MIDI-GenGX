$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\MotifRelationship.h",
    "Source\Music\MotifRelationship.cpp",
    "Source\Tests\MotifRelationshipTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 22 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\MotifRelationship.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\MotifRelationship.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\MotifRelationshipTests.cpp")

if ($CMake -notmatch "Source/Music/MotifRelationship\.cpp")
{
    throw "MotifRelationship.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_MotifRelationshipTests")
{
    throw "MotifRelationshipTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_MotifRelationshipTests")
{
    throw "build-x64.ps1 does not execute MotifRelationshipTests."
}

if ($Header -notmatch "enum class MotifRelationshipKind")
{
    throw "Motif relationship vocabulary is missing."
}

if ($Impl -notmatch "analyzeMotifRelationship")
{
    throw "Motif relationship analysis is missing."
}

if ($Impl -notmatch "exactMotifEquivalent")
{
    throw "Exact motif identity guard is missing."
}

foreach ($Case in @(
    "testIdentity",
    "testTransposition",
    "testRetrogradeAndInversion",
    "testRhythmicVariation",
    "testIntervalVariation",
    "testCompoundVariation",
    "testInvalidInput"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required motif-relationship test is missing: $Case"
    }
}

Write-Host "Phase 22 motif-relationship validation passed." -ForegroundColor Green
