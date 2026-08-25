$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionKnowledgeSnapshot.h",
    "Source\Music\CompositionKnowledgeSnapshot.cpp",
    "Source\Tests\CompositionKnowledgeSnapshotTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 31 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionKnowledgeSnapshot.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionKnowledgeSnapshot.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionKnowledgeSnapshotTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionKnowledgeSnapshot\.cpp")
{
    throw "CompositionKnowledgeSnapshot.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionKnowledgeSnapshotTests")
{
    throw "CompositionKnowledgeSnapshotTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionKnowledgeSnapshotTests")
{
    throw "build-x64.ps1 does not execute CompositionKnowledgeSnapshotTests."
}

if ($Header -notmatch "struct CompositionKnowledgeSnapshot")
{
    throw "Composition knowledge snapshot model is missing."
}

if ($Impl -notmatch "buildCompositionKnowledgeSnapshot")
{
    throw "Composition knowledge snapshot builder is missing."
}

if ($Impl -notmatch "tensionDeltaFromPrevious")
{
    throw "Section-level transition feature is missing."
}

foreach ($Case in @(
    "testSnapshotValidity",
    "testSectionFeaturesMatchSourceGraph",
    "testTransitionFeaturesAreAligned",
    "testInvalidComponentRejectsSnapshot",
    "testDefaultSnapshotState"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required snapshot test is missing: $Case"
    }
}

Write-Host "Phase 31 composition-knowledge-snapshot validation passed." -ForegroundColor Green
