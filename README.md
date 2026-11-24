# Chell

C言語とWin32 APIで開発している、Windows向けのコマンドラインシェルです。

現在は開発初期段階で、本体には `Hello World` を出力する確認用コードが入っています。

## 開発環境

Windows上で、MSVCのx64向けツールチェーンを使用します。

必要なツールの準備方法は、[環境構築ガイド](docs/ENVIRONMENT.md)を参照してください。使用するバージョンは [config/toolchain.json](config/toolchain.json) に定義しています。

## ビルド・実行

以下のコマンドは、リポジトリのルートでPowerShellから実行します。

### 環境を確認する

```powershell
.\scripts\dev.cmd check
```

必要なツールとバージョンが揃っていると、`Environment OK` が表示されます。

### Debugビルドする

```powershell
.\scripts\dev.cmd build debug
```

構成とビルドが順に実行され、`build/debug/chell.exe` が生成されます。

### 実行する

```powershell
.\build\debug\chell.exe
```

現在のコードでは `Hello World` が表示されます。

### Releaseビルドする

```powershell
.\scripts\dev.cmd build release
```

生成先は `build/release/chell.exe` です。

### VS Codeからビルドする

VS Codeでリポジトリのルートを開き、`Ctrl+Shift+B` を押すとDebugビルドを実行できます。

Releaseビルドは「ターミナル → タスクの実行」から `chell: build Release` を選択してください。

## 動作確認・テスト

現在、Chell本体の自動テスト（CTest）は登録されていません。変更後はビルドし、実行結果を手元で確認してください。

テストが追加された後は、次のコマンドで構成・ビルド・テストを実行できます。

```powershell
.\scripts\dev.cmd test debug
```

テストが未登録の状態では、このコマンドはエラー終了します。

### フォーマットを確認する

Pull Requestでは、Cソースとヘッダーのフォーマットチェックが実行されます。手元では次のコマンドで確認できます。

```powershell
.\scripts\dev.cmd format-check
```

自動修正する場合は、次のコマンドを実行し、変更された差分を確認してください。

```powershell
.\scripts\dev.cmd format
```

対象は教材を含むGit管理下の `.c`・`.h` ファイルです。必要なツールと指定方法は、[CONTRIBUTING.md](CONTRIBUTING.md)の「C/H の整形」を参照してください。

整形スクリプトには、CTestとは別に実行する回帰テストもあります。実行方法は同じ節に記載しています。

## ディレクトリ構成

| パス | 内容 |
|---|---|
| `src/` | Chell本体のソースコード |
| `scripts/` | 環境確認・ビルド・フォーマット確認用のスクリプト |
| `config/` | 使用するツールとバージョンの設定 |
| `.vscode/` | VS Codeのタスク設定 |
| `api-hands-on/` | C言語とAPIの使い方を学ぶための読解教材 |
| `build/` | ビルド時に生成されるファイル。Gitの管理対象外 |

## 学習用教材

[api-hands-on](api-hands-on/README.md)には、コードを読みながらポインターやAPIの使い方を学ぶ教材があります。

教材は本体のビルド対象には含まれていません。読解とコードの補完を目的としており、ビルド・実行の準備は不要です。

## 開発仕様

開発仕様・公開API・担当別ガイドは [仕様書の入口](docs/spec/README.md) を参照してください（レビュー用草案）。

## 開発への参加

ブランチ、コミット、Pull Request、レビューのルールは、[CONTRIBUTING.md](CONTRIBUTING.md)を参照してください。
