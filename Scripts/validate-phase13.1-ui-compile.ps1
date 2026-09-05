$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Header = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginEditor.h")
$Editor = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginEditor.cpp")

if ($Header -notmatch "std::function\s*<\s*void\s*\(\s*int\s*\)\s*>\s+onChange;")
{
    throw "DownwardSelector onChange callback declaration is missing."
}

if ($Header -notmatch "#include\s*<functional>")
{
    throw "PluginEditor.h must explicitly include <functional>."
}

if ($Editor -match "(?m)\b(?:button|settingsButton|generatorButton)\.setFont\s*\(")
{
    throw "Unsupported TextButton::setFont() call remains."
}

if ($Editor -notmatch "genreBox\.onChange\s*=\s*\[this\]\s*\(\s*int\s+genreIndex\s*\)")
{
    throw "Genre selector callback must use the DownwardSelector int callback."
}

if ($Editor -notmatch "if\s*\(\s*onChange\s*\)")
{
    throw "DownwardSelector callback dispatch is missing."
}

Write-Host "Phase 13.1 UI compile validation passed." -ForegroundColor Green
