$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Generation\PhraseGenerationWorker.h",
    "Source\Generation\PhraseGenerationWorker.cpp",
    "Source\Plugin\AIRuntimeGeneration.h",
    "Source\Plugin\AIRuntimeGeneration.cpp",
    "Source\Tests\AIPluginRuntimeIntegrationTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 56 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Worker = Get-Content -Raw (Join-Path $Root "Source\Generation\PhraseGenerationWorker.cpp")
$Processor = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginProcessor.cpp")
$Runtime = Get-Content -Raw (Join-Path $Root "Source\Plugin\AIRuntimeGeneration.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\AIPluginRuntimeIntegrationTests.cpp")

if ($CMake -notmatch "Source/Plugin/AIRuntimeGeneration\.cpp")
{
    throw "AIRuntimeGeneration.cpp is not part of the plugin target."
}

if ($CMake -notmatch "MIDI_GenGX_AIPluginRuntimeIntegrationTests")
{
    throw "AIPluginRuntimeIntegrationTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_AIPluginRuntimeIntegrationTests")
{
    throw "build-x64.ps1 does not execute AIPluginRuntimeIntegrationTests."
}

if ($Worker -notmatch "GenerationProvider")
{
    throw "Generation worker provider interface is missing."
}

if ($Worker -notmatch "generationProvider")
{
    throw "Generation worker does not use the injectable provider."
}

if ($Processor -notmatch "aiRuntimeGeneration\.generate")
{
    throw "PluginProcessor is not connected to the AI runtime provider."
}

if ($Runtime -notmatch "isEnabled")
{
    throw "AI runtime enable state is missing."
}


if ($Processor -notmatch "midigengx::plugin::AIRuntimeGeneration")
{
    throw "PluginProcessor does not qualify the AI runtime type with its namespace."
}
if ($Runtime -notmatch "engine\.generate")
{
    throw "AI runtime baseline fallback is missing."
}

foreach ($Case in @(
    "testRuntimeFallsBackWhenDisabled",
    "testRuntimeUsesProviderWhenEnabled",
    "testRuntimeProviderCanBeReplaced",
    "testWorkerUsesInjectedGenerationProvider",
    "testWorkerFallbackWithoutProvider"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 56 integration test is missing: $Case"
    }
}


if ($CMake -notmatch 'MIDI_GenGX_artefacts/JuceLibraryCode')
{
    throw "AI runtime integration test target is missing the generated JUCE header include directory."
}

if ($CMake -notmatch 'juce::juce_audio_processors')
{
    throw "AI runtime integration test target is missing its JUCE dependency."
}

Write-Host "Phase 56 plugin AI runtime integration validation passed." -ForegroundColor Green
