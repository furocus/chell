#Requires -Version 5.1
param(
    [string]$ClangFormatPath = $env:CSHELL_CLANG_FORMAT
)

& "$PSScriptRoot/format.ps1" -Mode check -ClangFormatPath $ClangFormatPath
exit $LASTEXITCODE
