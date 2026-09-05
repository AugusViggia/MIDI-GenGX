$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$CMakePath = Join-Path $Root "CMakeLists.txt"
$CMake = Get-Content -Raw -LiteralPath $CMakePath

function Require([string] $Condition, [string] $Message)
{
    if (-not $Condition)
    {
        throw $Message
    }
}

function Get-SetBlock([string] $VariableName)
{
    $Pattern = "(?s)set\(\s*" +
               [regex]::Escape($VariableName) +
               "\s+(.*?)\)"
    $Match = [regex]::Match($CMake, $Pattern)

    if (-not $Match.Success)
    {
        throw "CMake variable not found: $VariableName"
    }

    return $Match.Groups[1].Value
}

function Get-TargetBlock([string] $Command, [string] $TargetName)
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

$DomainSources = Get-SetBlock "MIDI_GENGX_DOMAIN_SOURCES"
$MusicSources = Get-SetBlock "MIDI_GENGX_MUSIC_SOURCES"

Require ($DomainSources -match "Source/Domain/GenrePresets\.cpp") `
    "GenrePresets.cpp is missing from MIDI_GENGX_DOMAIN_SOURCES."

Require ($DomainSources -match "Source/Domain/GenrePresets\.h") `
    "GenrePresets.h is missing from MIDI_GENGX_DOMAIN_SOURCES."

Require ($MusicSources -match "Source/Music/MusicalEngine\.cpp") `
    "MusicalEngine.cpp is missing from MIDI_GENGX_MUSIC_SOURCES."

$DomainTarget = Get-TargetBlock "add_library" "MIDI_GenGX_Domain"
$MusicTarget = Get-TargetBlock "add_library" "MIDI_GenGX_Music"
$DomainTests = Get-TargetBlock "add_executable" "MIDI_GenGX_DomainTests"
$MusicTests = Get-TargetBlock "add_executable" "MIDI_GenGX_MusicalEngineTests"
$PluginTarget = Get-TargetBlock "target_sources" "MIDI_GenGX"

# The actual project uses source variables. The target checks therefore verify
# that each target consumes the correct variable, rather than requiring the
# expanded source filename to be repeated inside add_library().
Require ($DomainTarget -match "MIDI_GENGX_DOMAIN_SOURCES") `
    "MIDI_GenGX_Domain does not consume MIDI_GENGX_DOMAIN_SOURCES."

Require ($MusicTarget -match "MIDI_GENGX_MUSIC_SOURCES") `
    "MIDI_GenGX_Music does not consume MIDI_GENGX_MUSIC_SOURCES."

Require ($CMake -match '(?s)target_link_libraries\(\s*MIDI_GenGX_Music\b.*?MIDI_GenGX_Domain') `
    "MIDI_GenGX_Music does not link MIDI_GenGX_Domain."

Require ($DomainTests -match "Source/Tests/DomainTests\.cpp") `
    "DomainTests target does not use Source/Tests/DomainTests.cpp."

Require ($CMake -match '(?s)target_link_libraries\(\s*MIDI_GenGX_DomainTests\b.*?MIDI_GenGX_Domain') `
    "DomainTests target does not link MIDI_GenGX_Domain."

Require ($MusicTests -match "Source/Tests/MusicalEngineTests\.cpp") `
    "MusicalEngineTests target does not use Source/Tests/MusicalEngineTests.cpp."

Require ($CMake -match '(?s)target_link_libraries\(\s*MIDI_GenGX_MusicalEngineTests\b.*?MIDI_GenGX_Music') `
    "MusicalEngineTests target does not link MIDI_GenGX_Music."

Require ($PluginTarget -match "Source/Plugin/PluginProcessor\.cpp") `
    "Plugin target does not consume PluginProcessor.cpp."

Require ($PluginTarget -match "Source/Plugin/PluginEditor\.cpp") `
    "Plugin target does not consume PluginEditor.cpp."

Require ($PluginTarget -match "Source/Generation/PhraseGenerationWorker\.cpp") `
    "Plugin target does not consume PhraseGenerationWorker.cpp."

Require ($CMake -match '(?s)target_link_libraries\(\s*MIDI_GenGX\b.*?MIDI_GenGX_Music') `
    "Plugin target does not link MIDI_GenGX_Music."

Write-Host "CMake source-graph validation passed." -ForegroundColor Green
