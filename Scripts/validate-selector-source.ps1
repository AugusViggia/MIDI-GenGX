$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$H = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginEditor.h")
$CPP = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginEditor.cpp")

if ($H -match 'std::function<') {
    throw "Selector callback architecture must not use std::function."
}

$hideCount = ([regex]::Matches($H, 'void hidePopup\(bool notify = false\);')).Count
if ($hideCount -ne 1) {
    throw "hidePopup must have exactly one public declaration."
}

if ($CPP -match 'onChange') {
    throw "Legacy selector onChange references remain."
}

if ($CPP -notmatch 'owner\.selectorValueChanged\(\)') {
    throw "Selector does not notify its owning editor directly."
}

if ($CPP -match 'closeModifierKeys') {
    throw "Stale closeModifierKeys symbol found."
}

if ($CPP -match 'juce::ignoreUnused\(\s*\)') {
    throw "Invalid empty juce::ignoreUnused() call found."
}

Write-Host "Selector architecture validation passed." -ForegroundColor Green
