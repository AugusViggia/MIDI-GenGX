$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$H = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginProcessor.h")
$CPP = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginProcessor.cpp")

if ($CPP -match '\bactiveNote\b') {
    throw "Stale undefined activeNote symbol remains. Use activeNoteMidi."
}

if ($CPP -notmatch 'activeNoteMidi\.exchange|activeNoteMidi >= 0') {
    throw "Stop logic does not reference activeNoteMidi."
}

if ($CPP -notmatch 'controllerEvent\(\s*midiChannel,\s*123,\s*0') {
    throw "CC123 All Notes Off is missing."
}

Write-Host "Stop source validation passed." -ForegroundColor Green
