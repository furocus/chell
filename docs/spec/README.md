# Chell 開発仕様・公開API 準備版

作成・照合日: 2026-11-17 / 状態: レビュー用草案（実装済み・合意済みの証明ではありません）

Chellを5人で開発するための共通資料です。全員dev、各自週6〜10時間、2027年1月初週の完成目標を前提にします。全員が設計・実装・テスト・説明・他者レビューを担います。E（furocus）が依頼者本人です。完成目標の厳密な日付は未確定です。

[図入り閲覧ページを開く](index.html)（オフライン対応・4資料の全文と描画済みの図）。GitHub上ではこのMarkdownを入口にし、HTMLは取得後にブラウザーで開いてください。

## 読む順序

| 順 | 資料 | 分かること |
|---|---|---|
| 1 | このページ | 作るもの、担当間の流れ、資料の位置付け |
| 2 | [動作仕様](BEHAVIOR.md) | 利用者に約束する動作、失敗時、完成確認 |
| 3 | [公開API](PUBLIC_API.md) | 共有型、関数宣言、戻り値、所有権、ヘッダー依存 |
| 4 | [担当別ガイド](DEVELOPMENT_GUIDE.md) | A〜Eの実装順、学習項目、APIの読み方、接続相手 |

「OpenAPIとファイル構成を図示」の会話（2026-11-09、01a1bdd2-a7fa-7480-89cc-5251912ba12e）にある、公開関数とファイル依存を可視化する構想を具体化しています。OpenAPI標準はHTTP APIの記述仕様です。このシェルのプロセス内のC関数は、Cの宣言・契約表・Mermaid図で記述します。[OpenAPI公式の定義](https://spec.openapis.org/oas/v3.2.1.html)

## 何を作るか

Windowsの対話コンソールで1行を受け取り、組み込みコマンドか外部exeを同期実行し、次の入力へ戻るシェルです。独自のターミナル画面は作りません。

対象: cd / pwd / dir / echo / help / exit、外部exe、PATH検索、日本語、引数解析、エラー後の復帰、中断、メモリとHANDLEの解放。詳しい境界は動作仕様を参照してください。

## 全員で共有する処理の図

矢印は関数呼び出し・処理順です。Issueをこの順にすべて完了させる、という意味ではありません。

~~~mermaid
flowchart TD
    START["A: main → shell_run"] --> READ["B: プロンプト表示・1行入力"]
    READ --> CHECK{"A: 入力結果"}
    CHECK -->|正常| PARSE["C: 検査・解析"]
    CHECK -->|中断・コード維持| READ
    CHECK -->|上限超過| INPUTERR["A/B: エラー報告・コード1"]
    INPUTERR --> READ
    CHECK -->|EOF| END["A: 終了・後片付け"]
    CHECK -->|入力API失敗| FATAL["A: エラー表示・非0終了（草案）"]
    PARSE --> RESULT{"A: 解析結果"}
    RESULT -->|空入力| READ
    RESULT -->|失敗| ERROR["A/B: エラー報告・コード1"]
    ERROR --> READ
    RESULT -->|成功| FS["D: cd・pwd・dir を判定"]
    FS --> KNOWNFS{"handled"}
    KNOWNFS -->|TRUE| BRESULT{"A: output_failed"}
    BRESULT -->|FALSE| FREE["A: 結果を保持 → Cのcommand_free"]
    BRESULT -->|TRUE| ABORTFREE
    KNOWNFS -->|FALSE| CORE["E: echo・help・exit を判定"]
    CORE --> KNOWNCORE{"handled"}
    KNOWNCORE -->|TRUE| BRESULT
    KNOWNCORE -->|FALSE| PROC["A: 外部exeの解決・起動・終了待ち"]
    PROC --> WAIT{"launched AND NOT completed"}
    WAIT -->|いいえ| FREE
    WAIT -->|はい| ABORTFREE["A: command_free・異常終了へ"]
    ABORTFREE --> FATAL
    FATAL --> END
    FREE --> EXIT{"exit_requested"}
    EXIT -->|FALSE| READ
    EXIT -->|TRUE| END
~~~

表示失敗をAへ伝えるoutput_failedはD03/D05の追加案です。入力・表示の致命的失敗は1で終了する案、外部exeの起動前の失敗は1で通常復帰する案です。

既知の組み込みで引数エラーになってもhandledはTRUEです。外部exeへ渡し直しません。解析に成功したCommandはexit要求や実行失敗でも解放します。起動後に終了待ち自体が失敗した経路は、通常復帰とは分けて扱います（D04）。この図は通常経路と主な分岐を表し、診断出力自体の失敗などの詳細は動作仕様と公開APIを参照します。

## 一例: echo "hello world"

~~~mermaid
sequenceDiagram
    participant A as A shell
    participant B as B console
    participant C as C parser
    participant D as D builtin_fs
    participant E as E builtin_core
    A->>B: console_read_line(buffer, capacity, event)
    B-->>A: LINE_OK / bufferに入力
    A->>C: parse_command(buffer, &command)
    C-->>A: PARSE_OK / argc=2 / argvを所有するCommand
    A->>D: builtin_fs_execute(&command, &state)
    D-->>A: handled=FALSE
    A->>E: builtin_core_execute(&command, &state)
    E->>B: console_write(表示文字列)
    B-->>E: 成功
    E-->>A: handled=TRUE / exit_code=0
    A->>C: command_free(&command)
    C-->>A: 解放済みの空Command
    Note over A,B: 次のプロンプトと入力へ
~~~

この図のA〜Eは開発担当です。実行時には同じプロセスの通常スレッドで順番に呼びます（制御ハンドラーは例外）。

このときargv[0]はecho、argv[1]はhello worldです。入力の引用符は表示しません。出力はhello worldと改行です。raw_lineには元の引数表記を残します。

## 担当と接続相手

| 人 | 既存担当を基礎にした責任範囲 | 主な接続相手 |
|---|---|---|
| A（GitHub: R25rvrt／会話表記: R25rvlt） | メインループ・振り分け・外部exe・統合コード | 全員 |
| B（kashu-nattu） | 入力・プロンプト・通常出力・エラー出力 | A、D、E |
| C（nekokawaipan） | 入力検査・引数解析・Commandの確保と解放 | A、B、D、E |
| D（monaka-0505） | cd・pwd・dir | A、B、C、E |
| E（furocus） | echo・help・exit、ビルド、共有仕様調整 | A、B、C、D |

Eが中断の共通制御の実装を受け持ち、A/Bと接続する案は今回の会話での提案です。合意前にAから所有権が移ったものとは扱いません。開発担当と、実行中の資源の所有者は別です。

## 公開APIの入口を探す

| 呼びたい処理 | 公開ヘッダー | 実装担当 | 主な呼び出し元 |
|---|---|---|---|
| シェル全体を開始 | shell.h | A | main.c |
| 入力・プロンプト・出力・エラー表示 | console.h | B | A、D、E |
| Commandを作成・解放 | parser.h | C | A |
| cd / pwd / dir、echo / help / exit | builtins.h | D / E | A |
| 外部exeを同期実行 | process.h | A | shell.c |
| Command / ShellStateと共有上限 | shell_types.h（提案） | 全員で契約確認 | parser.h / builtins.h / process.h |

関数名だけでなく、公開APIの前提・戻り値・所有権を合わせて読みます。中断モジュールの宣言はD02が決まってから追加します。

## ファイル配置図（提案・未移動）

2026-11-09の担当ディレクトリ案を図にしています。2026-11-10には機能別の平坦な配置案もありました。ここでは過去の会話を無条件に確定扱いせず、D01として選択を残します。

~~~text
src/
  main.c
  A/
    shell.c
    process.c
  B/
    console_input.c
    console_output.c
  C/
    parser.c
  D/
    builtins_fs.c
  E/
    builtins_core.c
    interrupt.c          # D02を採用する場合のみ
include/
  shell_types.h          # 型だけを集める新規提案
  shell.h
  console.h
  parser.h
  builtins.h
  process.h
  interrupt.h            # D02を採用する場合のみ
tests/
  test_parser.c
  test_builtins.c
  test_process.c
  helpers/child_probe.c
~~~

関数1個につきCファイル1個には分けません。parse_commandとcommand_freeは同じparser.cで管理します。実装Cファイルを別のCファイルからincludeせず、共有ヘッダーを通じて利用します。追加する本体Cファイルと必要なライブラリはCMakeへ明示登録します。

## 合意する項目

| ID | 項目 | 準備版での扱い | 確認する人 |
|---|---|---|---|
| D01 | 担当別ディレクトリか機能別配置か | 上の図は担当別の提案。コード移動は行わない | 全員 |
| D02 | 中断制御の担当・通知・読み取り解除・寿命 | E実装、A/B接続という案。詳細APIは未固定 | A・B・E |
| D03 | LINE_ERROR、プロンプト・表示失敗 | 非0終了とBuiltinResult.output_failedによる伝達を提案 | A・B・D・E |
| D04 | 子の起動後、終了待ちに失敗した場合 | 次のコマンドを重ねず異常終了する案 | A・E |
| D05 | 共通型・結果型・出力失敗のエラー取得 | shell_types.h、BOOL報告API、output_failed、エラー対応表を提案 | 全員 |
| D06 | 引用符・バックスラッシュの検査規則 | API実測例を動作仕様へ掲載。独自検査・子CRTとの差は要合意 | C・A・E |

草案の追加仕様（例: dirの表示形式、相対PATH項目の扱い）もレビュー対象です。まずD03/D05の戻り値、D06の解析期待値を関係者で揃え、D02は独立した検証課題にします。中断の未決事項で、通常のecho/pwd接続や単独確認まで止める必要はありません。

関係者で決まった仕様へ日付と理由を残します。規約の変更時は関係するヘッダー・仕様・呼び出し側・テストを同じPRで更新します。

## 根拠と現状の区別

- 過去の設計根拠: cShellのARCHITECTURE.md、FEATURES.md、TESTING.md、TEAM.mdと上記会話。
- 現在の前提: このチャットでのA〜E対応、Eが依頼者、週6〜10時間、1月初週の目標。
- 本ワークツリーの確認時点: src/main.cはHello World段階、CMakeはmain.cのみ本体へ登録。include/や機能テストの共有実装は未確認。
- 2026-11-17にIssue #5〜#10の本文・担当を参照。#9はclosedですが、本ワークツリーへの統合・機能合格を意味しません。既存Issueは最小接続の単位であり、外部exe・cd・dir・help・中断までの完了条件をすべて含むものではありません。
- 他の作業ツリーやPRでの作業を、ここに実装・統合済みとは扱いません。
- この資料は設計・学習・レビューの準備物であり、機能テストの合格記録ではありません。

原資料（この端末上の参照用）:

- [OpenAPIの会話](C:/Users/abxw4/chell/.local/conversation-history/cShell/conversations/01a1bdd2-a7fa-7480-89cc-5251912ba12e.md)
- [過去の設計](C:/Users/abxw4/Documents/ChatGPT/cShell/docs/ARCHITECTURE.md)
- [過去の機能一覧](C:/Users/abxw4/Documents/ChatGPT/cShell/docs/FEATURES.md)
- [過去のテスト計画](C:/Users/abxw4/Documents/ChatGPT/cShell/docs/TESTING.md)

上の絶対パスは原資料の所在説明です。チームへ配るときの仕様本文はこのディレクトリ内で読めるようにしています。

## 文書の検証と更新

[検証記録](VALIDATION.md)に図・リンク・宣言の確認結果と、機能未検証の範囲を記載します。Markdownを正本とし、描画済みSVGとHTMLは同じ変更で再生成します。更新手順は[表示用資料の生成手順](tools/README.md)を参照してください。
