/*
 * A / Day 5：Win32の可変コマンドラインと出力先
 * 担当：制御・外部実行
 * 実API・CRTを読む見本。実行・ビルド・環境構築は不要。
 * Windows/CRTヘッダーが見つからない表示を、補完の正誤判定に使わない。
 *
 * 【読む30分】見本10分、公式docs10分、answer.mdの概念説明10分。
 * CreateProcessW：https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessw
 * 指定節：lpCommandLine、lpProcessInformation、Return value
 * 【実践90分】TODO補完40分、値と所有権を手で追う25分、解答照合15分、説明10分。
 * 今日の作業は1関数だけ。本体機能全体を追加しない。
 * 問い：out_lineは**なのに、out_processは*でよいのはなぜか。
 * ヒント：Day 2のchar版を、UTF-16の要素数からバイト数への換算へ読み替える。
 */
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

/* 実APIの見本：構造体の中身を埋めるのでout_processは単ポインタ。
   lpCommandLineはW版APIに書き換えられ得る。読み取り専用文字列は渡さない。
   成功したhThreadとhProcessは、利用後それぞれCloseHandleが必要。 */
static BOOL example_start(
    const wchar_t *exe, wchar_t *mutable_line, PROCESS_INFORMATION *out_process)
{
    if (out_process == NULL) {
        return FALSE;
    }
    STARTUPINFOW startup = {0};
    startup.cb = sizeof startup;
    ZeroMemory(out_process, sizeof *out_process);
    return CreateProcessW(
        exe,
        mutable_line,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &startup,
        out_process);
}

/* 補完する1関数 */
int prepare_win32_line(const wchar_t *raw, wchar_t **out_line)
{
    if (out_line == NULL) {
        return 0;
    }
    /* TODO 1: 呼び出し元のポインタをNULLへ初期化する。 */
    if (raw == NULL) {
        return 0;
    }
    size_t bytes = (wcslen(raw) + 1) * sizeof(wchar_t);
    wchar_t *copy = NULL;
    /* TODO 2: bytesを確保してcopyへ代入し、失敗時は0を返す。 */
    /* TODO 3: 終端込みでコピーし、呼び出し元へ所有ポインタを渡す。 */
    return 1;
}

/* 実行の代わりに、この場面の状態と所有権を手で追う。 */
static void walkthrough(void)
{
    const wchar_t *raw = L"tool.exe --label sample";
    wchar_t *line = NULL;
    int ok = prepare_win32_line(raw, &line);
    /* 予想：rawは変わるか。lineの解放は誰が行うか。
       example_startはここでは呼ばない。プロセス起動の課題ではない。 */
    if (ok) {
        free(line);
    }
}

/* メモ：なぜその段数か／作成元と解放方法／未解決点（この欄だけでよい）。 */
