$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Path = Join-Path $Root "Source\Tests\DomainTests.cpp"
$Text = Get-Content -Raw $Path

if ($Text -notmatch '(?s)namespace\s*\{.*?\}\s*//\s*namespace\s*\r?\n\r?\nint\s+main\s*\(')
{
    throw "DomainTests anonymous namespace is not explicitly closed before main()."
}

$Open = ([regex]::Matches($Text, "\{")).Count
$Close = ([regex]::Matches($Text, "\}")).Count

if ($Open -ne $Close)
{
    throw "DomainTests.cpp has unbalanced braces: $Open open vs $Close close."
}

foreach ($TestName in @(
    "testKey",
    "testScales",
    "testGenreProfiles",
    "testAllGenreProfiles"
))
{
    if ($Text -notmatch "void\s+$TestName\s*\(")
    {
        throw "$TestName() definition is missing."
    }

    if ($Text -notmatch "$TestName\s*\(\);")
    {
        throw "$TestName() is not executed by main()."
    }
}

if ($Text -notmatch "ScaleType::Arabic" -or
    $Text -notmatch "ScaleType::Hungarian")
{
    throw "Extended world-scale coverage is missing."
}

Write-Host "Phase 14.7.2 DomainTests structural validation passed." -ForegroundColor Green
