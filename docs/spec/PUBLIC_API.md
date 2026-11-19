# Chell 公開API仕様（準備版）

[入口](README.md) / [動作仕様](BEHAVIOR.md) / [担当別ガイド](DEVELOPMENT_GUIDE.md)

以下はヘッダーへ反映するための宣言案です。本体の完成解答や実装済みヘッダーではありません。既存設計の型・関数を具体化し、循環includeを避ける共通型ヘッダーを新規提案しています（D05）。

## 1. 図の読み方と依存方向

下の矢印は「左のファイルが右のヘッダーの宣言を使う」です。実行時のデータ移動とは別です。

~~~mermaid
flowchart LR
    subgraph IMPL["本体 .c"]
        MAIN["A: main.c"]
        S["A: shell.c"]
        P["A: process.c"]
        CI["B: console_input.c"]
        CW["B: console_output.c"]
        CP["C: parser.c"]
        DF["D: builtins_fs.c"]
        EC["E: builtins_core.c"]
    end
    subgraph HEADERS["公開ヘッダー .h"]
        SH["shell.h"]
        CO["console.h"]
        PA["parser.h"]
        BI["builtins.h"]
        PR["process.h"]
    end
    MAIN --> SH
    S --> SH
    S --> CO
    S --> PA
    S --> BI
    S --> PR
    P --> PR
    CI --> CO
    CI --> TY["shell_types.h"]
    CW --> CO
    CP --> PA
    DF --> BI
    EC --> BI
    DF --> CO
    EC --> CO
    S --> TY
    PA --> TY
    BI --> TY
    PR --> TY
~~~

console.hはstddef.hとwindows.hを自分でincludeし、共有のCommandやShellStateには依存しません。共有上限を使うconsole_input.cはshell_types.hを直接includeします。shell_types.hは上位のshell.hをincludeしません。各ヘッダーにinclude guardを付けます。

## 2. 共有型: shell_types.h（配置はD05の提案）

~~~c
#include <stddef.h>
#include <windows.h>

enum {
    SHELL_LINE_CAPACITY = 4096,
    SHELL_ARG_LIMIT = 64
};

typedef struct {
    wchar_t *raw_line;
    int argc;
    wchar_t **argv;
} Command;

typedef struct {
    BOOL exit_requested;
    DWORD last_exit_code;
} ShellState;
~~~

Commandの空状態はraw_line=NULL、argc=0、argv=NULL。ShellStateの初期値はexit_requested=FALSE、last_exit_code=0です。

Commandはコピーして別々に解放しません。構造体代入はポインタを複製するだけで、所有メモリを複製しないためです。argvはargcで走査し、argv[argc]がNULLであることへ依存しません。

Commandの有効期間中、argvの文字列とraw_lineはCommandが所有します。呼び出し先はconst Command *で借用し、内容の変更・個別解放・呼び出し後の保存をしません。外部実行用の変更可能なコピーはprocess側が別に確保します。const Command *でも、ポインタが指す文字列までCの型だけで変更禁止になるわけではありません。変更しないことを契約として守ります。

ShellStateの生存期間をAが管理します。通常はAがlast_exit_codeを更新し、Eのexit処理がexit_requestedをTRUEにします。各コマンドが無関係な状態を書き換えないようにします。

## 3. 入出力: console.h / B

~~~c
#include <stddef.h>
#include <windows.h>

typedef enum {
    LINE_OK,
    LINE_CANCELLED,
    LINE_EOF,
    LINE_TOO_LONG,
    LINE_ERROR
} ConsoleReadStatus;

typedef struct {
    ConsoleReadStatus status;
    DWORD win32_error;
} ConsoleReadResult;

ConsoleReadResult console_read_line(
    wchar_t *buffer, size_t capacity, HANDLE interrupt_event);
BOOL console_write(const wchar_t *text);
BOOL console_write_error(const wchar_t *text);
BOOL console_report_error(const wchar_t *context, DWORD error_code);
BOOL console_write_prompt(void);
~~~

| 関数 | 前提と入力 | 結果・失敗 | 所有権・副作用 |
|---|---|---|---|
| console_read_line | 書き込み可能buffer、終端を含むcapacity、借用イベント | 下表。本文を終端する | buffer内にだけ書く。入力を消費。イベントを閉じない |
| console_write | 終端付き、非NULLの借用文字列 | stdoutへ出力。成功TRUE、失敗FALSE | 文字列を保持・解放しない |
| console_write_error | 同上 | stderrへ出力。成功TRUE、失敗FALSE | 同上 |
| console_report_error | 文脈、保存済みの非0Win32エラー | 文脈付きエラーをstderrへ。成功TRUE、出力失敗FALSE（D05変更案） | メッセージ用の一時資源はBが解放。報告失敗で再帰しない |
| console_write_prompt | 引数なし | 現在位置と入力待ち記号を表示。失敗FALSE | 現在位置は変更しない |

読み取り前提: bufferは非NULL、capacityは1以上で、実領域の要素数を超えません。Aは通常4,096要素を渡します。Bは共有上限とcapacityの小さい方を守ります。中断イベントは有効な借用HANDLEを渡す方針で、最小構成でイベント未導入の呼び出しをどう表すかはD02で固定します。

console_report_errorのBOOL化は過去資料のvoidからの変更案です。エラー文の生成に失敗したら、文脈と数値コードを代替表示し、その表示の成否を返します。

BOOL出力APIのFALSE時には呼び出し側が直後にGetLastErrorで原因を保存できる契約を提案します（D05）。Bは後片付け前に最初のエラー値を保存し、返す直前にSetLastErrorで復元します。変換・確保失敗も非0のエラー値へ対応付けます（10節）。成功後のGetLastErrorは読みません。

| status | buffer | win32_error | Aの動作 |
|---|---|---|---|
| LINE_OK | 改行を除いた終端付き入力。空行も含む | 0 | Cへ渡す |
| LINE_CANCELLED | 空文字列。途中入力を使わない | 0 | 再入力、直前コードを維持 |
| LINE_EOF | 空文字列 | 0 | 後片付けして終了 |
| LINE_TOO_LONG | 空文字列。残りを行末まで捨てる | 0 | 報告後に再入力 |
| LINE_ERROR | 空文字列 | 保存値または10節の非0値 | D03: 報告して非0終了する案 |

出力先がコンソールならWriteConsoleW、ファイル等ならUTF-8へ変換してWriteFileを使う方針です。全体を書き切るまで進め、成功扱いで0単位しか進まない場合も失敗へ変換します。通常出力はstdout、診断はstderr、改行はCRLF、UTF-8出力にBOMは付けない案です。エラー文の全文一致は確認条件にせず、文脈とエラー値を確認します。

出力関数は渡された文字列へ改行を自動追加しません。コマンド側が仕様どおりの改行を文字列へ含め、プロンプトや分割出力へ余分な改行を入れないようにします。

ConsoleReadResultをゼロ初期化すると、現在の宣言ではstatusがLINE_OKになります。ゼロ初期化だけを完了結果として返さず、全経路でstatusを明示します。LINE_ERROR=0へ並べ替える案はD05の代案であり、採用する場合は呼び出し側とテストも同時に合わせます。ParseResultも同様に、PARSE_OKを明示した結果だけを利用します。

ReadConsoleWはコンソールの入力用で、別イベントを引数に渡して待機を解除するAPIではありません。中断方式と入力モードの実機確認をD02から落とさないでください。[公式資料](https://learn.microsoft.com/en-us/windows/console/readconsole)

## 4. 解析: parser.h / C

~~~c
#include "shell_types.h"

typedef enum {
    PARSE_OK,
    PARSE_EMPTY,
    PARSE_TOO_LONG,
    PARSE_TOO_MANY_ARGS,
    PARSE_INVALID_SYNTAX,
    PARSE_NO_MEMORY,
    PARSE_API_ERROR
} ParseStatus;

typedef struct {
    ParseStatus status;
    DWORD win32_error;
} ParseResult;

ParseResult parse_command(const wchar_t *line, Command *out_command);
void command_free(Command *command);
~~~

前提: lineは改行除去済み・終端付きの非NULL文字列、out_commandは非NULLの空Commandです。生きたCommandを上書きして再利用せず、先にcommand_freeします。入力lineは借用で変更しません。

- 長さ上限は先頭空白を取り除く前の入力全体へ適用する。本文4,095単位を超えたらPARSE_TOO_LONG。
- PARSE_OKだけが有効なCommandを返す。argcは1〜64、argv[0]は非空。
- PARSE_EMPTYを含む成功以外の全結果は空Commandを返す。Cが途中の確保も片付ける。
- raw_lineはmalloc系による独立コピー、argvはCommandLineToArgvWの戻り値。
- command_freeはraw_lineをfree、argvを1回のLocalFreeで解放し、全フィールドを空状態へ戻す。空Commandへの繰り返し呼び出しを安全にする。
- command_free(NULL)は準備版では前提外。空構造体とNULLポインタを混同しない。
- malloc系失敗はPARSE_NO_MEMORY、CommandLineToArgvW等のAPI失敗はPARSE_API_ERROR。win32_errorはPARSE_API_ERRORだけで保存値を返し、他は0。API側のメモリ不足もここではPARSE_API_ERRORとする。後片付け前にエラー値を保存する。

CommandLineToArgvWの確保単位と解放方法は[公式資料](https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-commandlinetoargvw)を参照します。

## 5. 組み込み: builtins.h / D・E

~~~c
#include "shell_types.h"

typedef struct {
    BOOL handled;
    BOOL output_failed;  /* D03/D05: 出力失敗をAへ伝える追加案 */
    DWORD exit_code;
    DWORD win32_error;
} BuiltinResult;

BuiltinResult builtin_fs_execute(
    const Command *command, ShellState *state);
BuiltinResult builtin_core_execute(
    const Command *command, ShellState *state);
~~~

前提: PARSE_OKのCommand、非NULLのstate。Commandを借用します。

| 戻り値 | 意味 | Aの動作 |
|---|---|---|
| handled=FALSE | この担当のコマンド名ではない。output_failedを含む残りのフィールドは0 | 次の担当へ判定を渡す |
| handled=TRUE, output_failed=FALSE, exit_code=0 | 既知コマンドが成功 | コードを保持 |
| handled=TRUE, output_failed=FALSE, exit_code=1, win32_error=0 | 引数誤りなどChellのエラー | 外部exeへ渡さない |
| handled=TRUE, output_failed=FALSE, exit_code=1, win32_error!=0 | API失敗 | 外部exeへ渡さない。既知エラーとして扱う |
| handled=TRUE, output_failed=TRUE, exit_code=1 | 通常出力または診断出力の失敗。win32_errorは出力の保存値 | Commandを解放し1で終了する案（D03） |

D/Eは名前が一致する前に入力を読み取ったり、状態を変更したり、未知コマンドのメッセージを表示したりしません。出力にはBを使います。担当で処理したエラーはB経由で文脈を付けて表示し、Aは二重報告しません。通常出力失敗も成功扱いにしません。

output_failedは過去の3フィールドからの追加提案です。出力失敗時は再報告を試し続けず、Aへ返します。診断出力も失敗した場合、win32_errorにはその出力失敗を返し、元の操作エラーと混同しません。output_failed=TRUEならexit_requestedの値より先に異常終了を選びます。

exitは引数なしのときだけstate->exit_requestedをTRUEにします。exit/ExitProcessを直接呼びません。コマンドの詳細は動作仕様4節を参照してください。

## 6. 外部実行: process.h / A

~~~c
#include "shell_types.h"

typedef struct {
    BOOL launched;
    BOOL completed;
    DWORD exit_code;
    DWORD win32_error;
} ProcessResult;

ProcessResult process_execute(const Command *command);
~~~

前提: PARSE_OKのCommandで、組み込みに処理されなかったもの。process_executeは文字列を借用し、画面表示しません。解決パス・起動用コピー・プロセスとスレッドのHANDLEを内部で管理します。

| launched | completed | 意味 |
|---|---|---|
| FALSE | FALSE | 検索・確保・起動等の段階で失敗 |
| TRUE | TRUE | 起動と終了コード取得が完了。exit_codeが有効 |
| TRUE | FALSE | 起動したが、待機・終了取得に失敗。exit_codeは無効 |
| FALSE | TRUE | 返してはいけない組み合わせ |

使用しないフィールドは0で初期化します。子の非0終了はlaunched=TRUE、completed=TRUEで、win32_error=0です。

ProcessResultにはChell固有の検索・確保失敗のstatusがありません。D05では次の対応案を使います: 見つからないexeはERROR_FILE_NOT_FOUND、対応外拡張子はERROR_NOT_SUPPORTED、確保失敗はERROR_NOT_ENOUGH_MEMORY。実際のAPI失敗はAPIのエラー値を保存します。別のProcessStatusを追加するなら呼び出し側も更新します。

起動後の待機失敗からの復帰方針はD04です。子の終了を確認しないまま次のコマンドへ進みません。[CreateProcessW公式資料](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessw)

## 7. 統合: shell.h / A

~~~c
#include <windows.h>
DWORD shell_run(void);
~~~

Aは入力バッファー・ShellStateと、Commandを借りる各関数の呼び出し順を管理します。戻り値は最終終了コードです。空行・中断でlast_exit_codeを上書きしません。mainから1回呼び、mainの戻り値へどう対応させるかは実装で明示します。

mainのintとDWORDは同じ符号・範囲ではありません。終了コード全32ビットの保持方法（MSVCでの変換と外側の観測を含む）はA/Eの確認対象です。子のコード259も、プロセスHANDLEが終了シグナル状態になった後なら実際の終了コードとして扱います。

## 8. 所有権と寿命の図

「所有」は、使うだけでなく、解放や生存期間を管理する責任です。担当者の記号と、実行時の所有者は別です。

~~~mermaid
flowchart TD
    INPUT["A: 入力bufferを所有"] -->|Bが範囲内へ書き込む| LINE["終端付きの1行"]
    LINE -->|Cへ借用| PARSE["C: 解析"]
    PARSE --> RAW["raw_line: malloc系"]
    PARSE --> ARGV["argv: CommandLineToArgvW"]
    RAW --> CMD["A: 成功したCommandを受領"]
    ARGV --> CMD
    CMD -->|借用・変更しない| FS["D/E: 組み込み"]
    CMD -->|借用・変更しない| PROC["A: 外部実行"]
    PROC --> COPY["起動用の可変コピー・子HANDLE"]
    COPY --> PCLEAN["process内部でfree / CloseHandle"]
    CMD -->|処理後に必ず呼ぶ| FREE["C: command_free"]
    FREE --> FRAW["raw_lineをfree"]
    FREE --> FARGV["argvをLocalFree"]
    FRAW --> EMPTY["空Commandへ戻す"]
    FARGV --> EMPTY
~~~

| 資源 | 作成・取得 | 寿命・解放 |
|---|---|---|
| 入力buffer | A | Bは借用。Aのスコープで管理 |
| raw_line / argv | C | Aがcommand_freeを呼び、Cの正しい解放方法へ集約 |
| 起動用コピー | process | processが解放 |
| 子のプロセス・スレッドHANDLE | process | processがCloseHandle |
| ディレクトリ列挙HANDLE | D | DがFindClose |
| 借用した標準HANDLE | BがGetStdHandleで取得 | BはCloseHandleしない |
| エラー文のシステム確保領域 | B | BがLocalFree |
| 中断イベント | 過去設計ではA。Eモジュール化はD02 | Bは借用。実行中ハンドラーの使用終了前に閉じない |

## 9. 中断APIを固定する前の確認事項: D02

Eへ共通制御の実装を配分する案を採用する場合に、interrupt.h / src/E/interrupt.cを追加します。以下の契約を先に決め、関数名・引数・構造体を後から確定します。存在しない宣言を合意済みAPIとして掲載しません。

1. 初期化失敗を呼び出し側へ返し、部分初期化も片付ける。
2. Aがシェルの生存期間を管理し、Bがイベントを借用する。
3. 入力開始前のリセットと、入力中断時の読み取り解除・残入力破棄を定義する。
4. 子の実行中は親を存続させ、子の終了確認後に入力へ戻る。
5. 制御ハンドラー内では通知に限定し、表示・メモリ解放・組み込みを実行しない。
6. 登録解除と実行中ハンドラーの同期を確認してからイベントを破棄する。
7. デバッガー外の通常コンソールでCtrl+CとCtrl+Breakを確認する。

制御ハンドラーや子のイベント受信に関しては[公式資料](https://learn.microsoft.com/en-us/windows/console/setconsolectrlhandler)を参照します。単にCTRL+C無視属性を親へ設定して済ませないでください。

## 10. 型と失敗を読むための早見表

| 表記 | なぜ使うか | 呼び出す側の注意 |
|---|---|---|
| wchar_t *buffer / size_t capacity | UTF-16配列と要素数を渡す | sizeofのバイト数をそのまま渡さない |
| const wchar_t *line | 入力文字列を借りて読む | 呼び出し終了まで終端付き文字列を生存させる |
| Command *out_command | 呼び出し元の構造体へ結果を書き込む | Command command = {0}; とし、&commandを渡す |
| const Command *command | 同じ解析結果を各担当が借りる | 文字列・配列の変更や解放をしない |
| ShellState *state | exit要求を呼び出し元へ伝える | last_exit_codeの更新はAへ集約 |
| enum status | 空・中断・入力誤り・API失敗を分ける | 非0をすべて同じ失敗として扱わない |
| DWORD win32_error | 後から原因を報告する | API失敗の直後に保存し、後片付け後も維持 |
| BOOL handled | 名前を担当したかを返す | 処理失敗でもTRUE。未知だけFALSE |
| BOOL output_failed | 出力装置の失敗で継続を止める案 | handled=TRUEのとき有効（D03/D05） |

Win32エラー型を持つAPIの共通対応案（D05）:

| 失敗 | 返すエラー |
|---|---|
| Win32 APIの失敗 | そのAPIの規約どおりに直後に保存したGetLastError |
| malloc系の失敗（出力・process内） | ERROR_NOT_ENOUGH_MEMORY |
| 対話入力以外の入力元 | ERROR_NOT_SUPPORTED |
| 解決対象exeが見つからない | ERROR_FILE_NOT_FOUND |
| 対応外の実行拡張子 | ERROR_NOT_SUPPORTED |
| 書き込みが成功扱いでも0単位で進まない | ERROR_WRITE_FAULT |

引数不正・構文不正はstatusやexit_codeで返し、通常はwin32_error=0です。公開APIの前提違反（NULLや実領域を超すcapacity等）は、利用者の入力誤りと別です。前提違反への防御を入れても、正しい呼び出し側を省略しません。

## 11. ビルドへの接続（反映時の案）

| 対象 | 必要な登録 | 理由 |
|---|---|---|
| Chell本体 | 採用した配置のmain.c、shell.c、process.c、B/C/D/Eの本体Cをソース一覧へ明示 | 現在のCMakeはsrc/main.cだけを登録している |
| parserを含むターゲット | parser.cでshellapi.hをincludeし、Shell32をリンク | CommandLineToArgvWの宣言とライブラリが必要 |
| 本体・単独確認・テスト用子exe | 別ターゲットに分け、必要なinclude・UNICODE / _UNICODE・C17・警告設定を与える | mainの重複や、本体と異なる文字列設定を避ける |
| 機能テスト | テストexeをビルドし、CTestにも登録 | exeができただけでは検査は実行されない |

これは反映時の説明であり、この作業で本体やテストを登録したものではありません。過去資料の共通ターゲット名を、現行リポジトリにも存在する前提で使いません。

## 12. ヘッダー反映前のチェック

- 呼び出す側と実装する側で、引数・戻り値・空状態・失敗時の後片付けを説明できる。
- 各ヘッダーは単独でincludeでき、Cファイルのinclude順へ依存しない。
- API表、共有ヘッダー、図、テスト期待値に同じ名前・型を使う。
- この文書の宣言案を本体へ反映する際、D01〜D06の関連項目を記録する。
