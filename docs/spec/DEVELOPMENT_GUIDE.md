# Chell 担当別実装・学習ガイド（準備版）

[入口](README.md) / [動作仕様](BEHAVIOR.md) / [公開API](PUBLIC_API.md)

全員devとして、自分のコード・確認・説明と他者レビューを担当します。必要な知識を使う場面と結び付け、他の担当の完成待ちで学習・単独確認を止めません。仕様の合意と学習の理解度を、実装完了と同一視しません。

## 全員共通: まず1本の流れを説明する

1. 入口のechoのシーケンス図を読む。
2. 入力buffer、Command、ShellStateが別のものだと説明する。
3. 自分の関数が借りるもの、作るもの、返すものを公開API表で確認する。
4. 具体的な入力・期待結果・失敗結果を先に3件以上挙げる。
5. 必要なCとWin32の項目を学び、単独で確認してからつなぐ。

共通知識:

- ポインタは何を指すか。値・指す先・ポインタ変数の更新を分ける。
- 配列の要素数、終端、バイト数を混同しない。
- constは借用中の変更を抑える約束。資源の所有権や寿命そのものは別に決める。
- APIごとに成功・失敗・エラー取得・解放関数が違う。
- HANDLEはすべてCloseHandleでよいわけではない。
- NULLポインタと、空フィールドを持つ構造体を分ける。

既存の[担当別ハンズオン](../../api-hands-on/README.md)を補助に使えます。教材の小さい型・関数名を、公開APIへそのままコピーしません。教材の課題完了は本体の機能合格とは別です。

## 受け渡しを1枚で読む

開発担当の記号と実行スレッドは別です。通常処理はAのループから同じスレッドで呼びます。次の図の矢印は「渡すデータ」で、include依存ではありません。

~~~mermaid
flowchart LR
    A["A shell: buffer / state / Commandの寿命"] -->|bufferと容量| B["B console"]
    B -->|ConsoleReadResultと1行| A
    A -->|lineと空Commandのアドレス| C["C parser"]
    C -->|ParseResultと所有メモリ| A
    A -->|借用Commandとstate| DE["D/E builtins"]
    DE -->|BuiltinResult| A
    DE -->|表示文字列| B
    A -->|借用Command| P["A process"]
    P -->|ProcessResult| A
    A -->|処理済みCommandを解放依頼| C
~~~

| 担当 | 受け取るもの | 返すもの | 自分で片付けるもの |
|---|---|---|---|
| A shell | 入力・解析・実行の結果 | 最終終了コード | Commandの解放呼び出し、シェル全体の資源 |
| B | 借用buffer・capacity・文字列・イベント（D02） | LINE結果、BOOL、保存エラー | 表示変換やエラー文の一時領域 |
| C | 借用lineと空Commandのアドレス | status、成功したCommand | 途中失敗の確保、command_freeで受けた領域 |
| D | 借用CommandとShellState | handled・output_failed・コード | 現在位置取得用領域、列挙HANDLE |
| E | 借用CommandとShellState | 同じ結果型、exit要求 | 自分が一時確保した表示用領域 |
| A process | 借用Command | 起動・終了確認の結果 | 可変コピー、解決パス、子のHANDLE |

### Cの宣言を具体例へ置き換える

~~~c
/* 呼び出し方の説明用。完成したshell_runではありません。 */
Command command = {0};
ParseResult parsed = parse_command(L"echo \"hello world\"", &command);
/* parsed.status == PARSE_OK の場合だけcommandを利用する。 */
/* 処理を終えたら、exit要求や実行失敗でも同じ解放地点へ進む。 */
command_free(&command);
~~~

&commandは「この構造体へ書いてよい」という出力先です。command_freeの後はraw_lineとargvを参照しません。D/Eが借りている最中に解放せず、D/Eもポインタを保存して後から使いません。

手書きCommandで確認する場合、文字列リテラルやスタック上の配列をCが確保したメモリのようにcommand_freeへ渡してはいけません。借用入力の確認と、解析成功した所有Commandの解放確認を分けます。

## A: メインループ・外部exe・統合

既存Issue: [#5](https://github.com/furocus/chell/issues/5)。2026-11-17の参照時点でGitHubの担当名はR25rvrt、会話での表記はR25rvltです。表記差を理由に既存のアサインを変更しません。#5の完了範囲は最小接続であり、外部exe・PATH・中断までの完了を意味しません。

目的: 部品を呼ぶ順番と、次の入力へ戻る条件を実装する。外部exeの解決・起動・終了待ちを担当する。

| 必要な知識 | 使う場面 |
|---|---|
| enum・構造体・分岐・反復 | LINE/PARSEの結果で処理を分ける |
| 借用ポインタと所有権 | Commandを各部品へ渡し、最後に解放する |
| プロセスとHANDLE | 起動・待機・終了取得と後片付け |
| 可変文字列のコピー | 元の入力を残して起動APIへ渡す |
| 明示パスとPATH | 実行対象を曖昧にしない |

実装順:
1. B/C/D/Eの宣言と結果型を合わせ、既存Issueのecho/pwd/exitを接続する。
2. 自作子exeを明示パスで起動し、終了とコードを確認する。
3. 相対パス、PATH検索、日本語・空白入りパスを追加する。
4. 失敗経路と資源の後片付けを確認し、中断をB/Eと接続する。

単独確認: テスト用child_probe.exeへ引数・現在位置・指定終了コードを記録させる。Windowsの既存ツールの偶然の動作を期待値にしない。
重点レビュー: CとCommandの寿命、Eと起動・待機失敗、中断はB/Eと共同。
説明チェック: 「子の終了コード1と、起動APIの失敗はどう違うか」「exitでもCommandを解放できるか」。

## B: コンソール入出力

既存Issue: [#6](https://github.com/furocus/chell/issues/6)、[#7](https://github.com/furocus/chell/issues/7)。担当kashu-nattu。

目的: 入力を安全に1行として渡し、文字とエラーを正しい出力先へ届ける。コマンドを解析・実行しない。

| 必要な知識 | 使う場面 |
|---|---|
| wchar_t配列・終端・容量 | bufferの範囲を守る |
| 標準HANDLEとコンソールモード | 入力元と出力先を区別 |
| UTF-16とUTF-8 | コンソール表示と捕捉出力 |
| 部分読み書きと結果の判定 | 長い入力・短い書き込みを扱う |
| 中断通知とブロック | A/Eと読み取りを解除する |

実装順:
1. console_write / console_write_errorの短い文字列と日本語を確認する。
2. 1行入力・改行除去・空行・Ctrl+Zを確認する。
3. 上限ちょうど/超過と次行への混入防止を確認する。
4. プロンプトと保存済みエラーの表示を追加する。
5. 出力捕捉、部分書き込み、中断を個別に確認する。

単独確認: 短いmainから入力関数・表示関数を呼ぶ。本体とは別の実行ターゲットにする。コンソール入力の検証をファイル入力の差し替えだけで済ませない。
重点レビュー: Aと入力結果、D/Eと表示・エラー、中断はA/Eと共同。
説明チェック: 「capacityはバイト数か」「GetStdHandleの戻り値を誰が閉じるか」。

## C: 入力検査・解析・所有権

既存Issue: [#8](https://github.com/furocus/chell/issues/8)。担当nekokawaipan。

目的: 入力を受け入れてよいか判定し、argvとraw_lineを作る。失敗時も所有資源を残さない。

| 必要な知識 | 使う場面 |
|---|---|
| 文字列走査・状態 | 引用符、未対応演算子、長さの検査 |
| ポインタ配列 | argvの各引数を参照 |
| 構造体の出力引数 | Command *へ解析結果を書き込む |
| malloc/freeとAPIの確保領域 | raw_lineとargvを違う方法で解放 |
| 境界値と独立した期待値 | 引数上限・空引数・引用符のテスト |

実装順:
1. 空入力、通常の空白区切り、raw_lineの保持を確認する。
2. 空白入り引数・空引数・日本語を確認する。
3. 未対応演算子、引用符、バックスラッシュの期待値をA/Eと固定する。
4. 上限と全失敗経路、command_freeの繰り返しを確認する。

単独確認: test_parserで入力と期待するargc/argv/raw_lineを並べる。検査対象と同じAPIを呼んだ値同士の比較だけにはしない。
重点レビュー: A/Eと仕様・所有権、Bと入力上限。
説明チェック: 「Command *で結果を返せる理由」「argvをfreeしてはいけない理由」。

## D: cd・pwd・dir

既存Issue: [#9](https://github.com/furocus/chell/issues/9)。担当monaka-0505。

目的: 解析済みCommandを受け、シェル自身の現在位置やディレクトリを操作する。組み込みの中で入力を読み直さない。

| 必要な知識 | 使う場面 |
|---|---|
| argc/argvと名前比較 | 名前と余分な引数を別々に判定 |
| プロセスの現在位置 | 親シェルのcdを実装 |
| 必要容量とバッファー | 現在位置の取得 |
| 列挙の開始・継続・終了 | dirとFindClose |
| 状態復元 | テスト後の現在位置を必ず元へ戻す |

実装順:
1. pwdを共通のbuiltins契約へ合わせる。
2. cdの引数なし・パス指定・失敗後の状態維持を追加する。
3. dirの直下列挙、空ディレクトリ、失敗、解放を追加する。

単独確認: 手書きCommandやCの解析結果を渡す。テスト自身が作った一時ディレクトリだけを後片付けし、失敗でも元の現在位置へ戻す。
重点レビュー: Eとhandled/引数誤り/output_failed、Bと表示、Aとcd後の子への継承。
説明チェック: 「pwd extraは未知コマンドか」「子exeでcdしても親が移動しない理由」。

## E: 基本コマンド・共通環境・仕様調整

既存Issue: [#10](https://github.com/furocus/chell/issues/10)。担当furocus（依頼者）。

目的: echo/help/exitを実装し、共通ビルド・テスト接続と仕様の整合を支える。全員のコードを代わりに完成させる担当ではない。

| 必要な知識 | 使う場面 |
|---|---|
| 借用文字列と状態の更新 | echoとexit |
| 戻り値・handled | 未知と引数誤りを区別 |
| ビルドターゲットとリンク | 本体・単独確認・テスト用子exeを分離 |
| 共有ヘッダーの依存 | API変更を呼び出し側と合わせる |
| イベント・別スレッド・寿命 | D02を採用した場合の中断共通制御 |

実装順:
1. echo/exitを共通契約へ接続し、helpを追加する。
2. 本体Cファイル・必要ライブラリ・各テストをCMakeへ明示登録する。parserを含むターゲットにはShell32が必要。現行CMakeの実物を確認し、過去資料にある共通ターゲットの存在を仮定しない。
3. Dと組み込みテストを接続する。
4. 中断の共通制御をA/Bと設計し、D02合意後に実装する。
5. 全員の確認手順を揃え、本人以外でも再現できることを確認する。

単独確認: 表示失敗がoutput_failedへ伝わるかもB/Aと確認する。exitを呼んでもテストプロセスは生きたままで、終了要求だけが更新される。Releaseでも検査が消えないよう、標準assertだけに頼らない。
重点レビュー: Dと組み込み、Aと環境・中断、Cと共有型。
説明チェック: 「Cファイルを増やすだけで本体に入るか」「担当者と実行時の所有者は同じか」。

## 契約をこの形にする理由（Why）

| 判断 | その理由 | 担当が確認すること |
|---|---|---|
| Commandの作成・解放をCへ集める | malloc系とLocalFreeの取り違え、失敗途中の漏れを防ぐ | Aは成功後に1回解放し、Cは失敗途中を片付ける |
| handledと成功を分ける | 既知コマンドの引数誤りが外部exeへ流れるのを防ぐ | D/Eで名前を先に判定し、Aはhandledを見て振り分ける |
| output_failedを通常の失敗と分ける（D03案） | 出力経路が壊れていても入力を続ける動作を避ける | D/Eは報告の成否を返し、Aは解放後に終了する |
| 起動成功と終了確認を分ける | 起動失敗、子の非0終了、待機失敗では対処が違う | Aはlaunched/completedを先に確認する |
| PATHを自分で順序通りに解決する | Chellの検索規則を起動APIの暗黙検索へ任せない | AはPATH順・空項目・明示パスを独立して確認する |
| raw_lineを保存する | argvから作り直すと元の引数表記を失う | Cは保持し、Aは起動用の可変コピーを別に作る |
| 中断通知と読み取り解除を別に設計する | イベントを通知するだけでは読み取りが戻る保証がない | A/B/Eは通常コンソールで破棄・復帰・終了時の同期を確認する |
| ソース配置と共有ヘッダーを分ける | 作業場所を変えても、利用側が必要とする契約を安定させる | 全員が.h経由で接続し、.cをincludeしない |

### 最小接続から完成範囲へ進む

既存Issue #5〜#10は、echo / pwd / exitを通す最小接続の単位です。2026-11-17の参照時点で#9はclosedですが、このワークツリーにはpwd本体がありません。Issueの状態、コードの統合、受け入れ合格を別々に確認します。

外部exe・PATHはA、cd / dirはD、helpはE、中断はD02の案に従いA/B/Eが接続します。追加分は担当・API・受け入れIDを記した別の作業単位として整理します。この文書の修正でIssueを作成・更新したものではありません。

## レビュー・Issueへの接続

| 作業のまとまり | 作成する成果物 | 機能完了に必要な接続 |
|---|---|---|
| 共有契約 | 型、関数宣言、期待値、未決事項の解消 | 呼び出す人と実装する人の確認 |
| 入出力 | Bの実装と単独確認 | Aからの入力、D/Eからの出力 |
| 解析 | Cの実装と解析テスト | B入力とA/D/EのCommand利用 |
| 組み込み | D/Eの実装と組み込みテスト | Cの解析、Bの表示、Aの状態管理 |
| 外部実行 | Aの実装と自作子exeによる検証 | Cのraw_line、Dのcd、B/Eの中断 |
| 最終確認 | 受け入れ結果と再現手順 | 5人のPCとDebug/Release |

Issueには、目的・担当・入力/出力・完了条件・依存先・確認方法を記録します。「参照する型が合意済み」と「相手の本体が完成済み」を別の条件として扱います。PR作成だけで完了にせず、受け入れ確認と必要なレビュー・マージまで追います。

## 公式資料の読む箇所

| 担当 | 公式資料 | 読む箇所・知りたいこと |
|---|---|---|
| A | [CreateProcessW](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessw) | lpApplicationName / lpCommandLine / 戻り値 / ハンドルの後片付け |
| A | [WaitForSingleObject](https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject) | 待機完了と失敗の区別 |
| A | [GetExitCodeProcess](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getexitcodeprocess) | 終了確認と終了コード取得 |
| B | [ReadConsole](https://learn.microsoft.com/en-us/windows/console/readconsole) | バッファー・読取数・入力モード・対象HANDLE |
| B | [WriteConsole](https://learn.microsoft.com/en-us/windows/console/writeconsole) | コンソール出力とリダイレクト済みHANDLEの違い |
| C | [CommandLineToArgvW](https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-commandlinetoargvw) | 先頭空白・引用符・確保単位・LocalFree・Shell32 |
| D | [GetCurrentDirectoryW](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getcurrentdirectoryw) | 必要容量と戻り値 |
| D | [SetCurrentDirectoryW](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setcurrentdirectoryw) | プロセスの現在位置と失敗 |
| D | [FindFirstFileW](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findfirstfilew) | 列挙開始の失敗とFindClose |
| E・A・B | [SetConsoleCtrlHandler](https://learn.microsoft.com/en-us/windows/console/setconsolectrlhandler) | 登録・通知・子への影響・通常起動時の確認 |

APIの詳しい性質は上記の一次資料を参照します。Chell固有の制限や扱いは動作仕様と公開APIを優先し、APIの自由度から勝手に機能を増やしません。
