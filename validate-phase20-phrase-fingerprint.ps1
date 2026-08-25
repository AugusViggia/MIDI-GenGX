$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\PhraseFingerprint.h",
    "Source\Music\PhraseFingerprint.cpp",
    "Source\Tests\PhraseFingerprintTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 20 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\PhraseFingerprint.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\PhraseFingerprint.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\PhraseFingerprintTests.cpp")

if ($CMake -notmatch "Source/Music/PhraseFingerprint\.cpp")
{
    throw "PhraseFingerprint.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_PhraseFingerprintTests")
{
    throw "PhraseFingerprintTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_PhraseFingerprintTests")
{
    throw "build-x64.ps1 does not execute PhraseFingerprintTests."
}

if ($Header -notmatch "struct PhraseFingerprint")
{
    throw "PhraseFingerprint model is missing."
}

if ($Impl -notmatch "fingerprintPhrase")
{
    throw "PhraseFingerprint implementation is missing."
}

if ($Impl -notmatch "stationary == motionCount")
{
    throw "Stationary melodic-profile classification guard is missing."
}

foreach ($Case in @(
    "testAscendingFingerprint",
    "testRepeatedRhythmAndPitch",
    "testScaleFailureAndPolyphony",
    "testRegisterNormalization"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required phrase-fingerprint test is missing: $Case"
    }
}

Write-Host "Phase 20 phrase-fingerprint validation passed." -ForegroundColor Green
