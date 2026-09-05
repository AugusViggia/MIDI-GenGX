$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path

function Require-File([string] $RelativePath)
{
    $Path = Join-Path $ProjectRoot $RelativePath

    if (-not (Test-Path -LiteralPath $Path -PathType Leaf))
    {
        throw "Missing required project file: $RelativePath"
    }

    return $Path
}

$requiredFiles = @(
    "CMakeLists.txt",
    "build-x64.ps1",

    "Source\Domain\Key.h",
    "Source\Domain\Scale.h",
    "Source\Domain\Scale.cpp",
    "Source\Domain\Role.h",
    "Source\Domain\Style.h",
    "Source\Domain\Character.h",
    "Source\Domain\MusicalParameters.h",
    "Source\Domain\MusicalContext.h",
    "Source\Domain\GenrePresets.h",
    "Source\Domain\GenrePresets.cpp",

    "Source\Music\NoteEvent.h",
    "Source\Music\Phrase.h",
    "Source\Music\MusicalEngine.h",
    "Source\Music\MusicalEngine.cpp",

    "Source\Generation\PhraseGenerationWorker.h",
    "Source\Generation\PhraseGenerationWorker.cpp",

    "Source\Plugin\PluginProcessor.h",
    "Source\Plugin\PluginProcessor.cpp",
    "Source\Plugin\PluginEditor.h",
    "Source\Plugin\PluginEditor.cpp",

    "Source\Tests\DomainTests.cpp",
    "Source\Tests\MusicalEngineTests.cpp"
)

foreach ($RelativePath in $requiredFiles)
{
    Require-File $RelativePath | Out-Null
}

$CMake = Get-Content -Raw -LiteralPath (
    Join-Path $ProjectRoot "CMakeLists.txt"
)

function Get-CMakeTargetBlock([string] $Command, [string] $TargetName)
{
    $Pattern = "(?s)" +
               [regex]::Escape($Command) +
               "\(\s*" +
               [regex]::Escape($TargetName) +
               "\b(.*?)\)"

    $Match = [regex]::Match($CMake, $Pattern)

    if (-not $Match.Success)
    {
        throw "CMake target not found: $TargetName"
    }

    return $Match.Groups[1].Value
}

function Get-CMakeSetBlock([string] $VariableName)
{
    $Pattern = "(?s)set\(\s*" +
               [regex]::Escape($VariableName) +
               "\s+(.*?)\)"
    $Match = [regex]::Match($CMake, $Pattern)

    if (-not $Match.Success)
    {
        throw "CMake source variable not found: $VariableName"
    }

    return $Match.Groups[1].Value
}

$DomainSources = Get-CMakeSetBlock "MIDI_GENGX_DOMAIN_SOURCES"
$MusicSources = Get-CMakeSetBlock "MIDI_GENGX_MUSIC_SOURCES"

if ($DomainSources -notmatch "Source/Domain/GenrePresets\.cpp")
{
    throw "GenrePresets.cpp is missing from MIDI_GENGX_DOMAIN_SOURCES."
}

$DomainBlock = Get-CMakeTargetBlock "add_library" "MIDI_GenGX_Domain"
$MusicBlock = Get-CMakeTargetBlock "add_library" "MIDI_GenGX_Music"
$DomainTestsBlock = Get-CMakeTargetBlock "add_executable" "MIDI_GenGX_DomainTests"
$MusicTestsBlock = Get-CMakeTargetBlock "add_executable" "MIDI_GenGX_MusicalEngineTests"
$PluginBlock = Get-CMakeTargetBlock "target_sources" "MIDI_GenGX"

if ($DomainBlock -notmatch "MIDI_GENGX_DOMAIN_SOURCES")
{
    throw "MIDI_GenGX_Domain does not consume MIDI_GENGX_DOMAIN_SOURCES."
}

if ($MusicSources -notmatch "Source/Music/MusicalEngine\.cpp")
{
    throw "MusicalEngine.cpp is missing from MIDI_GENGX_MUSIC_SOURCES."
}

if ($MusicBlock -notmatch "MIDI_GENGX_MUSIC_SOURCES")
{
    throw "MIDI_GenGX_Music does not consume MIDI_GENGX_MUSIC_SOURCES."
}

if ($CMake -notmatch '(?s)target_link_libraries\(\s*MIDI_GenGX_Music\b.*?MIDI_GenGX_Domain')
{
    throw "MIDI_GenGX_Music does not link MIDI_GenGX_Domain."
}

if ($DomainTestsBlock -notmatch "Source/Tests/DomainTests\.cpp")
{
    throw "DomainTests target does not use Source/Tests/DomainTests.cpp."
}

if ($CMake -notmatch '(?s)target_link_libraries\(\s*MIDI_GenGX_DomainTests\b.*?MIDI_GenGX_Domain')
{
    throw "DomainTests target does not link MIDI_GenGX_Domain."
}

if ($MusicTestsBlock -notmatch "Source/Tests/MusicalEngineTests\.cpp")
{
    throw "MusicalEngineTests target does not use Source/Tests/MusicalEngineTests.cpp."
}

if ($CMake -notmatch '(?s)target_link_libraries\(\s*MIDI_GenGX_MusicalEngineTests\b.*?MIDI_GenGX_Music')
{
    throw "MusicalEngineTests target does not link MIDI_GenGX_Music."
}

if ($PluginBlock -notmatch "Source/Plugin/PluginProcessor\.cpp")
{
    throw "Plugin target is missing PluginProcessor.cpp."
}

if ($PluginBlock -notmatch "Source/Plugin/PluginEditor\.cpp")
{
    throw "Plugin target is missing PluginEditor.cpp."
}

if ($PluginBlock -notmatch "Source/Generation/PhraseGenerationWorker\.cpp")
{
    throw "Plugin target is missing PhraseGenerationWorker.cpp."
}

if ($CMake -notmatch '(?s)target_link_libraries\(\s*MIDI_GenGX\b.*?MIDI_GenGX_Music')
{
    throw "Plugin target does not link MIDI_GenGX_Music."
}

$Processor = Get-Content -Raw -LiteralPath (
    Join-Path $ProjectRoot "Source\Plugin\PluginProcessor.cpp"
)

$ProcessorHeader = Get-Content -Raw -LiteralPath (
    Join-Path $ProjectRoot "Source\Plugin\PluginProcessor.h"
)

$Worker = Get-Content -Raw -LiteralPath (
    Join-Path $ProjectRoot "Source\Generation\PhraseGenerationWorker.cpp"
)

$Editor = Get-Content -Raw -LiteralPath (
    Join-Path $ProjectRoot "Source\Plugin\PluginEditor.cpp"
)

$EditorHeader = Get-Content -Raw -LiteralPath (
    Join-Path $ProjectRoot "Source\Plugin\PluginEditor.h"
)

# The audio callback must not instantiate or invoke the musical generator.
$ProcessBlockMatch = [regex]::Match(
    $Processor,
    "(?s)void\s+MIDIGenGXAudioProcessor::processBlock\s*\(.*?\n\}"
)

if (-not $ProcessBlockMatch.Success)
{
    throw "Could not locate processBlock() for realtime validation."
}

$ProcessBlock = $ProcessBlockMatch.Value

if ($ProcessBlock -match "MusicalEngine")
{
    throw "processBlock() directly references MusicalEngine. Generation must stay off the realtime thread."
}

if ($ProcessBlock -match "generate\s*\(")
{
    throw "processBlock() directly invokes generation. This is not realtime-safe."
}

if ($ProcessBlock -match "make_unique")
{
    throw "processBlock() performs dynamic object creation."
}

if ($ProcessBlock -match "new\s+")
{
    throw "processBlock() contains dynamic allocation."
}

if ($ProcessBlock -match "std::vector")
{
    throw "processBlock() contains std::vector usage."
}

if ($ProcessBlock -match "sleep\s*\(")
{
    throw "processBlock() contains a sleep/wait operation."
}

# Generation must be delegated to the worker.
if ($Processor -notmatch "PhraseGenerationWorker")
{
    throw "Processor does not reference PhraseGenerationWorker."
}

if ($Worker -notmatch "juce::Thread")
{
    throw "PhraseGenerationWorker is not a JUCE worker thread."
}

if ($Worker -notmatch "MusicalEngine")
{
    throw "PhraseGenerationWorker does not own the musical generation operation."
}

# Active MIDI note state belongs to the processor and must be explicitly
# stopped. We validate the state actually used by this project instead of
# requiring an unrelated auxiliary flag.
if ($ProcessorHeader -notmatch "activeNoteMidi")
{
    throw "Active MIDI note state is missing."
}

if ($Processor -notmatch "stopActiveNote")
{
    throw "stopActiveNote() is missing."
}

if ($Processor -notmatch "noteOff")
{
    throw "No MIDI Note Off path found."
}

# Selector UX contract: the editor and its custom nested components all declare
# the mouseDown overrides that are implemented in PluginEditor.cpp.
$MouseDownDeclarations = (
    [regex]::Matches(
        $EditorHeader,
        "void\s+mouseDown\(const\s+juce::MouseEvent&\)\s+override;"
    )
).Count

if ($MouseDownDeclarations -lt 3)
{
    throw "Expected editor + DownwardSelector + Popup mouseDown declarations."
}

if ($Editor -notmatch "MIDIGenGXAudioProcessorEditor::mouseDown")
{
    throw "Editor mouseDown implementation is missing."
}

if ($Editor -notmatch "(?s)DownwardSelector::\s*mouseDown\s*\(\s*const\s+juce::MouseEvent&") {
    throw "DownwardSelector mouseDown implementation is missing."
}

if ($Editor -notmatch "(?s)DownwardSelector::\s*Popup::\s*mouseDown\s*\(\s*const\s+juce::MouseEvent&") {
    throw "DownwardSelector::Popup mouseDown implementation is missing."
}

Write-Host "Production architecture validation passed." -ForegroundColor Green
