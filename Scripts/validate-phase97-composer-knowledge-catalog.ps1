$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$RequiredFiles = @(
    "Source\Music\CompositionComposerKnowledgeCatalog.h",
    "Source\Music\CompositionComposerKnowledgeCatalog.cpp",
    "Source\Tests\CompositionComposerKnowledgeCatalogTests.cpp"
)

foreach ($RelativePath in $RequiredFiles)
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 97 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Implementation = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionComposerKnowledgeCatalog.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionComposerKnowledgeCatalogTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionComposerKnowledgeCatalog\.h" -or
    $CMake -notmatch "Source/Music/CompositionComposerKnowledgeCatalog\.cpp")
{
    throw "Phase 97 composer knowledge catalog is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionComposerKnowledgeCatalogTests")
{
    throw "Phase 97 test target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionComposerKnowledgeCatalogTests")
{
    throw "build-x64.ps1 does not execute Phase 97 tests."
}

foreach ($Required in @(
    "buildCompositionComposerKnowledgeCatalog",
    "composerId",
    "verified",
    "findComposer"
))
{
    if ($Implementation -notmatch [regex]::Escape($Required))
    {
        throw "Phase 97 catalog component is missing: $Required"
    }
}

foreach ($Case in @(
    "testCatalogGroupsByComposer",
    "testComposerLookup",
    "testUnverifiedMetadataFailsByDefault",
    "testUnverifiedMetadataCanBeRepresented"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 97 test is missing: $Case"
    }
}


if ($Implementation -notmatch "std::all_of")
{
    throw "Phase 97 catalog does not derive its verification state from its samples."
}

Write-Host "Phase 97 composer knowledge catalog validation passed." -ForegroundColor Green
