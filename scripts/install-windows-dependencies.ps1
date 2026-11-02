[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

function Get-RequiredCommand {
    param([Parameter(Mandatory)][string]$Name)

    $command = Get-Command $Name -CommandType Application -ErrorAction SilentlyContinue
    if ($null -eq $command) {
        throw "$Name was not found on PATH. Install WinLibs and add its mingw64\bin directory to PATH."
    }

    return $command.Source
}

function Install-WingetPackage {
    param([Parameter(Mandatory)][string]$Id)

    $winget = Get-Command winget -CommandType Application -ErrorAction SilentlyContinue
    if ($null -eq $winget) {
        throw "winget is required to install $Id. Install or update App Installer from the Microsoft Store."
    }

    & $winget.Source install --id $Id --exact --source winget `
        --accept-source-agreements --accept-package-agreements `
        --silent --disable-interactivity
    if ($LASTEXITCODE -ne 0) {
        throw "winget failed to install $Id (exit code $LASTEXITCODE)."
    }
}

function Refresh-ProcessEnvironment {
    $machinePath = [Environment]::GetEnvironmentVariable('Path', 'Machine')
    $userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
    $env:Path = "$machinePath;$userPath"

    foreach ($scope in @('Machine', 'User')) {
        $sdkPath = [Environment]::GetEnvironmentVariable('VULKAN_SDK', $scope)
        if ($sdkPath) {
            $env:VULKAN_SDK = $sdkPath
            break
        }
    }
}

function Test-VulkanSdk {
    param([string]$Path)

    return $Path -and (Test-Path (Join-Path $Path 'Include\vulkan\vulkan.h'))
}

$gcc = Get-RequiredCommand 'gcc'
$gxx = Get-RequiredCommand 'g++'
$toolchainBin = Split-Path $gcc -Parent
if ((Split-Path $gxx -Parent) -ne $toolchainBin) {
    throw "gcc and g++ must come from the same WinLibs directory. Found $gcc and $gxx."
}

$cmake = Join-Path $toolchainBin 'cmake.exe'
$ninja = Join-Path $toolchainBin 'ninja.exe'
foreach ($tool in @($cmake, $ninja)) {
    if (-not (Test-Path $tool)) {
        throw "The WinLibs directory must contain cmake.exe and ninja.exe: $tool was not found."
    }
}

[Environment]::SetEnvironmentVariable('AXIOM_WINLIBS_BIN', $toolchainBin, 'User')
$env:AXIOM_WINLIBS_BIN = $toolchainBin

$cmakeVersionOutput = & $cmake --version | Select-Object -First 1
if ($cmakeVersionOutput -notmatch 'cmake version (\d+)\.(\d+)\.(\d+)') {
    throw "Could not determine the CMake version from: $cmakeVersionOutput"
}
$cmakeVersion = [version]::new(
    [int]$Matches[1],
    [int]$Matches[2],
    [int]$Matches[3]
)
if ($cmakeVersion -lt [version]'3.25.0') {
    throw "CMake 3.25 or newer is required; found $cmakeVersion at $cmake."
}

$python = Get-Command python -CommandType Application -ErrorAction SilentlyContinue |
    Select-Object -First 1
if ($null -eq $python) {
    Install-WingetPackage 'Python.Python.3.14'
    Refresh-ProcessEnvironment
    $python = Get-Command python -CommandType Application -ErrorAction SilentlyContinue |
        Select-Object -First 1
}
if ($null -eq $python) {
    throw 'Python was installed but is still not available. Open a new terminal and run this script again.'
}

$pythonVersionOutput = & $python.Source --version 2>&1
if ($LASTEXITCODE -ne 0 -or $pythonVersionOutput -notmatch '^Python \d+\.\d+') {
    throw "The Python command did not run successfully: $pythonVersionOutput"
}

if (-not (Test-VulkanSdk $env:VULKAN_SDK)) {
    Install-WingetPackage 'KhronosGroup.VulkanSDK'
    Refresh-ProcessEnvironment
}
if (-not (Test-VulkanSdk $env:VULKAN_SDK)) {
    $sdkDirectory = 'C:\VulkanSDK'
    if (Test-Path $sdkDirectory) {
        $sdk = Get-ChildItem $sdkDirectory -Directory |
            Sort-Object LastWriteTime -Descending |
            Where-Object { Test-VulkanSdk $_.FullName } |
            Select-Object -First 1
        if ($null -ne $sdk) {
            $env:VULKAN_SDK = $sdk.FullName
        }
    }
}
if (-not (Test-VulkanSdk $env:VULKAN_SDK)) {
    throw 'The Vulkan SDK installation could not be located. Set VULKAN_SDK to its installation directory and rerun this script.'
}

Write-Host "CMake: $cmakeVersion ($cmake)"
Write-Host "Ninja: $ninja"
Write-Host "Python: $pythonVersionOutput ($($python.Source))"
Write-Host "Vulkan SDK: $env:VULKAN_SDK"
Write-Host 'Dependencies are ready. Open a new terminal before configuring.'
