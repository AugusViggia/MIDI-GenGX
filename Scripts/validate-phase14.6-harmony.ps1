$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\HarmonyPlan.h",
    "Source\Music\HarmonyPlan.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 14.6 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
if ($CMake -notmatch "Source/Music/HarmonyPlan\.cpp")
{
    throw "HarmonyPlan.cpp is not part of MIDI_GenGX_Music."
}

$Harmony = Get-Content -Raw (
    Join-Path $Root "Source\Music\HarmonyPlan.cpp"
)

foreach ($Symbol in @(
    "planHarmony",
    "inferTriadQuality",
    "HarmonyPlan::isValid"
))
{
    if ($Harmony -notmatch [regex]::Escape($Symbol))
    {
        throw "$Symbol implementation is missing."
    }
}

$Tests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\MusicalEngineTests.cpp"
)

if ($Tests -notmatch "testHarmonyPlanning")
{
    throw "Harmony planning tests are missing."
}

Write-Host "Phase 14.6 harmony planning validation passed." -ForegroundColor Green
