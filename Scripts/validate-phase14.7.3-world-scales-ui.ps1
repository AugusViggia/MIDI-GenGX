$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$Editor = Get-Content -Raw (
    Join-Path $Root "Source\Plugin\PluginEditor.cpp"
)

$Scale = Get-Content -Raw (
    Join-Path $Root "Source\Domain\Scale.cpp"
)

$ScaleHeader = Get-Content -Raw (
    Join-Path $Root "Source\Domain\Scale.h"
)

$DomainTests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\DomainTests.cpp"
)

foreach ($Entry in @(
    "case 12: return ""Arabic"";",
    "case 13: return ""Rumanian"";",
    "case 14: return ""Hindu"";",
    "case 15: return ""Spanish"";",
    "case 16: return ""Hungarian"";"
))
{
    if ($Editor -notmatch [regex]::Escape($Entry))
    {
        throw "Scale UI mapping is missing: $Entry"
    }
}

if ($Editor -match "case 1[2-6]: return ""Custom"";")
{
    throw "Extended world scales still fall back to Custom in the UI."
}

if ($ScaleHeader -notmatch "Count")
{
    throw "ScaleType::Count boundary is missing."
}

foreach ($Name in @(
    "Arabic",
    "Rumanian",
    "Hindu",
    "Spanish",
    "Hungarian"
))
{
    if ($Scale -notmatch "ScaleType::$Name")
    {
        throw "$Name scale is missing from the domain implementation."
    }

    if ($DomainTests -notmatch "ScaleType::$Name")
    {
        throw "$Name scale regression coverage is missing."
    }
}

Write-Host "Phase 14.7.3 world-scale UI validation passed." -ForegroundColor Green
