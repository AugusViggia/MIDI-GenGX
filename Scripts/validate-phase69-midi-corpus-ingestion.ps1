$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionMidiCorpusRecord.h",
    "Source\Music\CompositionMidiCorpusRecord.cpp",
    "Source\Music\CompositionMidiFileCorpusReader.h",
    "Source\Music\CompositionMidiFileCorpusReader.cpp",
    "Source\Tests\CompositionMidiFileCorpusReaderTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 69 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiFileCorpusReader.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiFileCorpusReaderTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionMidiFileCorpusReader\.cpp")
{
    throw "CompositionMidiFileCorpusReader.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionMidiFileCorpusReaderTests")
{
    throw "CompositionMidiFileCorpusReaderTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionMidiFileCorpusReaderTests")
{
    throw "build-x64.ps1 does not execute CompositionMidiFileCorpusReaderTests."
}

if ($Impl -notmatch "'M'\s*\|\|?" -and
    $Impl -notmatch "data\[0\]\s*!=\s*'M'")
{
    throw "MIDI header parsing is missing."
}

if ($Impl -notmatch "data\[0\]\s*!=\s*'M'" -or
    $Impl -notmatch "data\[3\]\s*!=\s*'d'")
{
    throw "MIDI header chunk validation is missing."
}

if ($Impl -notmatch "data\[cursor\]\s*!=\s*'M'" -or
    $Impl -notmatch "data\[cursor\s*\+\s*3\]\s*!=\s*'k'")
{
    throw "MIDI track chunk parsing is missing."
}

if ($Impl -notmatch "readVlq")
{
    throw "MIDI variable-length quantity parsing is missing."
}

if ($Impl -notmatch "0x90u")
{
    throw "MIDI note-on parsing is missing."
}

if ($Impl -notmatch "0x80u")
{
    throw "MIDI note-off parsing is missing."
}

foreach ($Case in @(
    "testReadsSimpleMidi",
    "testDeterministicRead",
    "testInvalidHeaderIsRejected",
    "testNullInputIsRejected",
    "testTruncatedTrackIsRejected",
    "testInvalidSampleIdIsRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 69 test is missing: $Case"
    }
}


if ($Impl -match "analysisValid\s*=\s*result\.isValid\(\)")
{
    throw "MIDI reader has a circular analysisValid/isValid assignment."
}

if ($Impl -notmatch "result\.analysisValid\s*=\s*true")
{
    throw "MIDI reader does not explicitly finalize analysis validity."
}

Write-Host "Phase 69 MIDI corpus ingestion validation passed." -ForegroundColor Green
