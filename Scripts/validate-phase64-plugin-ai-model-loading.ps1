$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Plugin\PluginProcessor.h",
    "Source\Plugin\PluginProcessor.cpp",
    "Source\Tests\PluginAIRuntimeModelLoadingTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 64 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginProcessor.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginProcessor.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\PluginAIRuntimeModelLoadingTests.cpp")

if ($CMake -notmatch "MIDI_GenGX_PluginAIRuntimeModelLoadingTests")
{
    throw "PluginAIRuntimeModelLoadingTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_PluginAIRuntimeModelLoadingTests")
{
    throw "build-x64.ps1 does not execute PluginAIRuntimeModelLoadingTests."
}

if ($Header -notmatch "loadAIRuntimeModelFromFile")
{
    throw "Plugin model-file loading API is missing."
}

if ($Header -notmatch "hasLoadedAIRuntimeModel")
{
    throw "Plugin loaded-model state API is missing."
}

if ($Impl -notmatch "loadFileAsData")
{
    throw "Plugin file loading does not use JUCE file data loading."
}

if ($Impl -notmatch "aiRuntimeGeneration\.loadModelArtifact")
{
    throw "Plugin processor is not forwarding artifacts to the AI runtime."
}

if ($Impl -notmatch "clearAIRuntimeModel")
{
    throw "Plugin model clearing API is missing."
}

foreach ($Case in @(
    "testProcessorLoadsArtifactBytes",
    "testProcessorRejectsInvalidBytes",
    "testProcessorLoadsArtifactFromFile",
    "testMissingFileIsRejected",
    "testClearRemovesLoadedModel",
    "testGenerationUsesLoadedModelWhenEnabled"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 64 test is missing: $Case"
    }
}


if ($Tests -match "\.toFile\(")
{
    throw "Plugin model loading tests still use unsupported MemoryBlock::toFile."
}

if ($Tests -notmatch "replaceWithData")
{
    throw "Plugin model file test does not use JUCE File::replaceWithData."
}

Write-Host "Phase 64 plugin AI model loading validation passed." -ForegroundColor Green
