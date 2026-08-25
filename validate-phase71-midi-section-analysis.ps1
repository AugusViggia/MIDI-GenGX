$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionMidiSection.h",
    "Source\Music\CompositionMidiSectionAnalyzer.h",
    "Source\Music\CompositionMidiSectionAnalyzer.cpp",
    "Source\Tests\CompositionMidiSectionAnalyzerTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 71 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiSectionAnalyzer.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiSectionAnalyzerTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionMidiSectionAnalyzer\.cpp")
{
    throw "CompositionMidiSectionAnalyzer.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionMidiSectionAnalyzerTests")
{
    throw "CompositionMidiSectionAnalyzerTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionMidiSectionAnalyzerTests")
{
    throw "build-x64.ps1 does not execute CompositionMidiSectionAnalyzerTests."
}

if ($Impl -notmatch "barsPerSection")
{
    throw "Phase 71 fixed-size section segmentation is missing."
}

if ($Impl -notmatch "PhraseSection::Opening")
{
    throw "Opening section classification is missing."
}

if ($Impl -notmatch "PhraseSection::Development")
{
    throw "Development section classification is missing."
}

if ($Impl -notmatch "PhraseSection::Cadence")
{
    throw "Cadence section classification is missing."
}

if ($Impl -notmatch "notesPerBeat")
{
    throw "Section note-density extraction is missing."
}

if ($Impl -notmatch "maxPolyphony")
{
    throw "Section polyphony extraction is missing."
}

foreach ($Case in @(
    "testFourBarSegmentation",
    "testSixteenBarStructureProducesFourSections",
    "testSectionBoundariesAreContiguous",
    "testEmptySectionIsNotCreated",
    "testSectionMetricsPropagate",
    "testInvalidRecordIsRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 71 test is missing: $Case"
    }
}


$SectionHeader = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiSection.h")

if ($SectionHeader -notmatch '#include\s+"PhraseStructure\.h"')
{
    throw "CompositionMidiSection.h does not include its PhraseSection dependency."
}


if ($Impl -match "valid\s*=\s*result\.isValid\(")
{
    throw "MIDI section analyzer has a circular valid/isValid finalization."
}

if ($Impl -notmatch "result\.valid\s*=\s*true")
{
    throw "MIDI section analyzer does not explicitly finalize analysis validity."
}

Write-Host "Phase 71 MIDI section analysis validation passed." -ForegroundColor Green
