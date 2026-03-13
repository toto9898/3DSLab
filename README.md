# 3DSLab

A 3DS file importer and viewer built with libigl, GLFW, and OpenGL.

## Prerequisites

Run the setup script from an **Administrator** PowerShell terminal:
```powershell
.\setup.ps1
```

By default, vcpkg is installed to `$env:USERPROFILE\vcpkg`. To install it elsewhere, pass the `-VcpkgDir` parameter:
```powershell
.\setup.ps1 -VcpkgDir "D:\dev\vcpkg"
```

This installs and configures: **CMake**, **Ninja**, **LLVM/Clang**, **Visual Studio 2022 Build Tools** (MSVC + Windows SDK), and **vcpkg**. Already-installed tools are skipped automatically. At the end, the script prints the exact `Launch-VsDevShell.ps1` command to use for your specific VS installation.

## Building

Every build session requires the MSVC environment to be active in your terminal. Run the `Launch-VsDevShell.ps1` command printed by `setup.ps1` at the end of setup, or find it yourself:
```powershell
& "$(vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath)\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64 -HostArch amd64
```

Then configure (first time only, or after changing CMakeLists.txt):
```powershell
cmake -B build -G Ninja -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_BUILD_TYPE=Debug
```

Then build:
```powershell
cmake --build build --config Debug
```

## Running

From the project root:
```powershell
.\build\example.exe
```

The working directory must be the project root so the relative path to the sample file (`src\3DS\Samples\sample_model.3ds`) resolves correctly.

## Viewer Controls

| Key | Action |
|-----|--------|
| Drag | Rotate scene |
| `N` / `P` | Next / Previous mesh |
| `C` | Clear selection |
| `L` | Toggle wireframe |
| `F` | Toggle face-based lighting |
| `O` | Toggle orthographic/perspective |
| `T` | Toggle filled faces |
| `Z` | Snap to canonical view |
