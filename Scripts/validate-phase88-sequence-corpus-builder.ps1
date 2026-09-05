$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

$RequiredFiles = @(
    "Source\Music\CompositionMidiSequenceCorpusBuilder.h",
    "Source\Music\CompositionMidiSequenceCorpusBuilder.cpp",
    "Source\Music\CompositionMidiTrainingSequence.cpp",
    "Source\Tests\CompositionMidiSequenceCorpusBuilderTests.cpp"
)

foreach ($RelativePath in $RequiredFiles)
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 88 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiSequenceCorpusBuilder.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\CompositionMidiSequenceCorpusBuilderTests.cpp")

if ($CMake -notmatch "Source/Music/CompositionMidiSequenceCorpusBuilder\.cpp")
{
    throw "CompositionMidiSequenceCorpusBuilder.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_CompositionMidiSequenceCorpusBuilderTests")
{
    throw "CompositionMidiSequenceCorpusBuilderTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_CompositionMidiSequenceCorpusBuilderTests")
{
    throw "build-x64.ps1 does not execute Phase 88 tests."
}

foreach ($Required in @(
    "loadCompositionMidiCorpusDirectory",
    "analyzeCompositionMidiSections",
    "buildWholePieceFallbackSections",
    "buildCompositionMidiTrainingSequence",
    "serializeCompositionMidiTrainingSequences"
))
{
    if ($Impl -notmatch [regex]::Escape($Required))
    {
        throw "Phase 88 corpus stage is missing: $Required"
    }
}

# Preferred four-bar analysis must be attempted before fallback inside the
# corpus-building function. The fallback helper itself is declared earlier in
# the translation unit, so a raw file-order comparison would be incorrect.
$BuilderStart = $Impl.IndexOf("buildCompositionMidiSequenceCorpusFromDirectory")
$BuilderEnd = $Impl.IndexOf("} // namespace midigengx::music", $BuilderStart)

if ($BuilderStart -lt 0 -or $BuilderEnd -le $BuilderStart)
{
    throw "Phase 88 corpus-building function could not be located."
}

$BuilderBody = $Impl.Substring(
    $BuilderStart,
    $BuilderEnd - $BuilderStart
)

$PreferredPos = $BuilderBody.IndexOf("analyzeCompositionMidiSections")
$FallbackPos = $BuilderBody.IndexOf("buildWholePieceFallbackSections")

if ($PreferredPos -lt 0 -or $FallbackPos -lt 0 -or $PreferredPos -gt $FallbackPos)
{
    throw "Phase 88 structural path is not ordered as preferred analysis -> fallback."
}

if ($Impl -match "analyzeCompositionMidiMotifs")
{
    throw "Phase 88 corpus builder must not depend on motif analysis."
}

if ($Impl -match "analyzeCompositionMidiHarmony")
{
    throw "Phase 88 corpus acceptance must not depend on harmonic analysis."
}

if ($Impl -notmatch "std::set<std::string>")
{
    throw "Phase 88 duplicate sequence identity protection is missing."
}

if ($Impl -notmatch "std::sort")
{
    throw "Phase 88 deterministic ordering is missing."
}

foreach ($Case in @(
    "testBuildsSequenceCorpus",
    "testNestedIdsAreDeterministic",
    "testMalformedFileIsRejected",
    "testEmptyDirectoryIsRejected",
    "testDeterministicOrderingIsStable"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required Phase 88 test is missing: $Case"
    }
}


$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\CompositionMidiSequenceCorpusBuilder.cpp")

if ($Impl -notmatch "result\.valid\s*=\s*true;\s*\n\s*\n\s*if\s*\(\s*!result\.isValid\(\)")
{
    throw "Phase 88 corpus builder result validity is evaluated before the valid flag is established."
}

if ($Impl -match "result\.valid\s*=\s*result\.isValid\(\)")
{
    throw "Phase 88 corpus builder contains recursive self-validation of the valid flag."
}

Write-Host "Phase 88 sequence corpus builder validation passed." -ForegroundColor Green
