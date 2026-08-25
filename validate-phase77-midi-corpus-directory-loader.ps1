$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\CompositionMidiCorpusDirectoryLoader.h",
    "Source\Music\CompositionMidiCorpusDirectoryLoader.cpp",
    "Source\Tests\CompositionMidiCorpusDirectoryLoaderTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 77 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiCorpusDirectoryLoader.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiCorpusDirectoryLoaderTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionMidiCorpusDirectoryLoader\.cpp")
{
    throw "CompositionMidiCorpusDirectoryLoader.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionMidiCorpusDirectoryLoaderTests")
{
    throw "CompositionMidiCorpusDirectoryLoaderTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionMidiCorpusDirectoryLoaderTests")
{
    throw "build-x64.ps1 does not execute CompositionMidiCorpusDirectoryLoaderTests."
}

foreach ($Required in @(
    "std::filesystem",
    "recursive_directory_iterator",
    "isMidiExtension",
    "CompositionMidiFileCorpusReader",
    "sampleIdForPath",
    "std::sort"
))
{
    if ($Impl -notmatch [regex]::Escape($Required))
    {
        throw "Required directory-ingestion capability is missing: $Required"
    }
}

foreach ($Case in @(
    "testNonRecursiveDiscovery",
    "testRecursiveDiscovery",
    "testCaseInsensitiveExtension",
    "testMissingDirectoryIsRejected",
    "testEmptyDirectoryIsValid",
    "testDeterministicDiscovery"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 77 test is missing: $Case"
    }
}


if ($Impl -match "const auto relative\s*=\s*\n\s*fs::relative")
{
    throw "sampleIdForPath keeps the relative filesystem path const before replace_extension()."
}

if ($Impl -notmatch "relative\.replace_extension\(\)")
{
    throw "sampleIdForPath does not normalize the MIDI extension."
}

Write-Host "Phase 77 MIDI corpus directory loader validation passed." -ForegroundColor Green
