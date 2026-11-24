#Requires -Version 5.1
[CmdletBinding()]
param(
    [ValidateSet('fix', 'check')]
    [string]$Mode = 'fix',
    [string]$ClangFormatPath = $env:CSHELL_CLANG_FORMAT
)

$ErrorActionPreference = 'Stop'
# Inspect native exit codes ourselves, including when called from PowerShell 7.
$PSNativeCommandUseErrorActionPreference = $false

function Find-Formatter {
    param([string]$ExplicitPath, [string]$ExpectedVersion)

    $hint = "Install clang-format $ExpectedVersion and add it to PATH, or set CSHELL_CLANG_FORMAT to its executable path (or use -ClangFormatPath)."
    $rejected = @()
    if ($ExplicitPath) {
        if (-not (Test-Path -LiteralPath $ExplicitPath -PathType Leaf)) {
            throw "Expected clang-format ${ExpectedVersion}; executable not found: $ExplicitPath. $hint"
        }
        $candidates = @((Resolve-Path -LiteralPath $ExplicitPath).ProviderPath)
    }
    else {
        $candidates = @(Get-Command clang-format.exe -CommandType Application -All -ErrorAction SilentlyContinue |
            Select-Object -ExpandProperty Source)
        $vswhere = @(Get-Command vswhere.exe -CommandType Application -ErrorAction SilentlyContinue |
            Select-Object -ExpandProperty Source)
        foreach ($base in @(${env:ProgramFiles(x86)}, $env:ProgramFiles)) {
            if ($base) {
                $vswhere += Join-Path $base 'Microsoft Visual Studio/Installer/vswhere.exe'
            }
        }
        $locator = $vswhere | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } | Select-Object -First 1
        if ($locator) {
            try {
                $installations = @(& $locator -all -prerelease -products '*' -property installationPath)
                if ($LASTEXITCODE -ne 0) { throw "vswhere exited with code $LASTEXITCODE" }
            }
            catch {
                $rejected += "${locator}: $($_.Exception.Message)"
                $installations = @()
            }
            foreach ($installation in $installations) {
                if ([string]::IsNullOrWhiteSpace($installation)) { continue }
                foreach ($relative in @('VC/Tools/Llvm/x64/bin/clang-format.exe', 'VC/Tools/Llvm/bin/clang-format.exe')) {
                    $candidate = Join-Path $installation $relative
                    if (Test-Path -LiteralPath $candidate -PathType Leaf) {
                        $candidates += $candidate
                    }
                }
            }
        }
    }

    foreach ($candidate in @($candidates | Select-Object -Unique)) {
        try {
            $versionOutput = (& $candidate --version 2>&1) -join ' '
            if ($LASTEXITCODE -ne 0) {
                throw "--version exited with code $LASTEXITCODE"
            }
            $versionMatch = [regex]::Match($versionOutput, '\bclang-format version (\d+\.\d+\.\d+)(?=\s|$)')
            if ($versionMatch.Groups[1].Value -eq $ExpectedVersion) {
                return $candidate
            }
            $rejected += "${candidate}: $versionOutput"
        }
        catch {
            $rejected += "${candidate}: $($_.Exception.Message)"
        }
    }
    $details = if ($rejected.Count) { $rejected -join '; ' } else { 'No executable found.' }
    throw "Expected clang-format $ExpectedVersion. $details $hint"
}

function Get-TrackedSources {
    param([string]$RepoRoot)

    # Read UTF-8 and NUL separators directly, independent of the console code page.
    $git = (Get-Command git.exe -CommandType Application -ErrorAction Stop | Select-Object -First 1).Source
    $startInfo = New-Object System.Diagnostics.ProcessStartInfo
    $startInfo.FileName = $git
    $startInfo.WorkingDirectory = $RepoRoot
    $startInfo.Arguments = 'ls-files -z -- *.c *.h'
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true
    $startInfo.RedirectStandardOutput = $true
    $startInfo.StandardOutputEncoding = New-Object System.Text.UTF8Encoding
    $process = New-Object System.Diagnostics.Process
    $process.StartInfo = $startInfo
    try {
        [void]$process.Start()
        $output = $process.StandardOutput.ReadToEnd()
        $process.WaitForExit()
        if ($process.ExitCode -ne 0) {
            throw "Could not list tracked C source and header files (git exited with code $($process.ExitCode))"
        }
        $output.Split([char]0, [System.StringSplitOptions]::RemoveEmptyEntries)
    }
    finally {
        $process.Dispose()
    }
}

try {
    $repoRoot = Split-Path -Parent $PSScriptRoot
    $styleFile = Join-Path $repoRoot '.clang-format'
    if (-not (Test-Path -LiteralPath $styleFile -PathType Leaf)) {
        throw "Missing style file: $styleFile"
    }
    $toolchain = Get-Content -LiteralPath (Join-Path $repoRoot 'config/toolchain.json') -Raw -Encoding UTF8 | ConvertFrom-Json
    $expectedVersion = $toolchain.clangFormat
    if ($expectedVersion -notmatch '^\d+\.\d+\.\d+$') {
        throw 'config/toolchain.json must specify clangFormat as X.Y.Z'
    }
    $formatter = Find-Formatter -ExplicitPath $ClangFormatPath -ExpectedVersion $expectedVersion
    Write-Host "Using clang-format $expectedVersion at $formatter"
    $files = @(Get-TrackedSources -RepoRoot $repoRoot)
    $formatArguments = @(if ($Mode -eq 'fix') { '-i' } else { '--dry-run'; '--Werror' })
    $processed = 0
    $failed = 0
    foreach ($file in $files) {
        $path = Join-Path $repoRoot $file
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
            continue
        }
        $processed++
        & $formatter "--style=file:$styleFile" @formatArguments $path
        if ($LASTEXITCODE -ne 0) {
            $failed++
        }
    }
    Write-Host "Format ($Mode): $processed file(s) processed, $failed failed"
    if ($failed -gt 0) { exit 1 }
    exit 0
}
catch {
    [Console]::Error.WriteLine("[ERROR] $($_.Exception.Message)")
    exit 1
}
