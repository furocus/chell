#Requires -Version 5.1
param(
    [Parameter(Mandatory)]
    [string]$ClangFormatPath,
    [string]$GitBashPath
)

$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$gitPath = (Get-Command git.exe -CommandType Application | Select-Object -First 1).Source
$powerShellPath = (Get-Command powershell.exe -CommandType Application | Select-Object -First 1).Source
if (-not $GitBashPath) {
    $GitBashPath = Join-Path (Split-Path -Parent (Split-Path -Parent $gitPath)) 'bin/bash.exe'
}
$ClangFormatPath = (Resolve-Path -LiteralPath $ClangFormatPath).ProviderPath
$GitBashPath = (Resolve-Path -LiteralPath $GitBashPath).ProviderPath
$tempBase = [IO.Path]::GetTempPath()
$fixtureRoot = Join-Path $tempBase ('format-command-' + [guid]::NewGuid().ToString('N'))
$fixture = Join-Path $fixtureRoot ('repo with spaces ' + [char]0x65E5 + [char]0x672C)
$savedEnvironment = @{}
foreach ($name in @('PATH', 'ProgramFiles', 'ProgramFiles(x86)', 'CSHELL_CLANG_FORMAT', 'CSHELL_VS_DIR',
        'CSHELL_FORMAT_TEST_ROOT', 'CSHELL_FORMAT_TEST_ACTION', 'FORMAT_TEST_VERSION',
        'FORMAT_TEST_EXIT', 'FORMAT_TEST_VERSION_EXIT', 'FORMAT_TEST_LOG', 'FORMAT_TEST_VS_ROOT', 'FORMAT_TEST_VS_EXIT')) {
    $savedEnvironment[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
}
$utf8 = New-Object System.Text.UTF8Encoding($false)
$assertions = 0

function Write-FixtureFile([string]$Path, [string]$Content) {
    if ($Path.EndsWith('.sh')) { $Content = $Content.Replace("`r`n", "`n") }
    [IO.File]::WriteAllText($Path, $Content, $utf8)
}
function Assert-True([bool]$Condition, [string]$Message) {
    if (-not $Condition) { throw "FAIL: $Message" }
    $script:assertions++
}
function Invoke-Entry([string]$Shell, [string]$Action, [int]$ExpectedExit) {
    $env:CSHELL_FORMAT_TEST_ACTION = $Action
    $ErrorActionPreference = 'Continue'
    if ($Shell -eq 'Git Bash') {
        $output = @(& $GitBashPath --noprofile --norc (Join-Path $fixtureRoot 'entry.sh') 2>&1)
    }
    else {
        $output = @(& $powerShellPath -NoProfile -ExecutionPolicy Bypass -File (Join-Path $fixtureRoot 'entry.ps1') 2>&1)
    }
    $code = $LASTEXITCODE
    $ErrorActionPreference = 'Stop'
    Assert-True ($code -eq $ExpectedExit) "$Shell $Action returned $code, expected $ExpectedExit. $($output -join ' ')"
    return ($output -join "`n")
}
function Get-SourceHash {
    return ((@($source, $header) | ForEach-Object { (Get-FileHash -LiteralPath $_).Hash }) -join ':')
}

try {
    [void](New-Item -ItemType Directory -Path (Join-Path $fixture 'scripts') -Force)
    [void](New-Item -ItemType Directory -Path (Join-Path $fixture 'config'))
    Copy-Item -LiteralPath (Join-Path $repoRoot '.clang-format') -Destination $fixture
    Copy-Item -LiteralPath (Join-Path $repoRoot 'config/toolchain.json') -Destination (Join-Path $fixture 'config')
    foreach ($name in @('dev.cmd', 'format.ps1', 'check-format.ps1')) {
        Copy-Item -LiteralPath (Join-Path $repoRoot "scripts/$name") -Destination (Join-Path $fixture 'scripts')
    }
    Write-FixtureFile (Join-Path $fixtureRoot 'entry.ps1') @'
Set-Location -LiteralPath $env:CSHELL_FORMAT_TEST_ROOT
& .\scripts\dev.cmd $env:CSHELL_FORMAT_TEST_ACTION
exit $LASTEXITCODE
'@
    Write-FixtureFile (Join-Path $fixtureRoot 'entry.sh') @'
cd "$CSHELL_FORMAT_TEST_ROOT" || exit 99
./scripts/dev.cmd "$CSHELL_FORMAT_TEST_ACTION"
exit $?
'@
    $env:CSHELL_FORMAT_TEST_ROOT = $fixture
    $env:CSHELL_CLANG_FORMAT = $ClangFormatPath
    $env:CSHELL_VS_DIR = 'Z:\must-not-initialize-msvc'
    $source = Join-Path $fixture ('source with spaces ' + [char]0x65E5 + [char]0x672C + '.c')
    $headerDirectory = Join-Path $fixture 'nested [headers]'
    [void](New-Item -ItemType Directory -Path $headerDirectory)
    $header = Join-Path $headerDirectory 'sample [1].h'
    $deleted = Join-Path $fixture 'deleted.c'
    $untracked = Join-Path $fixture 'untracked.c'
    $other = Join-Path $fixture 'unchanged.cpp'
    $badSource = "int main(){return 0;}`r`n"
    $badHeader = "int sample(int first,int second);`r`n"
    Write-FixtureFile $source $badSource
    Write-FixtureFile $header $badHeader
    Write-FixtureFile $deleted $badSource
    Write-FixtureFile $other $badSource
    & $gitPath -C $fixture init --quiet
    if ($LASTEXITCODE) { throw 'git init failed' }
    & $gitPath -C $fixture add -- '*.c' '*.h' '*.cpp'
    if ($LASTEXITCODE) { throw 'git add failed' }
    Remove-Item -LiteralPath $deleted
    Write-FixtureFile $untracked $badSource

    foreach ($shell in @('PowerShell', 'Git Bash')) {
        Write-FixtureFile $source $badSource
        Write-FixtureFile $header $badHeader
        $before = Get-SourceHash
        $output = Invoke-Entry $shell 'format-check' 1
        Assert-True ((Get-SourceHash) -eq $before) "$shell check must not change files"
        Assert-True ($output -match '2 file\(s\) processed, 2 failed') "$shell must process tracked C/H only and skip deleted files: $output"
        [void](Invoke-Entry $shell 'format' 0)
        $formatted = Get-SourceHash
        Assert-True ($formatted -ne $before) "$shell format must fix files"
        [void](Invoke-Entry $shell 'format-check' 0)
        [void](Invoke-Entry $shell 'format' 0)
        Assert-True ((Get-SourceHash) -eq $formatted) "$shell format must be idempotent"
        Assert-True (([IO.File]::ReadAllText($untracked)) -eq $badSource) 'Untracked C source must remain unchanged'
        Assert-True (([IO.File]::ReadAllText($other)) -eq $badSource) 'Non-C/H file must remain unchanged'
        Write-Host "PASS: $shell check/fix/check/idempotence, spaces, Unicode, tracked files and deletions"
    }

    # Exercise the existing CI entry with an explicit executable path.
    Write-FixtureFile $source $badSource
    $before = Get-SourceHash
    $ErrorActionPreference = 'Continue'
    $output = @(& $powerShellPath -NoProfile -ExecutionPolicy Bypass -File (Join-Path $fixture 'scripts/check-format.ps1') -ClangFormatPath $ClangFormatPath 2>&1)
    $ErrorActionPreference = 'Stop'
    Assert-True ($LASTEXITCODE -eq 1) 'CI compatibility wrapper must preserve check failure'
    Assert-True ((Get-SourceHash) -eq $before) 'CI wrapper must not write files'
    [void](Invoke-Entry 'PowerShell' 'format' 0)
    $ErrorActionPreference = 'Continue'
    $env:CSHELL_CLANG_FORMAT = 'Z:\explicit-argument-must-win.exe'
    $output = @(& $powerShellPath -NoProfile -ExecutionPolicy Bypass -File (Join-Path $fixture 'scripts/check-format.ps1') -ClangFormatPath $ClangFormatPath 2>&1)
    $ErrorActionPreference = 'Stop'
    Assert-True ($LASTEXITCODE -eq 0) 'CI compatibility wrapper must preserve success and prefer explicit argument to environment'
    Write-Host 'PASS: existing check-format.ps1 -ClangFormatPath compatibility'

    # A small native stub lets us test discovery and failures without installing tools.
    $tools = Join-Path $fixtureRoot 'stub tools'
    [void](New-Item -ItemType Directory -Path $tools)
    $stub = Join-Path $tools 'clang-format.exe'
    Add-Type -TypeDefinition @'
using System;
using System.IO;
public class FormatCommandFixture {
    public static int Main(string[] args) {
        if (Path.GetFileName(Environment.GetCommandLineArgs()[0]) == "vswhere.exe") {
            Console.WriteLine(Environment.GetEnvironmentVariable("FORMAT_TEST_VS_ROOT"));
            return Int32.Parse(Environment.GetEnvironmentVariable("FORMAT_TEST_VS_EXIT") ?? "0");
        }
        if (args.Length == 1 && args[0] == "--version") {
            Console.WriteLine("clang-format version " + Environment.GetEnvironmentVariable("FORMAT_TEST_VERSION"));
            return Int32.Parse(Environment.GetEnvironmentVariable("FORMAT_TEST_VERSION_EXIT") ?? "0");
        }
        File.AppendAllText(Environment.GetEnvironmentVariable("FORMAT_TEST_LOG"), String.Join("|", args) + "\n");
        return Int32.Parse(Environment.GetEnvironmentVariable("FORMAT_TEST_EXIT") ?? "0");
    }
}
'@ -OutputAssembly $stub -OutputType ConsoleApplication
    Copy-Item -LiteralPath $stub -Destination (Join-Path $tools 'vswhere.exe')
    $env:FORMAT_TEST_LOG = Join-Path $fixtureRoot 'formatter.log'
    $env:FORMAT_TEST_VERSION = '22.1.30'
    $env:CSHELL_CLANG_FORMAT = $stub
    foreach ($shell in @('PowerShell', 'Git Bash')) {
        $output = Invoke-Entry $shell 'format-check' 1
        Assert-True ($output -match 'Expected clang-format 22\.1\.3' -and $output -match '22\.1\.30') "$shell must reject non-exact version with expected/actual versions"
        $env:CSHELL_CLANG_FORMAT = Join-Path $tools 'missing.exe'
        $output = Invoke-Entry $shell 'format' 1
        Assert-True ($output -match '22\.1\.3' -and $output -match 'CSHELL_CLANG_FORMAT') "$shell must explain missing tool remediation"
        $env:CSHELL_CLANG_FORMAT = $stub
    }
    $env:FORMAT_TEST_VERSION = '22.1.3'
    $env:FORMAT_TEST_EXIT = '23'
    foreach ($shell in @('PowerShell', 'Git Bash')) {
        foreach ($action in @('format', 'format-check')) {
            $output = Invoke-Entry $shell $action 1
            Assert-True ($output -match '2 file\(s\) processed, 2 failed') "$shell must report all native formatter failures"
        }
    }
    $log = [IO.File]::ReadAllText($env:FORMAT_TEST_LOG)
    Assert-True ($log -match '\|--dry-run\|--Werror\|' -and $log -match '\|-i\|') 'Correct native fix/check flags must be used'
    Assert-True ($log -match '--style=file:') 'Explicit style file must be passed'
    $env:FORMAT_TEST_VERSION_EXIT = '24'
    [void](Invoke-Entry 'PowerShell' 'format-check' 1)
    $env:FORMAT_TEST_VERSION_EXIT = $null
    Write-Host 'PASS: exact version, explicit missing tool, native fix/check and version failures, exit codes in both shells'

    # Restrict discovery to the fixture, including a simulated Build Tools installation.
    $env:CSHELL_CLANG_FORMAT = $null
    $env:PATH = $tools + ';' + (Split-Path -Parent $gitPath) + ';' + (Split-Path -Parent $powerShellPath) + ';' + $env:SystemRoot + '\System32'
    $env:ProgramFiles = Join-Path $fixtureRoot 'no program files'
    ${env:ProgramFiles(x86)} = $env:ProgramFiles
    $env:FORMAT_TEST_VS_ROOT = $null
    $env:FORMAT_TEST_VERSION = '22.1.30'
    $output = Invoke-Entry 'PowerShell' 'format-check' 1
    Assert-True ($output -match '22\.1\.3' -and $output -match '22\.1\.30') "Discovery must reject wrong PATH version: $output"
    $fallback = Join-Path $fixtureRoot 'Visual Studio Build Tools/VC/Tools/Llvm/bin'
    [void](New-Item -ItemType Directory -Path $fallback -Force)
    Copy-Item -LiteralPath $ClangFormatPath -Destination (Join-Path $fallback 'clang-format.exe')
    $env:FORMAT_TEST_VS_ROOT = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $fallback)))
    $output = Invoke-Entry 'PowerShell' 'format-check' 0
    Assert-True ($output -match 'Visual Studio Build Tools') 'Discovery must skip wrong PATH version and find matching Build Tools formatter'
    $env:FORMAT_TEST_VS_ROOT = $null
    Copy-Item -LiteralPath $ClangFormatPath -Destination $stub -Force
    $output = Invoke-Entry 'PowerShell' 'format-check' 0
    Assert-True ($output -match 'stub tools') 'Discovery must accept matching PATH formatter'
    $env:FORMAT_TEST_VS_EXIT = '13'
    [void](Invoke-Entry 'PowerShell' 'format-check' 0)
    $env:FORMAT_TEST_VS_EXIT = $null
    Remove-Item -LiteralPath $stub
    $output = Invoke-Entry 'PowerShell' 'format-check' 1
    Assert-True ($output -match 'No executable found' -and $output -match '22\.1\.3') 'Absent formatter must explain expected version'
    Write-Host 'PASS: PATH discovery, mismatched PATH fallback to Build Tools, no matching version and no executable'

    $env:CSHELL_CLANG_FORMAT = $ClangFormatPath
    & $gitPath -C $fixture rm --cached --force --quiet -- '*.c' '*.h'
    if ($LASTEXITCODE) { throw 'git rm --cached failed' }
    foreach ($shell in @('PowerShell', 'Git Bash')) {
        $output = Invoke-Entry $shell 'format-check' 0
        Assert-True ($output -match '0 file\(s\) processed, 0 failed') "$shell zero tracked files must succeed"
    }
    Rename-Item -LiteralPath (Join-Path $fixture '.git') -NewName '.git-hidden'
    $output = Invoke-Entry 'PowerShell' 'format-check' 1
    Assert-True ($output -match 'Could not list tracked') 'Git listing failure must not be mistaken for zero files'
    $env:PATH = $env:SystemRoot + '\System32'
    $output = Invoke-Entry 'PowerShell' 'format-check' 1
    Assert-True ($output -match 'Windows PowerShell 5\.1 is required') 'Missing PowerShell must have actionable error'
    Write-Host 'PASS: zero files, git listing failure and missing PowerShell'
    Write-Host "PASS: $assertions assertions"
}
finally {
    foreach ($name in $savedEnvironment.Keys) {
        [Environment]::SetEnvironmentVariable($name, $savedEnvironment[$name], 'Process')
    }
    # Delete only the unique fixture directory created under the system temp directory.
    $resolvedFixture = [IO.Path]::GetFullPath($fixtureRoot)
    $resolvedTemp = [IO.Path]::GetFullPath($tempBase).TrimEnd('\') + '\'
    if ($resolvedFixture.StartsWith($resolvedTemp, [StringComparison]::OrdinalIgnoreCase) -and
        (Split-Path -Leaf $resolvedFixture) -match '^format-command-[0-9a-f]{32}$') {
        if (Test-Path -LiteralPath $resolvedFixture) { Remove-Item -LiteralPath $resolvedFixture -Recurse -Force }
    }
}
