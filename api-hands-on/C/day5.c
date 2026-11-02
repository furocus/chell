/*
 * C / Day 5：argvの**と構造体への*を区別する
 * 担当：解析・所有権
 * 実API・CRTを読む見本。実行・ビルド・環境構築は不要。
 * Windows/CRTヘッダーが見つからない表示を、補完の正誤判定に使わない。
 *
 * 【読む30分】見本10分、公式docs10分、answer.mdの概念説明10分。
 * CommandLineToArgvW：https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-commandlinetoargvw
 * 指定節：Syntax、空入力、確保とLocalFree
 * 【実践90分】TODO補完40分、値と所有権を手で追う25分、解答照合15分、説明10分。
 * 今日の作業は1関数だけ。本体機能全体を追加しない。
 * 問い：フィールドargvが**だからといって、out_parsedも**や***にする必要があるか。
 * ヒント：out_parsed->argcの型はint。itemsの型はwchar_t **。各値の型を一つずつ書く。
 */
#include <windows.h>
#include <shellapi.h>

typedef struct { int argc; wchar_t **argv; } Parsed;

/* 実APIの見本：戻り値の**は引数文字列を指すポインタの表。
   argcの出力先は整数変数の場所なのでint *。 */
static wchar_t **example_split(const wchar_t *line, int *out_count)
{
    return CommandLineToArgvW(line, out_count);
}

static void example_free_parsed(Parsed *parsed)
{
    if (parsed == NULL) return;
    if (parsed->argv != NULL) LocalFree(parsed->argv);
    parsed->argv = NULL;
    parsed->argc = 0;
}

/* 補完する1関数 */
int split_for_parser(const wchar_t *line, Parsed *out_parsed)
{
    if (out_parsed == NULL) return 0;
    out_parsed->argc = 0;
    out_parsed->argv = NULL;
    if (line == NULL || line[0] == L'\0') return 0;
    wchar_t **items = NULL;
    /* TODO 1: example_splitを呼ぶ。個数の保存先には何のアドレスを渡すか。 */
    if (items == NULL) { out_parsed->argc = 0; return 0; }
    /* TODO 2: 取得した表の所有ポインタをout_parsedのargvへ保存する。 */
    return 1;
}

/* 実行の代わりに、この場面の状態と所有権を手で追う。 */
static void walkthrough(void)
{
    Parsed parsed = {0};
    int ok = split_for_parser(L"echo \"hello world\" \"\"", &parsed);
    /* 予想：argc、argv[1]、argv[2]。各要素を個別にfreeしてよいか。 */
    if (ok) example_free_parsed(&parsed);
    /* 予想：解放後のargcとargv。 */
}

/* メモ：なぜその段数か／作成元と解放方法／未解決点（この欄だけでよい）。 */
