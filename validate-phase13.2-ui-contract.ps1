$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Header = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginEditor.h")
$Editor = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginEditor.cpp")

if ($Header -notmatch "std::function\s*<\s*void\s*\(\s*int\s*\)\s*>\s+onChange;")
{
    throw "DownwardSelector onChange callback contract is missing."
}

if ($Header -notmatch "#include\s*<functional>")
{
    throw "PluginEditor.h must explicitly include <functional>."
}

if ($Editor -match "(?m)\bbutton->setFont\s*\(")
{
    throw "Unsupported TextButton::setFont() call remains in Popup::showPopup()."
}

if ($Editor -notmatch "genreBox\.onChange\s*=\s*\[this\]\s*\(\s*int\s+genreIndex\s*\)")
{
    throw "Genre selector does not use the typed DownwardSelector callback."
}

if ($Editor -notmatch "if\s*\(\s*onChange\s*\)")
{
    throw "DownwardSelector callback dispatch is missing."
}

$callbackBlock = [regex]::Match(
    $Editor,
    "(?s)void\s+MIDIGenGXAudioProcessorEditor::DownwardSelector::\s*setSelectedIndex\s*\(.*?\n\}"
)

if (-not $callbackBlock.Success)
{
    throw "setSelectedIndex() could not be located."
}

$matchesOwner = ([regex]::Matches(
    $callbackBlock.Value,
    "owner\.selectorValueChanged\(\)"
)).Count

if ($matchesOwner -ne 1)
{
    throw "Selector callback fallback must be dispatched exactly once."
}

Write-Host "Phase 13.2 UI contract validation passed." -ForegroundColor Green
