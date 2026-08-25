$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionAIGenerationCoordinator.h",
    "Source\Music\CompositionAIGenerationCoordinator.cpp",
    "Source\Tests\CompositionAIGenerationCoordinatorTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 54 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionAIGenerationCoordinator.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionAIGenerationCoordinator.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionAIGenerationCoordinatorTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionAIGenerationCoordinator\.cpp")
{
    throw "CompositionAIGenerationCoordinator.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionAIGenerationCoordinatorTests")
{
    throw "CompositionAIGenerationCoordinatorTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionAIGenerationCoordinatorTests")
{
    throw "build-x64.ps1 does not execute CompositionAIGenerationCoordinatorTests."
}

if ($Header -notmatch "struct CompositionAIGenerationCoordinator")
{
    throw "AI generation coordinator is missing."
}

if ($Impl -notmatch "pipeline\.infer")
{
    throw "Coordinator is not connected to the inference pipeline."
}

if ($Impl -notmatch "generateWithAIGuidance")
{
    throw "Coordinator is not connected to the MusicalEngine AI path."
}

if ($Impl -notmatch "adaptAIResultToMusicalConstraints")
{
    throw "Coordinator is not connected to the AI constraint adapter."
}

foreach ($Case in @(
    "testCoordinatorIsValid",
    "testCoordinatorUsesAI",
    "testCoordinatorIsDeterministic",
    "testDisabledCoordinatorDoesNotUseAI",
    "testEnabledCoordinatorRejectsInvalidAIRequest",
    "testInvalidRequestIsRejected",
    "testExplicitContextRemainsValid"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 54 test is missing: $Case"
    }
}


if ($Impl -notmatch 'if \(!enabled\)')
{
    throw "Disabled coordinator fallback path is missing."
}

if ($Impl -notmatch 'engine\.generate\(')
{
    throw "Disabled coordinator does not fall back to the baseline MusicalEngine."
}

if ($Tests -notmatch "testEnabledCoordinatorRejectsInvalidAIRequest")
{
    throw "Enabled-mode AI request validation regression test is missing."
}

Write-Host "Phase 54 AI generation coordinator validation passed." -ForegroundColor Green
