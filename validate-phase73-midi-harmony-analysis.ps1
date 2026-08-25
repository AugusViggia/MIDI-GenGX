$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionMidiHarmony.h",
    "Source\Music\CompositionMidiHarmony.cpp",
    "Source\Tests\CompositionMidiHarmonyTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 73 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiHarmony.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiHarmonyTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionMidiHarmony\.cpp")
{
    throw "CompositionMidiHarmony.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionMidiHarmonyTests")
{
    throw "CompositionMidiHarmonyTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionMidiHarmonyTests")
{
    throw "build-x64.ps1 does not execute CompositionMidiHarmonyTests."
}

if ($Impl -notmatch "majorProfile")
{
    throw "Major key profile analysis is missing."
}

if ($Impl -notmatch "minorProfile")
{
    throw "Minor key profile analysis is missing."
}

if ($Impl -notmatch "ChordQuality")
{
    throw "Section chord-quality inference is missing."
}

if ($Impl -notmatch "Unknown")
{
    throw "Unknown-harmony fallback is missing."
}

if ($Impl -notmatch "scaleDegreeForPitchClass")
{
    throw "Scale-degree mapping is missing."
}

foreach ($Case in @(
    "testKeyIsEstimatedAsCMajor",
    "testSectionHarmonyFindsCMajorTriad",
    "testHarmonyIsDeterministic",
    "testInvalidRecordIsRejected",
    "testAmbiguousSectionDoesNotFabricateUnknownQuality"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 73 test is missing: $Case"
    }
}


$HarmonyTests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiHarmonyTests.cpp")

if ($HarmonyTests -notmatch "record\.lengthTicks\s*=\s*64\s*\*\s*480")
{
    throw "Phase 73 C-major harmony fixture is not long enough for four 4-bar sections."
}

if ($HarmonyTests -notmatch "section\s*\*\s*16\s*\*\s*480")
{
    throw "Phase 73 harmony fixture section spacing is inconsistent with four-bar segmentation."
}

Write-Host "Phase 73 MIDI harmony analysis validation passed." -ForegroundColor Green
