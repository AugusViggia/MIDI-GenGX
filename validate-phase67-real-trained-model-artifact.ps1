$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionNeuralArtifactTrainingService.h",
    "Source\Music\CompositionNeuralArtifactTrainingService.cpp",
    "Source\Tests\CompositionNeuralArtifactTrainingServiceTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 67 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionNeuralArtifactTrainingService.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionNeuralArtifactTrainingService.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionNeuralArtifactTrainingServiceTests.cpp")


if ($CMake -notmatch "Source/Music/CompositionNeuralArtifactTrainingService\.cpp")
{
    throw "CompositionNeuralArtifactTrainingService.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionNeuralArtifactTrainingServiceTests")
{
    throw "CompositionNeuralArtifactTrainingServiceTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionNeuralArtifactTrainingServiceTests")
{
    throw "build-x64.ps1 does not execute CompositionNeuralArtifactTrainingServiceTests."
}

if ($Impl -notmatch "trainCompositionNeuralModel")
{
    throw "Phase 67 service is not connected to the real trainer."
}

if ($Impl -notmatch "serializeCompositionNeuralModel")
{
    throw "Phase 67 service does not emit a neural model artifact."
}

if ($Impl -notmatch "training\.finalLoss")
{
    throw "Phase 67 service does not preserve final training metrics."
}

foreach ($Case in @(
    "testTrainingProducesArtifact",
    "testTrainingActuallyChangesTheModel",
    "testTrainingImprovesOrMaintainsLoss",
    "testArtifactLoadsBackExactly",
    "testTrainingIsDeterministic",
    "testInvalidPreparedDatasetIsRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 67 test is missing: $Case"
    }
}

Write-Host "Phase 67 real-trained model artifact validation passed." -ForegroundColor Green
