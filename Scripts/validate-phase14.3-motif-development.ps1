$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\MotifDevelopment.h",
    "Source\Music\MotifDevelopment.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 14.3 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
if ($CMake -notmatch "Source/Music/MotifDevelopment\.cpp")
{
    throw "MotifDevelopment.cpp is not part of MIDI_GenGX_Music."
}

$Implementation = Get-Content -Raw (
    Join-Path $Root "Source\Music\MotifDevelopment.cpp"
)

foreach ($Symbol in @(
    "MotifDevelopment::transpose",
    "MotifDevelopment::invert",
    "MotifDevelopment::stretchTime",
    "MotifDevelopment::repeat",
    "MotifDevelopment::transposeAndStretch"
))
{
    if ($Implementation -notmatch [regex]::Escape($Symbol))
    {
        throw "$Symbol implementation is missing."
    }
}

$Tests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\MusicalEngineTests.cpp"
)

if ($Tests -notmatch "testMotifDevelopment")
{
    throw "Motif development tests are missing."
}

Write-Host "Phase 14.3 motif-development validation passed." -ForegroundColor Green
