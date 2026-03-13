#Requires -RunAsAdministrator

param(
    [string]$VcpkgDir = "$env:USERPROFILE\vcpkg"
)

$ErrorActionPreference = "Stop"

function Invoke-Winget {
    # Runs winget and ignores exit code 1 ("no upgrade available" / "already installed")
    param([string[]]$Arguments)
    $global:LASTEXITCODE = 0
    winget @Arguments
    if ($LASTEXITCODE -ne 0 -and $LASTEXITCODE -ne 1) {
        throw "winget exited with code $LASTEXITCODE"
    }
}

function Add-ToUserPath {
    param([string]$NewPath)
    $current = [System.Environment]::GetEnvironmentVariable("PATH", "User")
    if ($current -notlike "*$NewPath*") {
        [System.Environment]::SetEnvironmentVariable("PATH", "$current;$NewPath", "User")
        $env:PATH += ";$NewPath"
        Write-Host "  Added to PATH: $NewPath" -ForegroundColor Green
    } else {
        Write-Host "  Already in PATH: $NewPath" -ForegroundColor Gray
    }
}

function Find-VsInstall {
    # Prefer vswhere (installed alongside any VS product) for reliable detection
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $path = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null
        if ($path) { return $path.Trim() }
    }
    # Fallback: check common install locations for all known VS 2022 editions
    $candidates = @(
        "$env:ProgramFiles\Microsoft Visual Studio\2022\BuildTools",
        "$env:ProgramFiles\Microsoft Visual Studio\2022\Community",
        "$env:ProgramFiles\Microsoft Visual Studio\2022\Professional",
        "$env:ProgramFiles\Microsoft Visual Studio\2022\Enterprise",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\Community",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\Professional",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\Enterprise"
    )
    foreach ($c in $candidates) {
        if (Test-Path "$c\VC\Tools\MSVC") { return $c }
    }
    return $null
}

Write-Host ""
Write-Host "=== 3DSLab Windows Setup ===" -ForegroundColor Cyan
Write-Host "vcpkg will be installed to: $VcpkgDir" -ForegroundColor DarkGray
Write-Host ""

# --- CMake ---
Write-Host "[1/5] Checking CMake..." -ForegroundColor Yellow
if (Get-Command cmake -ErrorAction SilentlyContinue) {
    Write-Host "  CMake already installed: $(cmake --version | Select-Object -First 1)" -ForegroundColor Gray
} else {
    Write-Host "  Installing CMake..."
    Invoke-Winget @("install", "Kitware.CMake", "--accept-package-agreements", "--accept-source-agreements")
}

# --- Ninja ---
Write-Host "[2/5] Checking Ninja..." -ForegroundColor Yellow
if (Get-Command ninja -ErrorAction SilentlyContinue) {
    Write-Host "  Ninja already installed: $(ninja --version)" -ForegroundColor Gray
} else {
    Write-Host "  Installing Ninja..."
    Invoke-Winget @("install", "Ninja-build.Ninja", "--accept-package-agreements", "--accept-source-agreements")
}

# --- LLVM / Clang ---
Write-Host "[3/5] Checking LLVM/Clang..." -ForegroundColor Yellow
if (Get-Command clang -ErrorAction SilentlyContinue) {
    Write-Host "  LLVM already installed: $((clang --version 2>&1) | Select-Object -First 1)" -ForegroundColor Gray
} else {
    # Check both 64-bit and 32-bit Program Files locations
    $llvmBin = @(
        "$env:ProgramFiles\LLVM\bin",
        "${env:ProgramFiles(x86)}\LLVM\bin"
    ) | Where-Object { Test-Path "$_\clang.exe" } | Select-Object -First 1

    if ($llvmBin) {
        Write-Host "  LLVM found but not in PATH." -ForegroundColor Gray
    } else {
        Write-Host "  Installing LLVM..."
        Invoke-Winget @("install", "LLVM.LLVM", "--accept-package-agreements", "--accept-source-agreements")
        $llvmBin = "$env:ProgramFiles\LLVM\bin"
    }
    Add-ToUserPath $llvmBin
}

# --- Visual Studio Build Tools ---
Write-Host "[4/5] Checking Visual Studio Build Tools..." -ForegroundColor Yellow
$vsDir = Find-VsInstall
if ($vsDir) {
    Write-Host "  VS / Build Tools found at: $vsDir" -ForegroundColor Gray
} else {
    Write-Host "  Installing Visual Studio 2022 Build Tools (C++ workload + Windows SDK)..."
    Write-Host "  This may take several minutes..." -ForegroundColor Gray
    Invoke-Winget @("install", "Microsoft.VisualStudio.2022.BuildTools",
        "--override", "--wait --quiet --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended",
        "--accept-package-agreements", "--accept-source-agreements")
    $vsDir = Find-VsInstall
}

# --- vcpkg ---
Write-Host "[5/5] Checking vcpkg..." -ForegroundColor Yellow
if (Test-Path "$VcpkgDir\vcpkg.exe") {
    Write-Host "  vcpkg already installed." -ForegroundColor Gray
} else {
    if (-not (Test-Path $VcpkgDir)) {
        if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
            throw "git is required to install vcpkg but was not found. Install git and re-run this script."
        }
        Write-Host "  Cloning vcpkg to $VcpkgDir..."
        git clone https://github.com/microsoft/vcpkg.git $VcpkgDir
    }
    Write-Host "  Bootstrapping vcpkg..."
    & "$VcpkgDir\bootstrap-vcpkg.bat" -disableMetrics
}
Add-ToUserPath $VcpkgDir
if (-not [System.Environment]::GetEnvironmentVariable("VCPKG_ROOT", "User")) {
    [System.Environment]::SetEnvironmentVariable("VCPKG_ROOT", $VcpkgDir, "User")
    $env:VCPKG_ROOT = $VcpkgDir
    Write-Host "  Set VCPKG_ROOT=$VcpkgDir" -ForegroundColor Green
}

# --- Summary ---
Write-Host ""
Write-Host "=== Setup Complete ===" -ForegroundColor Cyan
Write-Host ""
Write-Host "Installed versions:" -ForegroundColor White

# Refresh PATH in the current session before printing versions
$env:PATH = [System.Environment]::GetEnvironmentVariable("PATH", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("PATH", "User")

& cmake --version 2>$null | Select-Object -First 1 | ForEach-Object { Write-Host "  CMake:  $_" }
& ninja --version 2>$null | ForEach-Object { Write-Host "  Ninja:  $_" }
& clang --version 2>&1 | Select-Object -First 1 | ForEach-Object { Write-Host "  Clang:  $_" }
& vcpkg version 2>$null | Select-Object -First 1 | ForEach-Object { Write-Host "  vcpkg:  $_" }
Write-Host ""
Write-Host "To build the project, open a new terminal and run:" -ForegroundColor White
if ($vsDir) {
    Write-Host "  & `"$vsDir\Common7\Tools\Launch-VsDevShell.ps1`" -Arch amd64 -HostArch amd64" -ForegroundColor DarkGray
}
Write-Host "  cmake -B build -G Ninja -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_BUILD_TYPE=Debug" -ForegroundColor DarkGray
Write-Host "  cmake --build build" -ForegroundColor DarkGray
Write-Host "  .\build\example.exe" -ForegroundColor DarkGray
Write-Host ""
