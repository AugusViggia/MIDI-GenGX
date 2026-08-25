$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionMidiMotifAnalysis.h",
    "Source\Music\CompositionMidiMotifAnalysis.cpp",
    "Source\Tests\CompositionMidiMotifAnalysisTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 75 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiMotifAnalysis.cpp")
$Extractor = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiDatasetFeatureExtractor.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiMotifAnalysisTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionMidiMotifAnalysis\.cpp")
{
    throw "CompositionMidiMotifAnalysis.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionMidiMotifAnalysisTests")
{
    throw "CompositionMidiMotifAnalysisTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionMidiMotifAnalysisTests")
{
    throw "build-x64.ps1 does not execute CompositionMidiMotifAnalysisTests."
}

foreach ($Required in @(
    "buildMotifOccurrenceGraph",
    "analyzeMotifRecurrence",
    "buildMotifKnowledgeCatalog",
    "totalFamilyCount",
    "recurringFamilyCount",
    "averageOccurrenceCount"
))
{
    if ($Impl -notmatch $Required)
    {
        throw "Required motif analysis path is missing: $Required"
    }
}

if ($Extractor -notmatch "motifs\.totalFamilyCount")
{
    throw "Global motif-family feature is not connected to real motif analysis."
}

if ($Extractor -notmatch "motifs\.recurringFamilyCount")
{
    throw "Global recurring-motif feature is not connected to real motif analysis."
}

if ($Extractor -notmatch "motifs\.averageOccurrenceCount")
{
    throw "Global motif-occurrence feature is not connected to real motif analysis."
}

foreach ($Case in @(
    "testMotifAnalysisIsValid",
    "testMotifFamiliesAreDetected",
    "testMotifAnalysisIsDeterministic",
    "testInvalidInputIsRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 75 test is missing: $Case"
    }
}


$ExtractorTests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiDatasetFeatureExtractorTests.cpp")

if ($ExtractorTests -match 'buildSample\(\s*[\s\S]*?harmony,\s*"(?:real-midi|deterministic)')
{
    throw "Phase 75 extractor tests contain a buildSample call without the motif analysis argument."
}

if ($ExtractorTests -notmatch "const auto motifs\s*=\s*[\s\S]*analyzeCompositionMidiMotifs")
{
    throw "Phase 75 extractor tests do not construct MIDI motif analysis."
}

Write-Host "Phase 75 MIDI motif analysis validation passed." -ForegroundColor Green
