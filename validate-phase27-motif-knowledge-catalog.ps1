$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\MotifKnowledgeCatalog.h",
    "Source\Music\MotifKnowledgeCatalog.cpp",
    "Source\Tests\MotifKnowledgeCatalogTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 27 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\MotifKnowledgeCatalog.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\MotifKnowledgeCatalog.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\MotifKnowledgeCatalogTests.cpp")

if ($CMake -notmatch "Source/Music/MotifKnowledgeCatalog\.cpp")
{
    throw "MotifKnowledgeCatalog.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_MotifKnowledgeCatalogTests")
{
    throw "MotifKnowledgeCatalogTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_MotifKnowledgeCatalogTests")
{
    throw "build-x64.ps1 does not execute MotifKnowledgeCatalogTests."
}

if ($Header -notmatch "struct MotifKnowledgeCatalog")
{
    throw "Motif knowledge catalog model is missing."
}

if ($Impl -notmatch "buildMotifKnowledgeCatalog")
{
    throw "Motif knowledge catalog builder is missing."
}

if ($Impl -notmatch "lower_bound")
{
    throw "Catalog lookup is not using deterministic ordered lookup."
}

foreach ($Case in @(
    "testCatalogValidity",
    "testDeterministicOrderingAndLookup",
    "testRecurringStatistics",
    "testInvalidProfileProducesEmptyCatalog"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required catalog test is missing: $Case"
    }
}

Write-Host "Phase 27 motif-knowledge-catalog validation passed." -ForegroundColor Green
