$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$Required = @(
    "Source\Music\PhraseAnalysis.h",
    "Source\Music\PhraseAnalysis.cpp",
    "Source\Domain\GenerationIntent.h"
)

foreach ($RelativePath in $Required)
{
    $Path = Join-Path $Root $RelativePath
    if (-not (Test-Path $Path))
    {
        throw "Missing Phase 14.1 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")

if ($CMake -notmatch "Source/Music/PhraseAnalysis\.cpp")
{
    throw "PhraseAnalysis.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "Source/Domain/GenerationIntent\.h")
{
    throw "GenerationIntent.h is not part of MIDI_GenGX_Domain."
}

$Analysis = Get-Content -Raw (
    Join-Path $Root "Source\Music\PhraseAnalysis.cpp"
)

foreach ($Symbol in @(
    "analyzePhrase",
    "isPhraseContainedInRange",
    "isPhraseScaleContained"
))
{
    if ($Analysis -notmatch $Symbol)
    {
        throw "$Symbol implementation is missing."
    }
}

$Intent = Get-Content -Raw (
    Join-Path $Root "Source\Domain\GenerationIntent.h"
)

if ($Intent -notmatch "struct GenerationIntent")
{
    throw "GenerationIntent contract is missing."
}

if ($Intent -notmatch "void normalize")
{
    throw "GenerationIntent normalization contract is missing."
}

Write-Host "Phase 14.1 architecture validation passed." -ForegroundColor Green
