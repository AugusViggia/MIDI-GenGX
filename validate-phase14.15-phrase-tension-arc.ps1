$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\PhraseTensionArc.h",
    "Source\Music\PhraseTensionArc.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 14.15 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
if ($CMake -notmatch "Source/Music/PhraseTensionArc\.cpp")
{
    throw "PhraseTensionArc.cpp is not part of MIDI_GenGX_Music."
}

$Structure = Get-Content -Raw (
    Join-Path $Root "Source\Music\PhraseStructure.cpp"
)

if ($Structure -notmatch "PhraseTensionArc::sectionTension")
{
    throw "PhraseStructure does not consume PhraseTensionArc."
}

$Arc = Get-Content -Raw (
    Join-Path $Root "Source\Music\PhraseTensionArc.cpp"
)

foreach ($Symbol in @(
    "PhraseTensionArc::normalizedProgress",
    "PhraseTensionArc::climaxPosition",
    "PhraseTensionArc::tensionAtProgress",
    "PhraseTensionArc::sectionTension"
))
{
    if ($Arc -notmatch [regex]::Escape($Symbol))
    {
        throw "$Symbol implementation is missing."
    }
}

$Tests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\MusicalEngineTests.cpp"
)

if ($Tests -notmatch "testPhraseTensionArc")
{
    throw "Phrase tension arc regression tests are missing."
}

Write-Host "Phase 14.15 phrase-tension validation passed." -ForegroundColor Green
