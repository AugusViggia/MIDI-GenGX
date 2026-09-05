
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$Header = Get-Content -Raw (
    Join-Path $Root "Source\Domain\AbletonOctaveConvention.h"
)

$Editor = Get-Content -Raw (
    Join-Path $Root "Source\Plugin\PluginEditor.cpp"
)

$DomainTests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\DomainTests.cpp"
)

$MusicTests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\MusicalEngineTests.cpp"
)

if ($Header -notmatch "abletonOctaveToInternal")
{
    throw "Ableton octave-to-internal conversion is missing."
}

if ($Header -notmatch "internalToAbletonOctave")
{
    throw "Internal-to-Ableton octave conversion is missing."
}

if ($Header -notmatch "midiForC")
{
    throw "Ableton C-to-MIDI mapping is missing."
}

if ($Editor -notmatch "\.\./Domain/AbletonOctaveConvention\.h")
{
    throw "PluginEditor.cpp does not include the centralized Ableton octave convention."
}

if ($Editor -notmatch "internalToAbletonOctave")
{
    throw "Plugin UI does not use the centralized Ableton octave convention."
}

if ($Header -notmatch "minAbletonOctave = -2")
{
    throw "Ableton octave minimum must remain -2."
}

if ($Header -notmatch "maxAbletonOctave = 8")
{
    throw "Ableton octave maximum must be 8."
}

if ($Editor -notmatch 'octaves\.add\(\s*juce::String\(\s*abletonOctave\s*\)')
{
    throw "Octave selector must expose numeric octave values without a C prefix."
}

if ($DomainTests -notmatch "testAbletonOctaveConvention")
{
    throw "Domain octave-convention tests are missing."
}

if ($MusicTests -notmatch "testConfiguredAbletonOctaveRange")
{
    throw "Musical engine Ableton-range integration test is missing."
}

Write-Host "Phase 19.1 Ableton octave convention validation passed." -ForegroundColor Green
