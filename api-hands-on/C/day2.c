/*
 * C / Day 2：所有コピーを呼び出し元へ渡す
 * 担当：解析・所有権
 * 実行・ビルド・環境構築は不要。コードを補い、answer.mdのDay 2で照合する。
 *
 * 【読む30分】見本10分、下記公式docsの指定節10分、answer.mdの概念説明10分。
 * malloc：https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/malloc?view=msvc-170
 * 読む箇所：Parameters、Return value
 * memcpy：https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/memcpy-wmemcpy?view=msvc-170
 * 読む箇所：Remarksのバイト数と非重複領域
 *
 * 【実践90分】TODO補完40分、下の場面を手で追う25分、解答照合15分、説明10分。
 * raw_lineを所有コピーし、入力配列が再利用されても元の行を保存する。
 * 編集は下の1関数だけ。main、テスト、ヘッダー、作業記録ファイルは作らない。
 *
 * 問い：char *out_ownedへmallocの結果を代入するだけでは、呼び出し元のsavedが変わらないのはなぜか。
 * ヒント：見本のintの確保を、文字列の長さ＋終端の確保に読み替える。自動変数の配列へのポインタは返さない。
 * 2時間になったら未解決点を最後のメモへ1つ記して終了。追加課題はない。
 */
#include <stdlib.h>
#include <string.h>

/* 見本：確保したintの場所を、呼び出し元のポインタへ返す。 */
static int example_create_number(int **out_number)
{
    if (out_number == NULL) return 0;
    *out_number = NULL;
    int *created = malloc(sizeof *created);
    if (created == NULL) return 0;
    *created = 7;
    *out_number = created;
    return 1;
}

/* 補完する1関数 */
int copy_raw_line(const char *source, char **out_owned)
{
    if (out_owned == NULL) return 0;
    /* TODO 1: 出力先をNULLへ初期化する。 */
    if (source == NULL) return 0;
    size_t bytes = strlen(source) + 1;
    /* TODO 2: bytes分を確保してchar *copyへ受け取る宣言を補う。 */
    /* TODO 3: 確保失敗なら0を返す。本文と終端をコピーし、所有権を渡す。 */
    return 1;
}

/* 実行の代わりに、変数の変化を紙やエディター上で追う場面。 */
static void walkthrough(void)
{
    char source[] = "echo \"hello world\"";
    char *saved = NULL;
    int ok = copy_raw_line(source, &saved);
    source[0] = '#';
    /* 予想：成功時、savedの先頭まで'#'になるか。誰がいつfreeするか。 */
    if (ok) free(saved);
    int *number = NULL;
    if (example_create_number(&number)) free(number);
    /* 空文字列と、mallocがNULLを返す場合も紙で追う。 */
}

/* メモ：誰の何を変更したか／借用か所有か／未解決点（この欄だけでよい）。 */
