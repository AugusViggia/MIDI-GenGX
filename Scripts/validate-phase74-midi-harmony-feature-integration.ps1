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
        throw "Missing Phase 74 file: $RelativePath"
    }
}

$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiDatasetFeatureExtractor.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiDatasetFeatureExtractor.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiDatasetFeatureExtractorTests.cpp")

if ($Header -notmatch "CompositionMidiHarmonyAnalysis")
{
    throw "Feature extractor API is not connected to MIDI harmony analysis."
}

if ($Impl -notmatch "harmony\.sections")
{
    throw "Feature extractor does not consume per-section harmony results."
}

if ($Impl -notmatch "harmony\.sections\.size\(\)")
{
    throw "Harmony event count is not integrated into the global feature vector."
}

if ($Impl -notmatch "harmonicQualityEncoding")
{
    throw "Harmony quality encoding is missing."
}

if ($Impl -notmatch "const auto harmonicDegreeDelta")
{
    throw "Harmonic degree delta integration is missing."
}

if ($Impl -match "No harmonic analysis yet")
{
    throw "Old placeholder harmonic extraction code remains."
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
        throw "Required Phase 74 regression test is missing: $Case"
    }
}


if ($Tests -notmatch "ChordQuality::Unknown")
{
    throw "Phase 74 Unknown-harmony regression test does not explicitly construct an Unknown harmonic label."
}

if ($Tests -notmatch "harmonicSection\.quality")
{
    throw "Phase 74 Unknown-harmony regression test does not modify the harmonic result."
}

Write-Host "Phase 74 MIDI harmony feature integration validation passed." -ForegroundColor Green
