$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Header = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginEditor.h")
$Editor = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginEditor.cpp")

if ($Header -match "InfoContent")
{
    throw "Legacy InfoContent declaration remains."
}

if ($Header -match "CallOutBox")
{
    throw "Legacy CallOutBox ownership remains."
}

if ($Header -notmatch "class InfoPopup\s*:\s*public\s+juce::Component")
{
    throw "InfoPopup declaration is missing."
}

if ($Editor -notmatch "activeInfoPopup->setBounds\(\s*x,\s*y,\s*popupWidth,\s*popupHeight")
{
    throw "Editor-local popup placement is missing."
}

if ($Editor -notmatch "visible\.getBottom\(\)")
{
    throw "Popup containment logic is missing."
}

if ($Editor -notmatch "getBoundsInParent\(\)")
{
    throw "Popup anchor is not derived from the clicked field."
}

if ($Editor -match "activeInfoCallout")
{
    throw "Legacy CallOutBox implementation remains."
}

if ($Header -notmatch "std::function<void\(int\)>\s+onChange;")
{
    throw "Phase 13.2 selector callback contract was not preserved."
}

Write-Host "Phase 13.2.1 popup integration validation passed." -ForegroundColor Green
