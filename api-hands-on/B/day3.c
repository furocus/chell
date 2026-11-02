/*
 * B / Day 3：解放とポインタのリセットを分ける
 * 担当：入力・表示
 * 実行・ビルド・環境構築は不要。コードを補い、answer.mdのDay 3で照合する。
 *
 * 【読む30分】見本10分、下記公式docsの指定節10分、answer.mdの概念説明10分。
 * free：https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/free?view=msvc-170
 * 読む箇所：RemarksのNULLと不正なポインタの扱い
 *
 * 【実践90分】TODO補完40分、下の場面を手で追う25分、解答照合15分、説明10分。
 * 所有する入力コピーを解放する。借りた標準入出力HANDLEにはこの関数を使わない。
 * 編集は下の1関数だけ。main、テスト、ヘッダー、作業記録ファイルは作らない。
 *
 * 問い：free(owned)とfree(*owned)では、対象の場所がどう違うか。owned = NULLでは誰の変数だけが変わるか。
 * ヒント：Day 2で所有権を受け取った変数そのものの場所を渡す。領域を解放する操作と、変数へNULLを入れる操作を1行ずつ書く。
 * 2時間になったら未解決点を最後のメモへ1つ記して終了。追加課題はない。
 */
#include <stdlib.h>

/* 見本：ポインタではなく、呼び出し元のintを0へ戻す。 */
static void example_reset_number(int *number)
{
    if (number != NULL) *number = 0;
}

/* 補完する1関数 */
void release_input_copy(char **owned)
{
    if (owned == NULL) return;
    /* TODO 1: 呼び出し元が所有する領域をfreeする。NULLでもfreeは許される。 */
    /* TODO 2: 呼び出し元のポインタ変数へNULLを代入する。 */
}

/* 実行の代わりに、変数の変化を紙やエディター上で追う場面。 */
static void walkthrough(void)
{
    char *saved = malloc(8);
    /* malloc失敗ならsavedはNULL。どちらの場合も片付けられる契約。 */
    release_input_copy(&saved);
    release_input_copy(&saved);
    release_input_copy(NULL);
    int number = 7;
    example_reset_number(&number);
    /* 予想：saved、number、実際に解放する回数。 */
    /* aliasがsavedと同じ領域を指していた場合、aliasもNULLになるか紙で答える。
       解放後の領域を実際に読んではいけない。 */
    (void)number;
}

/* メモ：誰の何を変更したか／借用か所有か／未解決点（この欄だけでよい）。 */
