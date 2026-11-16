# 開発環境の構築

Chellをビルドするための環境を準備します。コマンドは、特に記載がなければリポジトリのルートでPowerShellから実行してください。

## 必要なツール

ビルドには、次のツールを使用します。

| ツール | 用途 |
|---|---|
| Git | ソースコードの取得・変更管理 |
| MSVCのC/C++ビルドツール | x64向けのコンパイル・リンク |
| Windows SDK | Windows向けのヘッダーとライブラリ |
| CMake | ビルド設定の生成 |
| Ninja | ビルドの実行 |

使用するバージョンは [config/toolchain.json](../config/toolchain.json) で管理しています。最新版ではなく、設定に記載されたバージョンを準備してください。

```powershell
Get-Content -Encoding UTF8 .\config\toolchain.json
```

MSVCは `msvc`、Windows SDKは `windowsSdk`、CMakeは `cmake`、Ninjaは `ninja` の値を確認してください。

ビルド対象はネイティブのWindows環境です。WSLやLinuxコンテナーでのビルドには対応していません。

## ビルドツールを準備する

1. Visual Studio Installerで、C++ビルドツールを導入します。Visual Studio IDEは必須ではありません。
2. 「個別のコンポーネント」で、設定に記載されたMSVCとWindows SDKが導入されていることを確認します。
3. 指定バージョンのCMakeとNinjaを導入し、コマンドとして検出できるようにします。

設定に記載されたバージョンを導入できない場合は、使用する配布元やバージョンをチームで確認してください。

## 環境を確認する

```powershell
.\scripts\dev.cmd check
```

このスクリプトは、Visual Studioのインストール先を検出し、指定されたMSVC・Windows SDKのx64環境を初期化します。その後、ツールのバージョンを確認します。

成功すると、`Environment OK` と使用するバージョンが表示されます。

この初期化はスクリプト内で有効になります。普段のビルドでは、次のように `dev.cmd` を経由してください。

```powershell
.\scripts\dev.cmd build debug
```

## VS Codeを使用する場合

VS Codeでリポジトリのルートを開くと、登録済みのタスクから環境確認とビルドを実行できます。

| タスク | 内容 |
|---|---|
| `chell: check environment` | 環境確認 |
| `chell: build Debug` | Debugビルド |
| `chell: build Release` | Releaseビルド |

`code` コマンドが利用できる場合は、次のコマンドでも開けます。

```powershell
.\scripts\dev.cmd code
```

## よくあるエラー

| エラー・症状 | 確認すること |
|---|---|
| MSVC Build Toolsが見つからない | Visual Studio InstallerでC++ビルドツールを導入したか |
| 指定のMSVCが見つからない | `toolchain.json` の `msvc` と一致するツールセットを導入したか |
| MSVC・SDKの初期化に失敗する | 指定のMSVCとWindows SDKが両方導入されているか |
| CMake・Ninjaが見つからない、またはバージョンが一致しない | 設定されたバージョンを導入し、別バージョンが優先されていないか |
| x64環境が必要と表示される | `dev.cmd` を経由して実行しているか |
| `code` コマンドが見つからない | VS CodeのコマンドがPATHに登録されているか。VS Codeから直接フォルダーを開くことも可能 |

Visual Studioを複数導入していて、別のインストール先を使用したい場合は、`CSHELL_VS_DIR` を設定できます。次のパスは、使用するインストール先に置き換えてください。

```powershell
$env:CSHELL_VS_DIR = 'C:\path\to\Visual Studio'
.\scripts\dev.cmd check
```
