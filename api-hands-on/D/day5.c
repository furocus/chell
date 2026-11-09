/*
 * D / Day 5：必要容量を調べて現在位置を返す
 * 担当：現在位置・ディレクトリ
 * 実API・CRTを読む見本。実行・ビルド・環境構築は不要。
 * Windows/CRTヘッダーが見つからない表示を、補完の正誤判定に使わない。
 *
 * 【読む30分】見本10分、公式docs10分、answer.mdの概念説明10分。
 * GetCurrentDirectory：https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getcurrentdirectory
 * 指定節：Parameters、Return value
 * 【実践90分】TODO補完40分、値と所有権を手で追う25分、解答照合15分、説明10分。
 * 今日の作業は1関数だけ。本体機能全体を追加しない。
 * 問い：Windows APIのbufferは*なのに、自作のout_pathは**なのはなぜか。
 * ヒント：sizeof *bufferで1要素のバイト数が得られる。DWORDの非0だけで取得成功と決めない。
 */
#include <windows.h>
#include <stdlib.h>

/* 実APIの見本：用意された配列の中身を埋めるのでbufferは*。
   容量不足なら終端込み必要量、成功なら終端を除く長さを返す。 */
static DWORD example_read_directory(wchar_t *buffer, DWORD capacity)
{
    return GetCurrentDirectoryW(capacity, buffer);
}

/* 補完する1関数 */
int own_current_directory(wchar_t **out_path)
{
    if (out_path == NULL) {
        return 0;
    }
    /* TODO 1: 呼び出し元のポインタをNULLへ初期化する。 */
    DWORD needed = example_read_directory(NULL, 0);
    if (needed == 0) {
        return 0;
    }
    wchar_t *buffer = NULL;
    /* TODO 2: needed要素分のwchar_t領域を確保し、失敗なら0を返す。 */
    DWORD copied = example_read_directory(buffer, needed);
    if (copied == 0 || copied >= needed) {
        free(buffer);
        return 0;
    }
    /* TODO 3: 所有ポインタを呼び出し元へ渡す。 */
    return 1;
}

/* 実行の代わりに、この場面の状態と所有権を手で追う。 */
static void walkthrough(void)
{
    wchar_t *path = NULL;
    int ok = own_current_directory(&path);
    /* 手で追う：needed=12、copied=11、sizeof(wchar_t)=2と仮定。
       確保バイト数、終端の領域、最後にfreeする人を答える。 */
    if (ok) {
        free(path);
    }
    /* copied=12以上や0なら、bufferを返してよいか。 */
}

/* メモ：なぜその段数か／作成元と解放方法／未解決点（この欄だけでよい）。 */
