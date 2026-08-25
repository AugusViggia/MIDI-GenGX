$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionAIModelRuntimeProvider.h",
    "Source\Music\CompositionAIModelRuntimeProvider.cpp",
    "Source\Tests\CompositionAIModelRuntimeProviderTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 62 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionAIModelRuntimeProvider.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionAIModelRuntimeProvider.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionAIModelRuntimeProviderTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionAIModelRuntimeProvider\.cpp")
{
    throw "CompositionAIModelRuntimeProvider.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionAIModelRuntimeProviderTests")
{
    throw "CompositionAIModelRuntimeProviderTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionAIModelRuntimeProviderTests")
{
    throw "build-x64.ps1 does not execute CompositionAIModelRuntimeProviderTests."
}

if ($Impl -notmatch "CompositionNeuralModelRuntimeLoader")
{
    throw "Runtime provider is not connected to the model loader."
}

if ($Impl -notmatch "buildCompositionInferencePipeline")
{
    throw "Runtime provider is not connected to inference pipeline construction."
}

if ($Impl -notmatch "buildCompositionAIGenerationCoordinator")
{
    throw "Runtime provider is not connected to AI generation coordination."
}

if ($Impl -notmatch "engine\.generate")
{
    throw "Runtime provider lacks safe baseline fallback."
}

foreach ($Case in @(
    "testEmptyProviderIsNotReady",
    "testValidArtifactMakesProviderReady",
    "testLoadedModelGeneratesValidPhrase",
    "testGeneratedPhraseIsDeterministic",
    "testDifferentSeedsRemainFunctional",
    "testInvalidArtifactDoesNotReplaceActiveModel",
    "testClearReturnsProviderToEmptyState"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 62 test is missing: $Case"
    }
}

Write-Host "Phase 62 AI model runtime provider validation passed." -ForegroundColor Green
