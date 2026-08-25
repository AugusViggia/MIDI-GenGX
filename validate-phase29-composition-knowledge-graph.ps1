$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionKnowledgeGraph.h",
    "Source\Music\CompositionKnowledgeGraph.cpp",
    "Source\Tests\CompositionKnowledgeGraphTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 29 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionKnowledgeGraph.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionKnowledgeGraph.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionKnowledgeGraphTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionKnowledgeGraph\.cpp")
{
    throw "CompositionKnowledgeGraph.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionKnowledgeGraphTests")
{
    throw "CompositionKnowledgeGraphTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionKnowledgeGraphTests")
{
    throw "build-x64.ps1 does not execute CompositionKnowledgeGraphTests."
}

if ($Header -notmatch "struct CompositionKnowledgeGraph")
{
    throw "Composition knowledge graph model is missing."
}

if ($Impl -notmatch "buildCompositionKnowledgeGraph")
{
    throw "Composition knowledge graph builder is missing."
}

if ($Impl -notmatch "expectedTransitions")
{
    throw "Composition graph transition-count invariant is missing."
}

if ($Impl -notmatch "expectedTransitions =")
{
    throw "Composition graph does not compute expected transition count explicitly."
}

if ($Impl -notmatch "tensionDelta")
{
    throw "Section transition tension delta is missing."
}

if ($Impl -notmatch "harmonicDegreeDelta")
{
    throw "Section transition harmonic delta is missing."
}

foreach ($Case in @(
    "testGraphValidityAndTopology",
    "testSingleSectionGraph",
    "testSectionKnowledge",
    "testTransitionDeltas",
    "testInvalidStructureHarmonyMismatch",
    "testEmptyDefaultState"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required composition-graph test is missing: $Case"
    }
}

Write-Host "Phase 29 composition-knowledge-graph validation passed." -ForegroundColor Green
