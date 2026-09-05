$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\PhraseStructure.h",
    "Source\Music\PhraseStructure.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 14.5 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
if ($CMake -notmatch "Source/Music/PhraseStructure\.cpp")
{
    throw "PhraseStructure.cpp is not part of MIDI_GenGX_Music."
}

$Structure = Get-Content -Raw (
    Join-Path $Root "Source\Music\PhraseStructure.cpp"
)

foreach ($Symbol in @(
    "planPhraseStructure",
    "cadenceTargetScaleDegree",
    "PhraseStructurePlan::isValid"
))
{
    if ($Structure -notmatch [regex]::Escape($Symbol))
    {
        throw "$Symbol implementation is missing."
    }
}

$Composer = Get-Content -Raw (
    Join-Path $Root "Source\Music\MotifPhraseComposer.cpp"
)

if ($Composer -notmatch "planPhraseStructure")
{
    throw "MotifPhraseComposer does not consume phrase structure."
}

if ($Composer -notmatch "cadenceStrength")
{
    throw "MotifPhraseComposer does not consume cadence strength."
}

$Tests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\MusicalEngineTests.cpp"
)

if ($Tests -notmatch "testPhraseStructureAndCadence")
{
    throw "Phrase structure/cadence tests are missing."
}

Write-Host "Phase 14.5 phrase-structure validation passed." -ForegroundColor Green
