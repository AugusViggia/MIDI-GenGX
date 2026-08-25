$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionLearningContract.h",
    "Source\Music\CompositionLearningContract.cpp",
    "Source\Tests\CompositionLearningContractTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 41 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionLearningContract.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionLearningContract.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionLearningContractTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionLearningContract\.cpp")
{
    throw "CompositionLearningContract.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionLearningContractTests")
{
    throw "CompositionLearningContractTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionLearningContractTests")
{
    throw "build-x64.ps1 does not execute CompositionLearningContractTests."
}

if ($Header -notmatch "struct CompositionLearningContract")
{
    throw "Learning contract model is missing."
}

if ($Header -notmatch "enum class LearningObjective")
{
    throw "Learning objective vocabulary is missing."
}

if ($Impl -notmatch "buildCompositionLearningContract")
{
    throw "Learning contract builder is missing."
}

foreach ($Case in @(
    "testNextSectionContract",
    "testReconstructionContract",
    "testContractIsDeterministic",
    "testInvalidPreparedViewIsRejected",
    "testWidthsCannotDriftFromSchema",
    "testMaskRequirementCannotBeRemoved"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required learning-contract test is missing: $Case"
    }
}


if ($Impl -match 'contextLength\s*<\s*1')
{
    throw "Learning contract still contains the redundant context-length comparison."
}

if ($Impl -notmatch 'contextLength\s*==\s*0')
{
    throw "Learning contract must explicitly reject zero context for next-section prediction."
}

Write-Host "Phase 41 composition-learning-contract validation passed." -ForegroundColor Green
