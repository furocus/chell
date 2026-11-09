#Requires -Version 7.0
param(
    [Parameter(Mandatory)]
    [string]$ClangFormatPath
)

$ErrorActionPreference = 'Stop'
# 書式違反があっても全ファイルを検査し、最後に結果を返す。
$PSNativeCommandUseErrorActionPreference = $false

$repoRoot = Split-Path -Parent $PSScriptRoot
$styleFile = Join-Path $repoRoot '.clang-format'
if (-not (Test-Path -LiteralPath $styleFile -PathType Leaf)) {
    throw "Missing style file: $styleFile"
}

$toolchainFile = Join-Path $repoRoot 'config/toolchain.json'
$toolchain = Get-Content -LiteralPath $toolchainFile -Raw | ConvertFrom-Json
$expectedVersion = $toolchain.clangFormat
if ($expectedVersion -notmatch '^\d+\.\d+\.\d+$') {
    throw 'config/toolchain.json must specify clangFormat as X.Y.Z'
}

$formatter = (Resolve-Path -LiteralPath $ClangFormatPath).ProviderPath
$versionOutput = (& $formatter --version) -join ' '
if ($LASTEXITCODE -ne 0) {
    throw "Could not read clang-format version: $formatter"
}

$versionMatch = [regex]::Match(
    $versionOutput,
    '\bclang-format version (\d+\.\d+\.\d+)(?=\s|$)'
)
if ($versionMatch.Groups[1].Value -ne $expectedVersion) {
    throw "Expected clang-format $expectedVersion, got '$versionOutput' from $formatter"
}
Write-Host "Using clang-format $expectedVersion at $formatter"

$files = @(git -C $repoRoot -c core.quotePath=false ls-files -- '*.c' '*.h')
if ($LASTEXITCODE -ne 0) {
    throw 'Could not list tracked C source and header files'
}

$checked = 0
$failed = 0
foreach ($file in $files) {
    $path = Join-Path $repoRoot $file
    # 手元で削除済みのファイルは検査対象から外す。
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        continue
    }

    $checked++
    & $formatter "--style=file:$styleFile" --dry-run --Werror $path
    if ($LASTEXITCODE -ne 0) {
        $failed++
    }
}

Write-Host "Format: $checked file(s) checked, $failed failed"
if ($failed -gt 0) {
    exit 1
}
exit 0