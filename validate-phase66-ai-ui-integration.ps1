$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Plugin\PluginEditor.h",
    "Source\Plugin\PluginEditor.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 66 file: $RelativePath"
    }
}

$Header = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginEditor.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginEditor.cpp")

foreach ($Token in @(
    "aiModelButton",
    "aiGenerateButton",
    "aiStatusLabel",
    "showAIModelFileChooser",
    "updateAIControls"
))
{
    if ($Header -notmatch $Token)
    {
        throw "Phase 66 UI declaration is missing: $Token"
    }
}

if ($Impl -notmatch "showAIModelFileChooser")
{
    throw "AI model file chooser implementation is missing."
}

if ($Impl -notmatch "loadAIRuntimeModelFromFile")
{
    throw "AI model file chooser is not connected to processor loading."
}

if ($Impl -notmatch "requestAIRuntimeGeneration")
{
    throw "AI Generate button is not connected to the AI runtime trigger."
}

if ($Impl -notmatch "AI MODEL READY")
{
    throw "AI model ready state is not represented in the UI."
}

if ($Impl -notmatch "AI GENERATING")
{
    throw "AI generation state is not represented in the UI."
}

if ($Impl -notmatch "aiGenerateButton\.setEnabled")
{
    throw "AI Generate button is not gated by model availability."
}

Write-Host "Phase 66 AI UI integration validation passed." -ForegroundColor Green
