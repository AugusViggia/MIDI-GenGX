$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$RequiredFiles = @(
    "Source\Music\CompositionComposerKnowledgeSample.h",
    "Source\Music\CompositionComposerKnowledgeSample.cpp",
    "Source\Tests\CompositionComposerKnowledgeSampleTests.cpp"
)

foreach ($RelativePath in $RequiredFiles)
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 96 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Implementation = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionComposerKnowledgeSample.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionComposerKnowledgeSampleTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionComposerKnowledgeSample\.h" -or
    $CMake -notmatch "Source/Music/CompositionComposerKnowledgeSample\.cpp")
{
    throw "Phase 96 composer knowledge representation is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionComposerKnowledgeSampleTests")
{
    throw "Phase 96 test target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionComposerKnowledgeSampleTests")
{
    throw "build-x64.ps1 does not execute Phase 96 tests."
}

foreach ($Required in @(
    "CompositionDatasetSample",
    "CompositionSequenceMetadata",
    "sampleId",
    "buildCompositionComposerKnowledgeSample"
))
{
    if ($Implementation -notmatch [regex]::Escape($Required))
    {
        throw "Phase 96 knowledge representation component is missing: $Required"
    }
}

if ($Implementation -match "composerId\s*=" -or
    $Implementation -match "styleId\s*=")
{
    throw "Phase 96 representation must preserve metadata, not invent composer/style values."
}

foreach ($Case in @(
    "testKnowledgeSampleJoinsAnalysisAndMetadata",
    "testMismatchedIdentityIsRejected",
    "testUnverifiedMetadataIsStructurallyRepresentable"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 96 test is missing: $Case"
    }
}

Write-Host "Phase 96 composer knowledge representation validation passed." -ForegroundColor Green
