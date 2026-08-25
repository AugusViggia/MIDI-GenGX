$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Generation\GenerationActivationPolicy.h",
    "Source\Generation\GenerationActivationPolicy.cpp",
    "Source\Tests\GenerationTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 19.2 file: $RelativePath"
    }
}

$Processor = Get-Content -Raw (
    Join-Path $Root "Source\Plugin\PluginProcessor.cpp"
)

$Editor = Get-Content -Raw (
    Join-Path $Root "Source\Plugin\PluginEditor.cpp"
)

$CMake = Get-Content -Raw (
    Join-Path $Root "CMakeLists.txt"
)

$Build = Get-Content -Raw (
    Join-Path $Root "build-x64.ps1"
)

$Policy = Get-Content -Raw (
    Join-Path $Root "Source\Generation\GenerationActivationPolicy.cpp"
)

$Tests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\GenerationTests.cpp"
)

if ($Processor -notmatch "shouldAdoptPublishedPhrase")
{
    throw "Processor does not use generation activation policy."
}

if ($Editor -match "setGeneratorEnabled\(true\);\s*audioProcessor\.requestGeneration\(\)")
{
    throw "Generator button still submits a duplicate generation request."
}

if ($Policy -notmatch "currentCycle > previousCycle")
{
    throw "Active phrase replacement is not boundary-driven."
}

if ($CMake -notmatch "MIDI_GenGX_GenerationTests")
{
    throw "GenerationTests target is missing from CMake."
}

if ($Build -notmatch "MIDI_GenGX_GenerationTests")
{
    throw "build-x64.ps1 does not execute GenerationTests."
}

foreach ($Case in @(
    "first generated phrase can be adopted during playback",
    "active phrase is not replaced mid-phrase",
    "replacement is adopted after phrase boundary",
    "stopped transport can adopt published phrase"
))
{
    if ($Tests -notmatch [regex]::Escape($Case))
    {
        throw "Generation activation regression case is missing: $Case"
    }
}

Write-Host "Phase 19.2 generation-during-playback validation passed." -ForegroundColor Green
