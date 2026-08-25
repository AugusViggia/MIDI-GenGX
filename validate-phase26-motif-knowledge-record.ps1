$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\MotifKnowledgeRecord.h",
    "Source\Music\MotifKnowledgeRecord.cpp",
    "Source\Tests\MotifKnowledgeRecordTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 26 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\MotifKnowledgeRecord.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\MotifKnowledgeRecord.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\MotifKnowledgeRecordTests.cpp")

if ($CMake -notmatch "Source/Music/MotifKnowledgeRecord\.cpp")
{
    throw "MotifKnowledgeRecord.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_MotifKnowledgeRecordTests")
{
    throw "MotifKnowledgeRecordTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_MotifKnowledgeRecordTests")
{
    throw "build-x64.ps1 does not execute MotifKnowledgeRecordTests."
}

if ($Header -notmatch "struct MotifKnowledgeRecord")
{
    throw "Motif knowledge record model is missing."
}

if ($Impl -notmatch "buildMotifKnowledgeRecord")
{
    throw "Motif knowledge record builder is missing."
}

foreach ($Case in @(
    "testRecordCapturesStableIdentity",
    "testRecordCapturesRecurrence",
    "testRecordCapturesTransformations",
    "testSingleRecord",
    "testInvalidInputsProduceInvalidRecord"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required knowledge-record test is missing: $Case"
    }
}

Write-Host "Phase 26 motif-knowledge-record validation passed." -ForegroundColor Green
