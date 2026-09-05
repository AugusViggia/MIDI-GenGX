$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\MotifFingerprint.h",
    "Source\Music\MotifFingerprint.cpp",
    "Source\Tests\MotifFingerprintTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 21 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\MotifFingerprint.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\MotifFingerprint.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\MotifFingerprintTests.cpp")

if ($CMake -notmatch "Source/Music/MotifFingerprint\.cpp")
{
    throw "MotifFingerprint.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_MotifFingerprintTests")
{
    throw "MotifFingerprintTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_MotifFingerprintTests")
{
    throw "build-x64.ps1 does not execute MotifFingerprintTests."
}

if ($Header -notmatch "struct MotifFingerprint")
{
    throw "MotifFingerprint model is missing."
}

if ($Impl -notmatch "fingerprintMotif")
{
    throw "MotifFingerprint implementation is missing."
}

if ($Impl -notmatch "isMotivicIdentityEquivalent")
{
    throw "Motif identity comparison is missing."
}

foreach ($Case in @(
    "testFingerprintCapturesIdentity",
    "testTranspositionPreservesIdentity",
    "testRhythmicChangeBreaksIdentity",
    "testIntervalChangeBreaksIdentity",
    "testInvalidMotifFingerprint"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required motif-fingerprint test is missing: $Case"
    }
}

Write-Host "Phase 21 motif-fingerprint validation passed." -ForegroundColor Green
