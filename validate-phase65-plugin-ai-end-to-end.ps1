$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Plugin\PluginProcessor.h",
    "Source\Plugin\PluginProcessor.cpp",
    "Source\Tests\PluginAIRuntimeEndToEndTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 65 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginProcessor.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginProcessor.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\PluginAIRuntimeEndToEndTests.cpp")

if ($CMake -notmatch "MIDI_GenGX_PluginAIRuntimeEndToEndTests")
{
    throw "PluginAIRuntimeEndToEndTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_PluginAIRuntimeEndToEndTests")
{
    throw "build-x64.ps1 does not execute PluginAIRuntimeEndToEndTests."
}

if ($Header -notmatch "requestAIRuntimeGeneration")
{
    throw "Explicit AI generation trigger is missing."
}

if ($Header -notmatch "isAIRuntimeModelActive")
{
    throw "AI runtime active-state API is missing."
}

if ($Impl -notmatch "requestPhraseGeneration")
{
    throw "AI runtime trigger is not connected to the generation worker."
}

if ($Impl -notmatch "aiRuntimeGeneration\.isEnabled")
{
    throw "AI runtime trigger does not check AI enable state."
}

foreach ($Case in @(
    "testAIModelActivationState",
    "testAIRequestRequiresGeneratorAndAI",
    "testEndToEndAIWorkerGenerationSettles",
    "testAIModelRemainsActiveAcrossMultipleRequests",
    "testDisablingAIReturnsToBaselineMode"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 65 test is missing: $Case"
    }
}


if ($Tests -notmatch "processor\.processBlock")
{
    throw "Phase 65 end-to-end tests do not exercise the plugin MIDI/audio adoption boundary."
}

if ($Tests -notmatch "prepareToPlay")
{
    throw "Phase 65 end-to-end tests do not initialize the plugin runtime before worker/adoption checks."
}

Write-Host "Phase 65 plugin AI end-to-end validation passed." -ForegroundColor Green
