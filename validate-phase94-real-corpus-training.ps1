$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$RequiredFiles = @(
    "Source\Music\CompositionConditionedCorpusTrainingService.h",
    "Source\Music\CompositionConditionedCorpusTrainingService.cpp",
    "Source\Tests\CompositionConditionedCorpusTrainingServiceTests.cpp"
)

foreach ($RelativePath in $RequiredFiles)
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 94 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Service = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedCorpusTrainingService.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionConditionedCorpusTrainingServiceTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionConditionedCorpusTrainingService\.cpp")
{
    throw "Phase 94 corpus training service is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionConditionedCorpusTrainingServiceTests")
{
    throw "Phase 94 test target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionConditionedCorpusTrainingServiceTests")
{
    throw "build-x64.ps1 does not execute Phase 94 tests."
}

foreach ($Required in @(
    "buildCompositionMidiSequenceCorpusFromDirectory",
    "serializeCompositionSequenceMetadataCatalog",
    "runCompositionConditionedTraining"
))
{
    if ($Service -notmatch [regex]::Escape($Required))
    {
        throw "Phase 94 training path is missing: $Required"
    }
}

if ($Service -match "composerId\s*=" -or
    $Service -match "styleId\s*=")
{
    throw "Phase 94 service must not infer composer/style metadata."
}

foreach ($Case in @(
    "testRealDirectoryToTraining",
    "testMissingMetadataFailsClosed",
    "testUnverifiedCatalogFailsClosed"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 94 test is missing: $Case"
    }
}


$Loader = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiCorpusDirectoryLoader.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionConditionedCorpusTrainingServiceTests.cpp")

if ($Loader -notmatch "sampleIdForPath")
{
    throw "Phase 94 validator cannot verify the canonical corpus sample-ID contract."
}

if ($Loader -notmatch "relative\.generic_string")
{
    throw "Phase 94 canonical sample IDs are not derived from normalized relative MIDI paths."
}

$MetadataBuilder = $Tests.Substring(
    $Tests.IndexOf("CompositionSequenceMetadataCatalog buildMetadata"),
    $Tests.IndexOf("void testRealDirectoryToTraining") -
        $Tests.IndexOf("CompositionSequenceMetadataCatalog buildMetadata")
)

if ($MetadataBuilder -match 'makeMetadata\(\s*\(root\s*/')
{
    throw "Phase 94 metadata builder still uses corpus filesystem paths as sample IDs."
}


$Loader = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiCorpusDirectoryLoader.cpp")

if ($Loader -notmatch "lexically_relative")
{
    throw "Phase 94 loader does not derive sample IDs from deterministic lexical relative paths."
}

if ($Loader -match "fs::relative\(")
{
    throw "Phase 94 loader still uses filesystem-resolved relative identity."
}


$CorpusTest = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionConditionedCorpusTrainingServiceTests.cpp")

if ($CorpusTest -notmatch 'CompositionMidiCorpusDirectoryLoader\.h')
{
    throw "Phase 94 corpus regression test is missing the explicit directory-loader dependency."
}


$CorpusTest = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionConditionedCorpusTrainingServiceTests.cpp")

if ($CorpusTest -notmatch 'root\s*/\s*"Nested"\s*/\s*"nested-piece\.midi"')
{
    throw "Phase 94 nested MIDI fixture is not actually created inside the Nested directory."
}

Write-Host "Phase 94 real corpus training validation passed." -ForegroundColor Green
