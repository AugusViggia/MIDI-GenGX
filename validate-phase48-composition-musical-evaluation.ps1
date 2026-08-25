$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionMusicalEvaluation.h",
    "Source\Music\CompositionMusicalEvaluation.cpp",
    "Source\Tests\CompositionMusicalEvaluationTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 48 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMusicalEvaluation.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMusicalEvaluation.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMusicalEvaluationTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionMusicalEvaluation\.cpp")
{
    throw "CompositionMusicalEvaluation.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionMusicalEvaluationTests")
{
    throw "CompositionMusicalEvaluationTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionMusicalEvaluationTests")
{
    throw "build-x64.ps1 does not execute CompositionMusicalEvaluationTests."
}

if ($Header -notmatch "struct CompositionMusicalEvaluationResult")
{
    throw "Musical evaluation result is missing."
}

if ($Impl -notmatch "tensionConsistencyScore")
{
    throw "Tension consistency scoring is missing."
}

if ($Impl -notmatch "harmonicConsistencyScore")
{
    throw "Harmonic consistency scoring is missing."
}

if ($Impl -notmatch "structuralCoherenceScore")
{
    throw "Structural coherence scoring is missing."
}

if ($Impl -notmatch "overallScore")
{
    throw "Overall musical score is missing."
}

foreach ($Case in @(
    "testMusicalEvaluationIsValid",
    "testScoresRemainNormalized",
    "testValidationAndTestRemainSeparated",
    "testMusicalEvaluationIsDeterministic",
    "testInvalidModelIsRejected",
    "testEmptyEvaluationSetIsRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required musical-evaluation test is missing: $Case"
    }
}

Write-Host "Phase 48 composition-musical-evaluation validation passed." -ForegroundColor Green
