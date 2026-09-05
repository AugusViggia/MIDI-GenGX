$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionKnowledgeRecord.h",
    "Source\Music\CompositionKnowledgeRecord.cpp",
    "Source\Tests\CompositionKnowledgeRecordTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 28 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionKnowledgeRecord.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionKnowledgeRecord.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionKnowledgeRecordTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionKnowledgeRecord\.cpp")
{
    throw "CompositionKnowledgeRecord.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionKnowledgeRecordTests")
{
    throw "CompositionKnowledgeRecordTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionKnowledgeRecordTests")
{
    throw "build-x64.ps1 does not execute CompositionKnowledgeRecordTests."
}

if ($Header -notmatch "struct CompositionKnowledgeRecord")
{
    throw "Composition knowledge record model is missing."
}

if ($Impl -notmatch "buildCompositionKnowledgeRecord")
{
    throw "Composition knowledge record builder is missing."
}

if ($Impl -notmatch "harmony.totalLengthBeats")
{
    throw "Structural/harmony consistency check is missing."
}

foreach ($Case in @(
    "testRecordValidityAndStructure",
    "testTensionSummary",
    "testMotifSummary",
    "testHarmonySummary",
    "testMismatchedCompositionLengthIsRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required composition-knowledge test is missing: $Case"
    }
}

Write-Host "Phase 28 composition-knowledge validation passed." -ForegroundColor Green
