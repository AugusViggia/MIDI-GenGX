$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$Rhythm = Get-Content -Raw (
    Join-Path $Root "Source\Music\RhythmPlan.cpp"
)

$Header = Get-Content -Raw (
    Join-Path $Root "Source\Music\RhythmPlan.h"
)

$Tests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\MusicalEngineTests.cpp"
)

if ($Header -notmatch "int\s+noteLengthVariation\s*=")
{
    throw "RhythmPlan does not expose note-length variation."
}

if ($Rhythm -match "if\s*\(\s*plan\.density\s*>=")
{
    throw "Density is still used as a duration-shaping condition."
}

if ($Rhythm -notmatch "plan\.noteLengthVariation")
{
    throw "Note duration variation is not driven by NoteLengthVariation."
}

if ($Rhythm -notmatch "durationForNoteLength")
{
    throw "Base duration is not derived from NoteLength semantics."
}

if ($Tests -notmatch "density does not alter note duration")
{
    throw "Density/duration separation regression test is missing."
}

if ($Tests -notmatch "short note length produces short duration")
{
    throw "Short-note duration regression test is missing."
}

Write-Host "Phase 14.9.1 duration semantics validation passed." -ForegroundColor Green
