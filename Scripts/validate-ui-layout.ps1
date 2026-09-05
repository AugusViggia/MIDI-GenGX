$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Editor = Get-Content -Raw -LiteralPath (
    Join-Path $Root "Source\Plugin\PluginEditor.cpp"
)
$Header = Get-Content -Raw -LiteralPath (
    Join-Path $Root "Source\Plugin\PluginEditor.h"
)

if ($Editor -notmatch '"GENRE"[\s\S]*"KEY"[\s\S]*"SCALE"[\s\S]*"ROLE"[\s\S]*"LENGTH"[\s\S]*"OCTAVE LOW"[\s\S]*"OCTAVE HIGH"[\s\S]*"OCTAVE SHIFT"[\s\S]*"NOTE LENGTH"[\s\S]*"PHRASE CONTOUR"[\s\S]*"CADENCE"')
{
    throw "Selector labels are not in the canonical 4/4/3 order."
}

if ($Editor -notmatch "const int row3Width")
{
    throw "Third selector row does not use a three-column layout."
}

if ($Header -notmatch "TextButton settingsButton")
{
    throw "Settings button declaration is missing."
}

if ($Editor -notmatch "genreBox\.onChange")
{
    throw "Genre selector callback is missing."
}

if ($Editor -notmatch "applyGenrePreset")
{
    throw "Genre selector is not connected to applyGenrePreset()."
}

if ($Editor -notmatch "syncControlsFromProcessor")
{
    throw "Genre preset UI synchronization is missing."
}

if ($Editor -notmatch "showSettingsMenu")
{
    throw "Settings popup implementation is missing."
}

if ($Editor -notmatch "setUiZoom")
{
    throw "UI zoom implementation is missing."
}

if ($Editor -notmatch '"75%"[\s\S]*"85%"[\s\S]*"100%"[\s\S]*"115%"[\s\S]*"130%"[\s\S]*"150%"')
{
    throw "Expected zoom levels are missing."
}

Write-Host "UI layout and zoom validation passed." -ForegroundColor Green
