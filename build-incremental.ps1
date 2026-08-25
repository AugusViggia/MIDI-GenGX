$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ProjectRoot

$VCTools = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231"
$WindowsKit = "C:\Program Files (x86)\Windows Kits\10"
$WindowsKitVersion = "10.0.26100.0"

$MsvcBinX64 = Join-Path $VCTools "bin\Hostx64\x64"
$MsvcLibX64 = Join-Path $VCTools "lib\x64"
$WindowsUmX64 = Join-Path $WindowsKit "Lib\$WindowsKitVersion\um\x64"
$WindowsUcrtX64 = Join-Path $WindowsKit "Lib\$WindowsKitVersion\ucrt\x64"

foreach ($RequiredPath in @(
    (Join-Path $ProjectRoot "JUCE"),
    $VCTools,
    $MsvcBinX64,
    $MsvcLibX64,
    $WindowsUmX64,
    $WindowsUcrtX64
)) {
    if (-not (Test-Path $RequiredPath)) {
        throw "Required path not found: $RequiredPath"
    }
}

$env:PATH = "$MsvcBinX64;$env:PATH"
$env:LIB = "$MsvcLibX64;$WindowsUcrtX64;$WindowsUmX64"
$env:LIBPATH = "$MsvcLibX64"
$env:VCToolsInstallDir = "$VCTools\"

$BuildDir = Join-Path $ProjectRoot "out\build\x64-Debug"

if (-not (Test-Path $BuildDir)) {
    Write-Host "Configuring x64 Debug..." -ForegroundColor Cyan

    cmake `
        -S $ProjectRoot `
        -B $BuildDir `
        -G "Ninja" `
        -DCMAKE_BUILD_TYPE=Debug `
        -DCMAKE_C_COMPILER="$MsvcBinX64\cl.exe" `
        -DCMAKE_CXX_COMPILER="$MsvcBinX64\cl.exe"

    if ($LASTEXITCODE -ne 0) {
        throw "CMake configure failed."
    }
}
else {
    Write-Host "Reusing existing x64 Debug build..." -ForegroundColor DarkGray
}

Write-Host "Incremental build: only changed files/targets will rebuild..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "Incremental build failed."
}

Write-Host "Running full CTest suite..." -ForegroundColor Cyan

ctest `
    --test-dir $BuildDir `
    --output-on-failure

if ($LASTEXITCODE -ne 0) {
    throw "CTest suite failed."
}

Write-Host "INCREMENTAL FULL BUILD + COMPLETE TEST SUITE PASSED" -ForegroundColor Green
