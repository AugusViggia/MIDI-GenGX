$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionAIMidiGenerationService.h",
    "Source\Music\CompositionAIMidiGenerationService.cpp",
    "Source\Tests\CompositionAIMidiGenerationServiceTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 82 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionAIMidiGenerationService.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionAIMidiGenerationServiceTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionAIMidiGenerationService\.cpp")
{
    throw "CompositionAIMidiGenerationService.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionAIMidiGenerationServiceTests")
{
    throw "CompositionAIMidiGenerationServiceTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionAIMidiGenerationServiceTests")
{
    throw "build-x64.ps1 does not execute CompositionAIMidiGenerationServiceTests."
}

foreach ($Required in @(
    "generateNextSections",
    "guidanceFromSection",
    "MusicalEngine",
    "generateWithAIGuidance"
))
{
    if ($Impl -notmatch $Required)
    {
        throw "Phase 82 AI MIDI generation stage is missing: $Required"
    }
}

foreach ($Case in @(
    "testGeneratedFeaturesBecomeRealMidiPhrases",
    "testGenerationIsDeterministic",
    "testUserConstraintsRemainAuthoritative",
    "testInvalidInputsAreRejected"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 82 test is missing: $Case"
    }
}


$ScaleHeader = Get-Content -Raw (Join-Path $Root "Source\Domain\Scale.h")
$ImplScaleCheck = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionAIMidiGenerationService.cpp")

if ($ImplScaleCheck -match "scale\.isValid\(\)")
{
    throw "Phase 82 still calls nonexistent Scale::isValid()."
}

if ($ScaleHeader -notmatch "getPitchClasses")
{
    throw "Phase 82 validator cannot confirm the supported Scale API."
}


$GenerationTests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionAIMidiGenerationServiceTests.cpp")

if ($GenerationTests -notmatch "expectedLowMidi")
{
    throw "Phase 82 octave regression test does not derive the expected MIDI lower bound from the current internal convention."
}

if ($GenerationTests -notmatch "expectedHighMidi")
{
    throw "Phase 82 octave regression test does not derive the expected MIDI upper bound from the current internal convention."
}

if ($GenerationTests -match 'note\.midiNote\s*>=\s*24\s*&&\s*note\.midiNote\s*<=\s*60')
{
    throw "Phase 82 octave regression test still contains the obsolete hard-coded 24-60 range."
}

Write-Host "Phase 82 AI MIDI generation validation passed." -ForegroundColor Green
