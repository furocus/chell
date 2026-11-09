/*
 * A / Day 4：ポインタの表を読む
 * 担当：制御・外部実行
 * 実行・ビルド・環境構築は不要。コードを補い、answer.mdのDay 4で照合する。
 *
 * 【読む30分】見本10分、下記公式docsの指定節10分、answer.mdの概念説明10分。
 * CommandLineToArgvW：https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-commandlinetoargvw
 * 読む箇所：Syntax、Remarksのポインタの表だけ。引用規則は対象外
 *
 * 【実践90分】TODO補完40分、下の場面を手で追う25分、解答照合15分、説明10分。
 * 子へ渡す引数の表を読む。argvから元の引用符付き入力を再構築する課題ではない。
 * 編集は下の1関数だけ。main、テスト、ヘッダー、作業記録ファイルは作らない。
 *
 * 問い：argvの二重ポインタ相当の型と、out_argumentの二重ポインタは、同じ用途か。char text[3][16]と同じ配置か。
 * ヒント：argv[index]はポインタ、argv[index][0]は文字。コピー、解放、文字列の正規化は不要。
 * 2時間になったら未解決点を最後のメモへ1つ記して終了。追加課題はない。
 */
#include <stddef.h>

/* 見本：表の先頭から、1本の文字列ポインタを借りる。 */
static const char *example_first(const char *const values[], size_t count)
{
    if (values == NULL || count == 0) {
        return NULL;
    }
    return values[0];
}

/* 補完する1関数 */
int select_process_argument(
    const char *const argv[], size_t count, size_t index,
    const char **out_argument)
{
    if (out_argument == NULL) {
        return 0;
    }
    /* TODO 1: 出力先のポインタをNULLへ初期化する。 */
    if (argv == NULL || index >= count) {
        return 0;
    }
    /* TODO 2: 指定要素の文字列ポインタを借用して返す。 */
    return 1;
}

/* 実行の代わりに、変数の変化を紙やエディター上で追う場面。 */
static void walkthrough(void)
{
    const char *arguments[] = {"tool.exe", "--label", "sample"};
    const char *selected = NULL;
    const char *first = example_first(arguments, 3);
    int ok = select_process_argument(arguments, 3, 1, &selected);
    /* 予想：first、selected、selected[0]のそれぞれが指す／表すもの。 */
    /* index=3、count=0、NULL出力先へ変えた場合を手で追う。 */
    (void)first;
    (void)ok;
    (void)selected;
}

/* メモ：誰の何を変更したか／借用か所有か／未解決点（この欄だけでよい）。 */
