$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\PhraseDevelopmentPlan.h",
    "Source\Music\PhraseDevelopmentPlan.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 14.16 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")

if ($CMake -notmatch "Source/Music/PhraseDevelopmentPlan\.cpp")
{
    throw "PhraseDevelopmentPlan.cpp is not part of MIDI_GenGX_Music."
}

$Composer = Get-Content -Raw (
    Join-Path $Root "Source\Music\MotifPhraseComposer.cpp"
)

if ($Composer -notmatch "planPhraseDevelopment")
{
    throw "MotifPhraseComposer does not consume PhraseDevelopmentPlan."
}

if ($Composer -match "const int direction\s*=\s*\(phraseIndex % 2")
{
    throw "Legacy phrase-index parity development heuristic is still embedded in composer."
}

$Plan = Get-Content -Raw (
    Join-Path $Root "Source\Music\PhraseDevelopmentPlan.cpp"
)

foreach ($Symbol in @(
    "planPhraseDevelopment",
    "PhraseDevelopmentPlan::isValid"
))
{
    if ($Plan -notmatch [regex]::Escape($Symbol))
    {
        throw "$Symbol implementation is missing."
    }
}

$Tests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\MusicalEngineTests.cpp"
)

if ($Tests -notmatch "testPhraseDevelopmentPlan")
{
    throw "Phrase development regression coverage is missing."
}

if ($Plan -notmatch "two-phrase form")
{
    throw "Short-form cadence development contract is missing."
}

if ($Plan -notmatch "cadenceDevelopment")
{
    throw "Cadence development path is missing."
}

Write-Host "Phase 14.16 phrase-development validation passed." -ForegroundColor Green
