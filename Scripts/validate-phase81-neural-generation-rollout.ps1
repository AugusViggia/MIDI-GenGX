$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionNeuralGenerationService.h",
    "Source\Music\CompositionNeuralGenerationService.cpp",
    "Source\Tests\CompositionNeuralGenerationServiceTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 81 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionNeuralGenerationService.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionNeuralGenerationServiceTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionNeuralGenerationService\.cpp")
{
    throw "CompositionNeuralGenerationService.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionNeuralGenerationServiceTests")
{
    throw "CompositionNeuralGenerationServiceTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionNeuralGenerationServiceTests")
{
    throw "build-x64.ps1 does not execute CompositionNeuralGenerationServiceTests."
}

if ($Impl -notmatch "pipeline\.infer")
{
    throw "Neural generation does not use the inference pipeline."
}

if ($Impl -notmatch "context\s*=\s*generated")
{
    throw "Autoregressive context update is missing."
}

if ($Impl -notmatch "generatedSections")
{
    throw "Generated section sequence storage is missing."
}

foreach ($Case in @(
    "testAutoregressiveRollout",
    "testGenerationIsDeterministic",
    "testGenerationChangesContext",
    "testInvalidInputsAreRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 81 test is missing: $Case"
    }
}

Write-Host "Phase 81 neural generation rollout validation passed." -ForegroundColor Green
