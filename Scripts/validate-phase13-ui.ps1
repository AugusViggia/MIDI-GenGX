$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Editor = Get-Content -Raw (
    Join-Path $Root "Source\Plugin\PluginEditor.cpp"
)
$Header = Get-Content -Raw (
    Join-Path $Root "Source\Plugin\PluginEditor.h"
)
$Domain = Get-Content -Raw (
    Join-Path $Root "Source\Domain\GenrePresets.cpp"
)
$Params = Get-Content -Raw (
    Join-Path $Root "Source\Domain\MusicalParameters.h"
)

if ($Params -notmatch "OrganicHouse")
{
    throw "OrganicHouse enum is missing."
}

if ($Domain -notmatch "organicHouseValues")
{
    throw "Organic House profile is missing."
}

if ($Editor -notmatch 'genres\.add\("Organic House"\)')
{
    throw "Organic House selector entry is missing."
}

if ($Header -notmatch "std::array<juce::TextButton, 20> infoButtons")
{
    throw "Expected 20 contextual help buttons."
}

if ($Editor -notmatch "CallOutBox")
{
    throw "Contextual help must use CallOutBox."
}

if ($Editor -match "actionPanel")
{
    throw "Obsolete action panel remains."
}

if ($Editor -notmatch "baseHeight = 750")
{
    throw "Compact 750px base editor height is missing."
}

if ($Editor -notmatch "generatorButton\.setBounds")
{
    throw "Generator button layout is missing."
}

if ($Editor -notmatch "showInfoPopup")
{
    throw "Contextual help implementation is missing."
}

Write-Host "Phase 13 UI validation passed." -ForegroundColor Green
