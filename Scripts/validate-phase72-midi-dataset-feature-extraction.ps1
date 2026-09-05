$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionMidiDatasetFeatureExtractor.h",
    "Source\Music\CompositionMidiDatasetFeatureExtractor.cpp",
    "Source\Tests\CompositionMidiDatasetFeatureExtractorTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 72 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiDatasetFeatureExtractor.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiDatasetFeatureExtractorTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionMidiDatasetFeatureExtractor\.cpp")
{
    throw "CompositionMidiDatasetFeatureExtractor.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionMidiDatasetFeatureExtractorTests")
{
    throw "CompositionMidiDatasetFeatureExtractorTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionMidiDatasetFeatureExtractorTests")
{
    throw "build-x64.ps1 does not execute CompositionMidiDatasetFeatureExtractorTests."
}

if ($Impl -notmatch "globalFeatures")
{
    throw "Global ML feature extraction is missing."
}

if ($Impl -notmatch "sectionFeatures")
{
    throw "Section ML feature extraction is missing."
}

if ($Impl -notmatch "Unknown")
{
    throw "Explicit unknown-harmony handling is missing."
}

if ($Impl -notmatch "normalizeSigned")
{
    throw "Signed transition normalization is missing."
}

foreach ($Case in @(
    "testRealMidiFeaturesBecomeSchemaSample",
    "testNoUnknownHarmonicDataIsFabricated",
    "testTensionInformationPropagates",
    "testTransitionFeaturesPropagate",
    "testInvalidInputsAreRejected",
    "testDeterministicExtraction"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 72 test is missing: $Case"
    }
}

Write-Host "Phase 72 MIDI dataset feature extraction validation passed." -ForegroundColor Green
