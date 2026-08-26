$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Header = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginEditor.h")
$Editor = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginEditor.cpp")

if ($Header -match "LOAD AI MODEL|aiModelButton|juce::ComboBox\s+generationCpuModeBox" -or
    $Editor -match "LOAD AI MODEL|aiModelButton|showAIModelFileChooser")
{
    throw "User-facing AI model loading UI or ComboBox CPU control is still present."
}

foreach ($Required in @(
    "juce::ScrollBar",
    "mouseWheelMove",
    "scrollBarMoved",
    "const bool needsScroll",
    "CpuWarningOverlay",
    "scanButton",
    "scanResultLabel",
    "scanCpuCapability",
    "getDetectedLogicalProcessorCount",
    "GenerationCpuMode::low"
))
{
    if (($Header + $Editor) -notmatch [regex]::Escape($Required))
    {
        throw "Required UI/runtime element is missing: $Required"
    }
}

foreach ($Required in @(
    "Detected ",
    "Recommended mode:",
    "SCAN SYSTEM",
    "RESCAN SYSTEM",
    "CANCEL",
    "CONTINUE",
    "GetActiveProcessorCount(ALL_PROCESSOR_GROUPS)"
))
{
    if ($Editor -notmatch [regex]::Escape($Required))
    {
        throw "CPU scan popup behavior is missing: $Required"
    }
}

if ($Editor -match "generationCpuCapabilityLabel")
{
    throw "CPU capability information must not be permanently displayed in the main plugin UI."
}

Write-Host "Selector scroll and popup CPU scan validation passed." -ForegroundColor Green
