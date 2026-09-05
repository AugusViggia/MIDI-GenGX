$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Tests = Get-Content -Raw (
    Join-Path $Root "Source\Tests\DomainTests.cpp"
)

if ($Tests -notmatch "void testGenreProfiles\(\)")
{
    throw "testGenreProfiles() was removed from DomainTests."
}

if ($Tests -notmatch "testGenreProfiles\(\);")
{
    throw "testGenreProfiles() is not executed by DomainTests."
}

if ($Tests -notmatch "testScales\(\);")
{
    throw "Extended scale regression tests are not executed."
}

if ($Tests -notmatch "testAllGenreProfiles\(\);")
{
    throw "All-genre regression tests are not executed."
}

Write-Host "Phase 14.7.1 DomainTests regression validation passed." -ForegroundColor Green
