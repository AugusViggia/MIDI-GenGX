$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$Service = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedCorpusTrainingService.cpp")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionConditionedCorpusTrainingService.h")

foreach ($Required in @(
    "CompositionComposerKnowledgeSample.h",
    "CompositionComposerKnowledgeCatalog.h",
    "CompositionComposerKnowledgeTrainingCorpus.h",
    "CompositionComposerKnowledgePartition.h"
))
{
    if (-not (Test-Path (Join-Path $Root "Source\Music\$Required")))
    {
        throw "Phase 106 dependency is missing: $Required"
    }
}

foreach ($Required in @(
    "buildCompositionComposerKnowledgeCatalog",
    "buildCompositionComposerKnowledgePartition",
    "buildCompositionComposerKnowledgeCorpusManifest",
    "buildCompositionComposerKnowledgeTrainingCorpus",
    "trainingSampleCount",
    "validationSampleCount",
    "testSampleCount"
))
{
    if ($Service -notmatch [regex]::Escape($Required))
    {
        throw "Phase 106 training split integration is missing: $Required"
    }
}

if ($Header -notmatch "trainingSampleCount" -or
    $Header -notmatch "validationSampleCount" -or
    $Header -notmatch "testSampleCount")
{
    throw "Phase 106 result does not expose split cardinalities."
}

if ($Service -notmatch "trainingCorpus\.trainingSamples")
{
    throw "Phase 106 does not train from the established training partition."
}

Write-Host "Phase 106 training split boundary validation passed." -ForegroundColor Green
