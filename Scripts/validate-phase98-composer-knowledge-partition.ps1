$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionComposerKnowledgePartition.h",
    "Source\Music\CompositionComposerKnowledgePartition.cpp",
    "Source\Tests\CompositionComposerKnowledgePartitionTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 98 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Implementation = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionComposerKnowledgePartition.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionComposerKnowledgePartitionTests.cpp")

foreach ($Required in @(
    "CompositionComposerKnowledgePartition.h",
    "CompositionComposerKnowledgePartition.cpp"
))
{
    if ($CMake -notmatch [regex]::Escape("Source/Music/$Required"))
    {
        throw "Phase 98 source is not part of MIDI_GenGX_Music: $Required"
    }
}

if ($CMake -notmatch "MIDI_GenGX_CompositionComposerKnowledgePartitionTests")
{
    throw "Phase 98 test target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionComposerKnowledgePartitionTests")
{
    throw "build-x64.ps1 does not execute Phase 98 tests."
}

foreach ($Required in @(
    "buildCompositionComposerKnowledgePartition",
    "workId",
    "composerId",
    "validationRatio",
    "testRatio",
    "stableHash"
))
{
    if ($Implementation -notmatch [regex]::Escape($Required))
    {
        throw "Phase 98 partition component is missing: $Required"
    }
}

foreach ($Case in @(
    "testPartitionIsValid",
    "testSplitDoesNotCrossWorkBoundary",
    "testComposerTrainingCoverage",
    "testPartitionIsDeterministic"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 98 test is missing: $Case"
    }
}

Write-Host "Phase 98 composer knowledge partition validation passed." -ForegroundColor Green
