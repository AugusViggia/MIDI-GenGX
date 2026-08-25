$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionMidiCorpusAnalysis.h",
    "Source\Music\CompositionMidiCorpusAnalysis.cpp",
    "Source\Tests\CompositionMidiCorpusAnalysisTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 70 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiCorpusAnalysis.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiCorpusAnalysisTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionMidiCorpusAnalysis\.cpp")
{
    throw "CompositionMidiCorpusAnalysis.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionMidiCorpusAnalysisTests")
{
    throw "CompositionMidiCorpusAnalysisTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionMidiCorpusAnalysisTests")
{
    throw "build-x64.ps1 does not execute CompositionMidiCorpusAnalysisTests."
}

foreach ($Metric in @(
    "notesPerBeat",
    "averagePitch",
    "pitchRange",
    "averageVelocity",
    "averageDurationBeats",
    "maxPolyphony"
))
{
    if ($Impl -notmatch $Metric)
    {
        throw "Required MIDI metric is missing: $Metric"
    }
}

if ($Impl -notmatch "std::sort")
{
    throw "Polyphony event ordering is missing."
}

foreach ($Case in @(
    "testAnalysisIsValid",
    "testBasicMetrics",
    "testDeterministicAnalysis",
    "testInvalidRecordIsRejected",
    "testZeroLengthIsRejected",
    "testOverlappingNotesDrivePolyphony"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 70 test is missing: $Case"
    }
}

Write-Host "Phase 70 MIDI corpus analysis validation passed." -ForegroundColor Green
