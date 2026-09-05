$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$Header = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginEditor.h")
$Editor = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginEditor.cpp")

if ($Header -match "LOAD AI MODEL|aiModelButton|showAIModelFileChooser")
{
    throw "UI still exposes a user-facing AI model loading control."
}

foreach ($Required in @(
    "GenerationCpuMode",
    "generationCpuModeBox",
    "generationCpuLabel",
    "configureGenerationCpuMode",
    "handleGenerationCpuModeChanged",
    "showHighCpuWarning"
))
{
    if ($Header -notmatch [regex]::Escape($Required))
    {
        throw "CPU mode UI contract is missing: $Required"
    }
}

# AI GENERATE is declared in the editor header, so validate it there.
if ($Header -notmatch [regex]::Escape("AI GENERATE"))
{
    throw "UI generation contract is missing: AI GENERATE"
}

foreach ($Required in @(
    "LOW  •  2 CORES",
    "BALANCED  •  4 CORES",
    "HIGH  •  6 CORES",
    "PRO  •  8 CORES",
    "PRO CPU MODE",
    "HIGH CPU MODE",
    "up to 8 CPU cores",
    "up to 6 CPU cores",
    "AI MODEL NOT EMBEDDED"
))
{
    if ($Editor -notmatch [regex]::Escape($Required))
    {
        throw "UI generation contract is missing: $Required"
    }
}

Write-Host "AI generation and CPU mode UI validation passed." -ForegroundColor Green
