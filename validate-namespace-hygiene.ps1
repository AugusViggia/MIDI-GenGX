$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path

$SourceFiles = @(
    "Source\Plugin\PluginProcessor.h",
    "Source\Plugin\PluginProcessor.cpp",
    "Source\Plugin\PluginEditor.h",
    "Source\Plugin\PluginEditor.cpp",
    "Source\Music\MusicalEngine.h",
    "Source\Music\MusicalEngine.cpp"
)

foreach ($RelativePath in $SourceFiles) {
    $Path = Join-Path $ProjectRoot $RelativePath

    if (-not (Test-Path $Path)) {
        throw "Required source not found: $Path"
    }

    $Content = Get-Content -Raw $Path

    $UnqualifiedDomain = [regex]::Matches(
        $Content,
        '(?<!midigengx::)\bdomain::'
    )

    $UnqualifiedMusic = [regex]::Matches(
        $Content,
        '(?<!midigengx::)\bmusic::'
    )

    if ($UnqualifiedDomain.Count -gt 0) {
        throw "Unqualified domain:: reference found in $RelativePath"
    }

    if ($UnqualifiedMusic.Count -gt 0) {
        throw "Unqualified music:: reference found in $RelativePath"
    }
}

Write-Host "MIDI-GenGX namespace hygiene check passed." -ForegroundColor Green
