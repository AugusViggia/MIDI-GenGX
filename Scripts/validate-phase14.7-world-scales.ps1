$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$Header = Get-Content -Raw (Join-Path $Root "Source\Domain\Scale.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Domain\Scale.cpp")
$Editor = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginEditor.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\DomainTests.cpp")

if ($Header -notmatch "\bCount\s*,")
{
    throw "ScaleType::Count boundary is missing."
}

foreach ($Name in @("Arabic","Rumanian","Hindu","Spanish","Hungarian"))
{
    if ($Header -notmatch "ScaleType::$Name" -and
        $Header -notmatch "\b$Name\s*,")
    {
        throw "$Name enum value is missing."
    }

    if ($Impl -notmatch "ScaleType::$Name")
    {
        throw "$Name interval/name implementation is missing."
    }
}

if ($Editor -notmatch "ScaleType::Count")
{
    throw "Scale selector is not derived from ScaleType::Count."
}

foreach ($Name in @("Arabic","Rumanian","Hindu","Spanish","Hungarian"))
{
    if ($Tests -notmatch "ScaleType::$Name")
    {
        throw "$Name regression test is missing."
    }
}

Write-Host "Phase 14.7 world-scale validation passed." -ForegroundColor Green
