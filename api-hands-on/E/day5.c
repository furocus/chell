/*
 * E / Day 5：単ポインタで十分な状態更新を選ぶ
 * 担当：基本コマンド
 * 実API・CRTを読む見本。実行・ビルド・環境構築は不要。
 * Windows/CRTヘッダーが見つからない表示を、補完の正誤判定に使わない。
 *
 * 【読む30分】見本10分、公式docs10分、answer.mdの概念説明10分。
 * _wcsicmp：https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/stricmp-wcsicmp-mbsicmp-stricmp-l-wcsicmp-l-mbsicmp-l?view=msvc-170
 * 指定節：Return value、Remarks
 * 【実践90分】TODO補完40分、値と所有権を手で追う25分、解答照合15分、説明10分。
 * 今日の作業は1関数だけ。本体機能全体を追加しない。
 * 問い：argvの表は二重ポインタ相当でも、stateやout_handledを二重ポインタにする理由はあるか。
 * ヒント：整数、構造体、ポインタ変数のどれを書き換えるかで段数を選ぶ。
 */
#include <stddef.h>
#include <wchar.h>

typedef struct {
    size_t argc;
    const wchar_t *const *argv;
} CommandView;
typedef struct {
    int exit_requested;
} ShellState;

/* 見本：既知のASCIIコマンド名をMSVCのCRTで大文字小文字なし比較。
   一致は戻り値0。今回は既定Cロケールを変えない。 */
static int example_is_exit(const wchar_t *name)
{
    return name != NULL && _wcsicmp(name, L"exit") == 0;
}

/* 補完する1関数 */
int apply_exit_request(
    const CommandView *command, ShellState *state, int *out_handled)
{
    if (out_handled == NULL) {
        return 0;
    }
    *out_handled = 0;
    if (command == NULL || state == NULL || command->argc == 0 ||
        command->argv == NULL || !example_is_exit(command->argv[0])) {
        return 0;
    }
    /* TODO 1: 既知コマンドだったことを呼び出し元の整数へ知らせる。 */
    if (command->argc != 1) {
        return 0;
    }
    /* TODO 2: stateのexit_requestedを1へ変更する。 */
    return 1;
}

/* 実行の代わりに、この場面の状態と所有権を手で追う。 */
static void walkthrough(void)
{
    const wchar_t *arguments[] = {L"EXIT"};
    CommandView command = {1, arguments};
    ShellState state = {0};
    int handled = 0;
    int ok = apply_exit_request(&command, &state, &handled);
    /* 予想：ok、handled、exit_requested。
       次に状態を0へ戻し、名前をunknown／引数を2個へ変えて手で追う。 */
    (void)ok;
}

/* メモ：なぜその段数か／作成元と解放方法／未解決点（この欄だけでよい）。 */
