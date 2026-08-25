param(
    [Parameter(Mandatory = $true)]
    [string]$MidiDirectory,

    [Parameter(Mandatory = $true)]
    [string]$MetadataFile,

    [Parameter(Mandatory = $true)]
    [string]$OutputModel,

    [int]$Epochs = 25,

    [double]$LearningRate = 0.001,

    [double]$GradientClip = 1.0,

    [switch]$NonRecursive
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildDir = Join-Path $Root "out\build\x64-Debug"
$Cli = Join-Path $BuildDir "MIDI_GenGX_CompositionRealComposerTrainingCli.exe"

if (-not (Test-Path $Cli))
{
    throw "Training CLI not built: $Cli"
}

$Arguments = @(
    "--midi-dir", $MidiDirectory,
    "--metadata", $MetadataFile,
    "--output-model", $OutputModel,
    "--epochs", $Epochs,
    "--learning-rate", $LearningRate,
    "--gradient-clip", $GradientClip
)

if ($NonRecursive)
{
    $Arguments += "--non-recursive"
}

& $Cli @Arguments

if ($LASTEXITCODE -ne 0)
{
    throw "Real composer training CLI failed with exit code $LASTEXITCODE."
}

Write-Host "REAL COMPOSER MODEL ARTIFACT CREATED" -ForegroundColor Green
Write-Host $OutputModel
