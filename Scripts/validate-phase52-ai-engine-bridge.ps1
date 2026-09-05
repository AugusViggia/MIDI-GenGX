$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionAIEngineBridge.h",
    "Source\Music\CompositionAIEngineBridge.cpp",
    "Source\Tests\CompositionAIEngineBridgeTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 52 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionAIEngineBridge.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionAIEngineBridge.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionAIEngineBridgeTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionAIEngineBridge\.cpp")
{
    throw "CompositionAIEngineBridge.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionAIEngineBridgeTests")
{
    throw "CompositionAIEngineBridgeTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionAIEngineBridgeTests")
{
    throw "build-x64.ps1 does not execute CompositionAIEngineBridgeTests."
}

if ($Header -notmatch "struct CompositionAIGuidance")
{
    throw "AI guidance representation is missing."
}

if ($Impl -notmatch "deriveGuidance")
{
    throw "AI engine bridge guidance derivation is missing."
}

if ($Impl -notmatch "enabled")
{
    throw "AI bridge enable/disable boundary is missing."
}

foreach ($Case in @(
    "testEnabledBridgeIsValid",
    "testDisabledBridgeIsInvalid",
    "testGuidancePreservesAllTargets",
    "testConfidenceIsPreserved",
    "testInvalidProfileIsRejected",
    "testDisabledBridgeProducesNoGuidance",
    "testGuidanceIsDeterministic"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required AI-engine-bridge test is missing: $Case"
    }
}

Write-Host "Phase 52 AI-engine-bridge validation passed." -ForegroundColor Green
