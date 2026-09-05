$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\MotifPhraseComposer.h",
    "Source\Music\MotifPhraseComposer.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 14.4 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
if ($CMake -notmatch "Source/Music/MotifPhraseComposer\.cpp")
{
    throw "MotifPhraseComposer.cpp is not part of MIDI_GenGX_Music."
}

$Composer = Get-Content -Raw (
    Join-Path $Root "Source\Music\MotifPhraseComposer.cpp"
)

foreach ($Symbol in @(
    "MotifPhraseComposer::compose",
    "MotifDevelopment::transpose",
    "MotifDevelopment::stretchTime",
    "MotifDevelopment::invert"
))
{
    if ($Composer -notmatch [regex]::Escape($Symbol))
    {
        throw "$Symbol is missing from Phase 14.4 composer implementation."
    }
}

$Tests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\MusicalEngineTests.cpp"
)

if ($Tests -notmatch "testMotifPhraseComposer")
{
    throw "Motif phrase composer tests are missing."
}

Write-Host "Phase 14.4 motif phrase composer validation passed." -ForegroundColor Green
