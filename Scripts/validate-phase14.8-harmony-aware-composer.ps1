$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\HarmonyGuidance.h",
    "Source\Music\HarmonyGuidance.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 14.8 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
if ($CMake -notmatch "Source/Music/HarmonyGuidance\.cpp")
{
    throw "HarmonyGuidance.cpp is not part of MIDI_GenGX_Music."
}

$Guidance = Get-Content -Raw (
    Join-Path $Root "Source\Music\HarmonyGuidance.cpp"
)

foreach ($Symbol in @(
    "findHarmonyEventAtBeat",
    "buildHarmonyGuidance",
    "chooseHarmonyAwarePitch",
    "HarmonyGuidance::scorePitch"
))
{
    if ($Guidance -notmatch [regex]::Escape($Symbol))
    {
        throw "$Symbol implementation is missing."
    }
}

$Composer = Get-Content -Raw (
    Join-Path $Root "Source\Music\MotifPhraseComposer.cpp"
)

foreach ($Symbol in @(
    "planHarmony",
    "findHarmonyEventAtBeat",
    "chooseHarmonyAwarePitch"
))
{
    if ($Composer -notmatch [regex]::Escape($Symbol))
    {
        throw "$Symbol is not integrated into the phrase composer."
    }
}

$Tests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\MusicalEngineTests.cpp"
)

if ($Tests -notmatch "testHarmonyAwareMotifComposition")
{
    throw "Harmony-aware composition regression test is missing."
}

Write-Host "Phase 14.8 harmony-aware composer validation passed." -ForegroundColor Green
