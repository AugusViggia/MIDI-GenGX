$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$Loader = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionSequenceMetadataFileLoader.cpp")
$Cli = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionRealComposerTrainingCli.cpp")
$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")

foreach ($Required in @(
    "Source\Music\CompositionSequenceMetadataFileLoader.h",
    "Source\Music\CompositionSequenceMetadataFileLoader.cpp",
    "Source\Music\CompositionRealComposerTrainingCli.cpp",
    "Source\Tests\CompositionSequenceMetadataFileLoaderTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $Required)))
    {
        throw "Missing Phase 109 file: $Required"
    }
}

foreach ($Required in @(
    "loadCompositionSequenceMetadataFile",
    "parsedEntryCount",
    "rejectedLineCount",
    "buildCompositionSequenceMetadataCatalog"
))
{
    if ($Loader -notmatch [regex]::Escape($Required))
    {
        throw "Phase 109 metadata loader component is missing: $Required"
    }
}

foreach ($Required in @(
    "--midi-dir",
    "--metadata",
    "--output-model",
    "runCompositionConditionedCorpusTraining",
    "writeBinary",
    "validationLoss",
    "testLoss"
))
{
    if ($Cli -notmatch [regex]::Escape($Required))
    {
        throw "Phase 109 CLI component is missing: $Required"
    }
}

if ($CMake -notmatch "MIDI_GenGX_CompositionRealComposerTrainingCli")
{
    throw "Phase 109 training CLI target is missing."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionSequenceMetadataFileLoaderTests")
{
    throw "Phase 109 metadata loader test target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionRealComposerTrainingCli" -or
    $Build -notmatch "MIDI_GenGX_CompositionSequenceMetadataFileLoaderTests")
{
    throw "build-x64.ps1 does not build Phase 109 targets."
}


$LoaderImplementation = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionSequenceMetadataFileLoader.cpp")

if ($LoaderImplementation -notmatch "metadata\.valid\s*=\s*true")
{
    throw "Phase 109 metadata loader does not finalize parsed metadata before validation."
}


if ($LoaderImplementation -notmatch "isspace")
{
    throw "Phase 109 metadata loader does not normalize CRLF whitespace in the verified field."
}

Write-Host "Phase 109 real composer training CLI validation passed." -ForegroundColor Green
