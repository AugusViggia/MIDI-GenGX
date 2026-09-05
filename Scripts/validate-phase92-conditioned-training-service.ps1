$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$RequiredFiles = @(
    "Source\Music\CompositionConditionedSequenceNeuralModelArtifact.h",
    "Source\Music\CompositionConditionedSequenceNeuralModelArtifact.cpp",
    "Source\Music\CompositionConditionedSequenceNeuralTrainingService.h",
    "Source\Music\CompositionConditionedSequenceNeuralTrainingService.cpp",
    "Source\Tests\CompositionConditionedSequenceNeuralTrainingServiceTests.cpp"
)

foreach ($RelativePath in $RequiredFiles)
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 92 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$ArtifactHeader = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedSequenceNeuralModelArtifact.h")
$Artifact = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedSequenceNeuralModelArtifact.cpp")
$Service = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedSequenceNeuralTrainingService.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionConditionedSequenceNeuralTrainingServiceTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionConditionedSequenceNeuralModelArtifact\.cpp" -or
    $CMake -notmatch "Source/Music/CompositionConditionedSequenceNeuralTrainingService\.cpp")
{
    throw "Phase 92 sources are not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionConditionedSequenceNeuralTrainingServiceTests")
{
    throw "Phase 92 training-service test target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionConditionedSequenceNeuralTrainingServiceTests")
{
    throw "build-x64.ps1 does not execute Phase 92 tests."
}

if ($ArtifactHeader -notmatch 'magic\s*=\s*0x4D47434E')
{
    throw "Phase 92 conditioned neural artifact magic is missing."
}

foreach ($Required in @(
    "serializeCompositionConditionedSequenceNeuralModel",
    "deserializeCompositionConditionedSequenceNeuralModel"
))
{
    if ($Artifact -notmatch $Required)
    {
        throw "Phase 92 artifact operation is missing: $Required"
    }
}

if ($Service -notmatch "trainCompositionConditionedSequenceNeuralModel" -or
    $Service -notmatch "serializeCompositionConditionedSequenceNeuralModel")
{
    throw "Phase 92 service does not perform conditioned training and artifact persistence."
}

foreach ($Case in @(
    "testTrainingFromDatasetProducesModelArtifact",
    "testModelArtifactRoundTripPreservesParameters",
    "testInvalidDatasetIsRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 92 test is missing: $Case"
    }
}


$ArtifactImpl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedSequenceNeuralModelArtifact.cpp")

if ($ArtifactImpl -notmatch "headerSize\s*=\s*\n\s*sizeof\(std::uint32_t\)\s*\*\s*13")
{
    throw "Phase 92 conditioned neural artifact header size does not match its 13 serialized uint32 fields."
}

if ($ArtifactImpl -match "const\s+auto\s+counts\[\]")
{
    throw "Phase 92 artifact serializer uses a C++17-invalid const auto array."
}

Write-Host "Phase 92 conditioned training service validation passed." -ForegroundColor Green
