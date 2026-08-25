$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

foreach ($RelativePath in @(
    "Source\Music\MotifRecurrenceMetrics.h",
    "Source\Music\MotifRecurrenceMetrics.cpp",
    "Source\Tests\MotifRecurrenceMetricsTests.cpp"
))
{
    if (-not (Test-Path (Join-Path $Root $RelativePath)))
    {
        throw "Missing Phase 25 file: $RelativePath"
    }
}

$CMake = Get-Content -Raw (Join-Path $Root "CMakeLists.txt")
$Build = Get-Content -Raw (Join-Path $Root "build-x64.ps1")
$Header = Get-Content -Raw (Join-Path $Root "Source\Music\MotifRecurrenceMetrics.h")
$Impl = Get-Content -Raw (Join-Path $Root "Source\Music\MotifRecurrenceMetrics.cpp")
$Tests = Get-Content -Raw (Join-Path $Root "Source\Tests\MotifRecurrenceMetricsTests.cpp")
$Profile = Get-Content -Raw (Join-Path $Root "Source\Music\MotifRecurrenceProfile.cpp")
$ProfileHeader = Get-Content -Raw (Join-Path $Root "Source\Music\MotifRecurrenceProfile.h")

if ($CMake -notmatch "Source/Music/MotifRecurrenceMetrics\.cpp")
{
    throw "MotifRecurrenceMetrics.cpp is not part of MIDI_GenGX_Music."
}

if ($CMake -notmatch "MIDI_GenGX_MotifRecurrenceMetricsTests")
{
    throw "MotifRecurrenceMetricsTests target is missing."
}

if ($Build -notmatch "MIDI_GenGX_MotifRecurrenceMetricsTests")
{
    throw "build-x64.ps1 does not execute MotifRecurrenceMetricsTests."
}

if ($Header -notmatch "enum class MotifRecurrencePattern")
{
    throw "Motif recurrence pattern vocabulary is missing."
}

if ($Impl -notmatch "calculateMotifRecurrenceMetrics")
{
    throw "Motif recurrence metrics implementation is missing."
}

if ($ProfileHeader -notmatch "phraseIndices")
{
    throw "Recurrence family lacks source phrase positions."
}

if ($Profile -notmatch "family\.phraseIndices\.push_back")
{
    throw "Recurrence profile does not preserve phrase positions."
}

foreach ($Case in @(
    "testSingleOccurrence",
    "testPeriodicRecurrence",
    "testClusteredRecurrence",
    "testTransformationRate",
    "testNullFamily"
))
{
    if ($Tests -notmatch $Case)
    {
        throw "Required recurrence-metrics test is missing: $Case"
    }
}


$RhythmPlan = Get-Content -Raw (
    Join-Path $Root "Source\Music\RhythmPlan.cpp"
)

$MusicalEngine = Get-Content -Raw (
    Join-Path $Root "Source\Music\MusicalEngine.cpp"
)

$Composer = Get-Content -Raw (
    Join-Path $Root "Source\Music\MotifPhraseComposer.cpp"
)

$OccurrenceGraph = Get-Content -Raw (
    Join-Path $Root "Source\Music\MotifOccurrenceGraph.cpp"
)

if ($RhythmPlan -match 'static_cast<void>\(feel\)')
{
    throw "RhythmPlan still contains the ignored-feel suppression patch."
}

if ($MusicalEngine -match 'static_cast<void>\(rootPitchClass\)')
{
    throw "MusicalEngine still contains the ignored-root-pitch suppression patch."
}

if ($Composer -match 'static_cast<void>\(seed\)')
{
    throw "MotifPhraseComposer still contains the ignored-seed suppression patch."
}

if ($OccurrenceGraph -match 'fingerprintMotif\(\s*motifs\[i\]\)\.isValid')
{
    throw "MotifOccurrenceGraph still recalculates source fingerprints."
}

if ($Profile -match 'numeric_limits<std::size_t>::max')
{
    throw "MotifRecurrenceProfile still contains the impossible size_t bound check."
}

Write-Host "Phase 25 motif-recurrence metrics validation passed." -ForegroundColor Green
