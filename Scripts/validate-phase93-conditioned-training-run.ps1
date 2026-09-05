$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$RequiredFiles = @(
    "Source\Music\CompositionConditionedTrainingRunService.h",
    "Source\Music\CompositionConditionedTrainingRunService.cpp",
    "Source\Tests\CompositionConditionedTrainingRunServiceTests.cpp"
)

foreach ($RelativePath in $RequiredFiles)
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 93 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Service = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedTrainingRunService.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionConditionedTrainingRunServiceTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionConditionedTrainingRunService\.cpp")
{
    throw "Phase 93 training-run service is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionConditionedTrainingRunServiceTests")
{
    throw "Phase 93 test target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionConditionedTrainingRunServiceTests")
{
    throw "build-x64.ps1 does not execute Phase 93 tests."
}

foreach ($Required in @(
    "deserializeCompositionMidiTrainingSequences",
    "deserializeCompositionSequenceMetadataCatalog",
    "buildCompositionConditionedTrainingDataset",
    "serializeCompositionConditionedTrainingDataset",
    "trainCompositionConditionedSequenceNeuralModelFromDataset"
))
{
    if ($Service -notmatch [regex]::Escape($Required))
    {
        throw "Phase 93 training-run stage is missing: $Required"
    }
}

foreach ($Case in @(
    "testEndToEndConditionedTraining",
    "testMissingMetadataFailsClosed",
    "testUnverifiedMetadataFailsClosed"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 93 test is missing: $Case"
    }
}


$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedTrainingRunService.h")
$Source = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedTrainingRunService.cpp")

if ($Header -notmatch 'CompositionMidiTrainingCorpusArtifact\.h')
{
    throw "Phase 93 public run-service contract is missing its corpus-artifact dependency."
}

if ($Source -notmatch 'CompositionMidiTrainingCorpusArtifact\.h')
{
    throw "Phase 93 implementation does not include the corpus-artifact API explicitly."
}

Write-Host "Phase 93 conditioned training run validation passed." -ForegroundColor Green
