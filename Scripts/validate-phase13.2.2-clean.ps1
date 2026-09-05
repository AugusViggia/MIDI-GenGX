$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$EditorH = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginEditor.h")
$EditorCpp = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginEditor.cpp")
$ProcessorH = Get-Content -Raw (Join-Path $Root "Source\Plugin\PluginProcessor.h")

if ($EditorH -notmatch "^#pragma once") { throw "PluginEditor.h must start with #pragma once." }
if ($EditorH -match "#include <functional>\s*#include <array>") { throw "PluginEditor.h standard includes are not normalized." }
if ($EditorH -match "void mouseDown\([^\)]*\) override;\s*\n\s{12}void mouseUp") { throw "PluginEditor.h contains malformed selector indentation." }
if ($EditorCpp -notmatch '#include "PluginEditor.h"\s*\r?\n\s*\r?\n#include <cmath>') { throw "PluginEditor.cpp include order is not normalized." }
if ($EditorCpp -notmatch "constexpr float kMinUiZoom = 0\.75f;") { throw "UI zoom minimum constant is missing." }
if ($EditorCpp -notmatch "constexpr float kMaxUiZoom = 1\.50f;") { throw "UI zoom maximum constant is missing." }
if ($EditorCpp -notmatch "constexpr int kPopupRowHeight = 28;") { throw "Popup row-height constant is missing." }
if ($EditorCpp -match "jlimit\(0\.75f,\s*1\.50f") { throw "Duplicate UI zoom literals remain in PluginEditor.cpp." }
if ($EditorCpp -notmatch "const float clampedScale") { throw "Selector UI scale should use a const clamped value." }
if ($EditorCpp -match "InfoContent|activeInfoContent|activeInfoCallout") { throw "Legacy popup implementation remains." }
if ($ProcessorH -notmatch "#include <array>\s*\r?\n#include <atomic>\s*\r?\n#include <cstdint>\s*\r?\n#include <memory>") { throw "PluginProcessor.h standard includes are not normalized." }

Write-Host "Phase 13.2.2 clean-code validation passed." -ForegroundColor Green
