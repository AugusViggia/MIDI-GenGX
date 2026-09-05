$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionNeuralModel.h",
    "Source\Music\CompositionNeuralModel.cpp",
    "Source\Tests\CompositionNeuralModelTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 43 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionNeuralModel.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionNeuralModel.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionNeuralModelTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionNeuralModel\.cpp")
{
    throw "CompositionNeuralModel.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionNeuralModelTests")
{
    throw "CompositionNeuralModelTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionNeuralModelTests")
{
    throw "build-x64.ps1 does not execute CompositionNeuralModelTests."
}

if ($Header -notmatch "struct CompositionNeuralModel")
{
    throw "Neural model definition is missing."
}

if ($Header -notmatch "hiddenWidth = 32")
{
    throw "Neural hidden-layer width contract is missing."
}

if ($Impl -notmatch "tanhActivation")
{
    throw "Neural activation implementation is missing."
}

if ($Impl -notmatch "initializeCompositionNeuralModel")
{
    throw "Neural initialization is missing."
}

if ($Impl -notmatch "predictNextSection")
{
    throw "Neural inference implementation is missing."
}

foreach ($Case in @(
    "testDeterministicInitialization",
    "testParameterShape",
    "testPredictionShapeAndRange",
    "testInferenceIsDeterministic",
    "testInvalidInputIsRejected",
    "testInvalidContractPreventsInitialization"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required neural-model test is missing: $Case"
    }
}

Write-Host "Phase 43 composition-neural-model validation passed." -ForegroundColor Green
