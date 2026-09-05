$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$Service = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedCorpusTrainingService.cpp")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedCorpusTrainingService.h")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionConditionedCorpusTrainingServiceTests.cpp")

foreach ($Required in @(
    "Source\Music\CompositionRealComposerCorpusPreparation.h",
    "Source\Music\CompositionRealComposerCorpusPreparation.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $Required)))
    {
        throw "Phase 105 dependency is missing: $Required"
    }
}

$ServiceText = $Service

if ($ServiceText -notmatch "CompositionMidiTrainingCorpusArtifact\.h")
{
    throw "Phase 105 training service is missing the MIDI training corpus artifact header."
}

foreach ($Required in @(
    "prepareRealComposerCorpusFromRecords",
    "serializeCompositionMidiTrainingSequences",
    "loadCompositionMidiCorpusDirectory",
    "CompositionRealComposerCorpusPreparation"
))
{
    if ($Service -notmatch [regex]::Escape($Required))
    {
        throw "Phase 105 training service is not using the real composer preparation path: $Required"
    }
}

if ($ServiceText -notmatch "CompositionMidiCorpusDirectoryLoader\.h")
{
    throw "Phase 105 training service is missing the MIDI corpus directory loader header."
}

if ($ServiceText -notmatch "metadataSampleIds" -or
    $ServiceText -notmatch "midiSampleIds")
{
    throw "Phase 105 training service is missing the exact MIDI-to-metadata coverage gate."
}

if ($Header -match "CompositionMidiSequenceCorpusBuilder")
{
    throw "Phase 105 service header still depends directly on the legacy sparse sequence corpus builder."
}

if ($Tests -notmatch "testServiceConsumesRealComposerPreparationPath")
{
    throw "Phase 105 regression test is missing."
}

Write-Host "Phase 105 real composer training path validation passed." -ForegroundColor Green
