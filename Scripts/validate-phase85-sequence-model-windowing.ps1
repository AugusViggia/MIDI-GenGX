$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionSequenceLearningContract.h",
    "Source\Music\CompositionSequenceLearningContract.cpp",
    "Source\Music\CompositionMidiSequenceWindow.h",
    "Source\Music\CompositionMidiSequenceWindow.cpp",
    "Source\Tests\CompositionMidiSequenceWindowTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 85 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiSequenceWindow.cpp")
$Contract = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionSequenceLearningContract.h")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiSequenceWindowTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionMidiSequenceWindow\.cpp")
{
    throw "CompositionMidiSequenceWindow.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionMidiSequenceWindowTests")
{
    throw "CompositionMidiSequenceWindowTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionMidiSequenceWindowTests")
{
    throw "build-x64.ps1 does not execute CompositionMidiSequenceWindowTests."
}

if ($Contract -notmatch "NextEventPrediction")
{
    throw "Phase 85 next-event learning objective is missing."
}

if ($Contract -notmatch "contextLength\s*=\s*64")
{
    throw "Phase 85 default sequence context length is not 64."
}

foreach ($Required in @(
    "buildCompositionMidiSequenceWindows",
    "paddingMask",
    "targetIndex",
    "firstContextIndex"
))
{
    if ($Impl -notmatch $Required)
    {
        throw "Phase 85 sequence windowing component is missing: $Required"
    }
}

foreach ($Case in @(
    "testContractIsValid",
    "testWindowCount",
    "testWindowShape",
    "testInitialPaddingIsExplicit",
    "testDeterministicWindowing",
    "testInvalidContractRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 85 test is missing: $Case"
    }
}


$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiSequenceWindow.cpp")

if ($Impl -notmatch "targetCount\s*=\s*\n\s*sequence\.events\.size\(\)\s*-\s*1")
{
    throw "Phase 85 window builder does not derive the next-event target count from sequence length."
}

if ($Impl -notmatch "targetIndex\s*<=\s*targetCount")
{
    throw "Phase 85 window builder does not cover every next-event target."
}


$WindowImpl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiSequenceWindow.cpp")

if ($WindowImpl -notmatch "window\.valid\s*=\s*true;\s*\n\s*if\s*\(\s*!window\.isValid")
{
    throw "Phase 85 builder validates a candidate window before marking it valid."
}

Write-Host "Phase 85 sequence model windowing validation passed." -ForegroundColor Green
