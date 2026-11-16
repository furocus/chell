# Contributing

このリポジトリで開発するときのルールです。

## ブランチ

作業ごとにブランチを分けます。

```bash
git switch -c feature/<issue-number>-short-title
```


`short-title` は英語の kebab-case を基本とします。

## C/H の整形

リポジトリルートから、Git 管理下のすべての `.c` / `.h` を整形できます。
教材も既存 CI と同じく対象に含みます。ローカルで削除済みのファイルと未追跡ファイルは対象外です。

Git Bash:

```bash
./scripts/dev.cmd format
./scripts/dev.cmd format-check
```

PowerShell:

```powershell
.\scripts\dev.cmd format
.\scripts\dev.cmd format-check
```

`format` は `.clang-format` を明示して `-i` で自動修正します。
`format-check` は `--dry-run --Werror` で検査し、ファイルを書き換えません。
書式違反やツールの失敗は非 0 の終了コードになります。
`debug` / `release` の指定や、MSVC・Windows SDK・CMake の環境初期化は不要です。

必要な環境は Windows PowerShell 5.1 以上（`dev.cmd` は Windows 標準の `powershell.exe` を使用）、
Git for Windows（`git.exe` が PATH に必要）、および `config/toolchain.json` の
`clangFormat` と完全に一致する clang-format（現在 **22.1.3**）です。PowerShell 7 は必須ではありません。

formatter は `CSHELL_CLANG_FORMAT` の明示指定を優先します。未指定の場合は PATH と、
`vswhere.exe` が列挙した Visual Studio / Build Tools の LLVM インストールから、版が一致するものを探します。
Community など特定のエディションやインストール先には依存しません。
不在・版不一致の場合は期待版と対処を表示して失敗します。実行時の自動ダウンロードは行いません。
必要な版を一度インストールして PATH に追加するか、別の場所にある場合はセッションで指定してください。
明示指定が不正な場合は、自動探索に切り替えず失敗します。

```bash
export CSHELL_CLANG_FORMAT='C:\path with spaces\clang-format.exe'
./scripts/dev.cmd format
```

```powershell
$env:CSHELL_CLANG_FORMAT = 'C:\path with spaces\clang-format.exe'
.\scripts\dev.cmd format
```

PowerShell スクリプトを直接使う場合は、明示引数でも指定できます。
既存 CI の `check-format.ps1 -ClangFormatPath ...` も引き続き利用できます。

```powershell
.\scripts\format.ps1 -ClangFormatPath 'C:\path with spaces\clang-format.exe'
.\scripts\check-format.ps1 -ClangFormatPath 'C:\path with spaces\clang-format.exe'
```

スクリプト自身の回帰テストは Windows PowerShell 5.1 で次のように実行します。
指定した実物の formatter と一時リポジトリを使い、Git Bash / PowerShell の入口や異常系を検証します。
一時ファイルは終了時に削除され、本体・教材は整形しません。

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\tests\format-command.tests.ps1 `
    -ClangFormatPath 'C:\path with spaces\clang-format.exe'
```

## コミットメッセージ

コミットメッセージは、変更の種類が分かる prefix を付けます。

よく使う prefix:

`feat`: 機能追加や画面追加。

`fix`: 不具合修正。

`docs`: README や説明文など、ドキュメントの変更。

`chore`: 設定、依存関係、開発環境などの変更。`chore`は雑用という意味。

例:

```text
feat: ホーム画面の土台を追加する
fix: 人数選択の表示崩れを修正する
docs: READMEにセットアップ手順を追加する
chore: TailwindとPrettierの設定を追加する
```

コミットは、できるだけ1つの目的ごとに分けます。
同じコミットに関係ない変更を混ぜないようにしてください。

## Pull Request

PR には、何を変更したか、なぜ変更したか、どう確認したかを書きます。

Issue を完了する PR では、PR の description に次のように書きます。

```text
Closes #1
```

関連だけ示したい場合は `refs #1` や `related #1` を使います。

PR を作る前に、変更したところを手元で確認してください。

## レビュー

マージ前にレビューを受けます。

レビューコメントでは、必要に応じて prefix を付けます。

`[must]` : 要修正という意味。

`[ask]` : ただの質問です、という意味。

`[nit]` : 細かい指摘で修正は不要、という意味。

`[imo]` : In My Opinionの略。私はこう思います、という意味。

`[fyi]` : For Your Informationの略。参考情報などを貼るときなどに使う。

`[good]` : 良かったところを伝える際などに使う。
