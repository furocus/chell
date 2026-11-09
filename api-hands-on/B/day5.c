/*
 * B / Day 5：APIが確保する説明文を受け取る
 * 担当：入力・表示
 * 実API・CRTを読む見本。実行・ビルド・環境構築は不要。
 * Windows/CRTヘッダーが見つからない表示を、補完の正誤判定に使わない。
 *
 * 【読む30分】見本10分、公式docs10分、answer.mdの概念説明10分。
 * FormatMessageW：https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-formatmessagew
 * 指定節：ALLOCATE_BUFFER、lpBuffer、Return value
 * LocalFree：https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-localfree
 * 指定節：Return value
 * 【実践90分】TODO補完40分、値と所有権を手で追う25分、解答照合15分、説明10分。
 * 今日の作業は1関数だけ。本体機能全体を追加しない。
 * 問い：messageの場所は既にout_ownedに入っている。さらに&out_ownedを渡すと、何の場所になるか。
 * ヒント：「呼び出し元のwchar_t *変数へ書く場所」はどの変数の値かを確認する。
 */
#include <windows.h>

/* 実APIの見本：ALLOCATE_BUFFER時は、lpBufferへ「ポインタの保存先」を渡す。
   通常の文字配列出力とは契約が異なる。成功した領域はLocalFreeする。 */
static DWORD example_format(DWORD code, wchar_t **out_message)
{
    return FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        code,
        0,
        (LPWSTR)out_message,
        0,
        NULL);
}

/* 補完する1関数 */
int receive_error_message(DWORD code, wchar_t **out_owned)
{
    if (out_owned == NULL) {
        return 0;
    }
    /* TODO 1: 出力先をNULLへ初期化する。 */
    DWORD characters = 0;
    /* TODO 2: 上の0をexample_formatの呼び出しへ置き換える。
       out_ownedと&out_ownedのどちらを渡すか、先に図で考える。 */
    if (characters == 0) {
        return 0;
    }
    return 1;
}

/* 実行の代わりに、この場面の状態と所有権を手で追う。 */
static void walkthrough(void)
{
    wchar_t *message = NULL;
    int ok = receive_error_message(ERROR_FILE_NOT_FOUND, &message);
    /* 予想：成功後のmessage、解放方法、説明文がOS言語で変わるか。 */
    if (ok) {
        LocalFree(message);
    }
    /* 失敗時と、出力先自体がNULLの場合はAPIを呼ぶ前から手で追う。 */
}

/* メモ：なぜその段数か／作成元と解放方法／未解決点（この欄だけでよい）。 */
