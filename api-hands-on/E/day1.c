/*
 * E / Day 1：固定文を借用して返す
 * 担当：基本コマンド
 * 実行・ビルド・環境構築は不要。コードを補い、answer.mdのDay 1で照合する。
 *
 * 【読む30分】見本10分、下記公式docsの指定節10分、answer.mdの概念説明10分。
 * Pointer Declarations：https://learn.microsoft.com/en-us/cpp/c-language/pointer-declarations?view=msvc-170
 * 読む箇所：宣言と間接参照
 *
 * 【実践90分】TODO補完40分、下の場面を手で追う25分、解答照合15分、説明10分。
 * コマンドの説明文を表から借用する。未知名と、既知名の説明文を分けて返す。
 * 編集は下の1関数だけ。main、テスト、ヘッダー、作業記録ファイルは作らない。
 *
 * 問い：out_note = entries[i].noteではなく、*out_noteへ代入するのはなぜか。
 * ヒント：「どの変数の中身を変えるか」を見る。out_noteは関数内の引数変数、*out_noteは呼び出し元の変数。
 * 2時間になったら未解決点を最後のメモへ1つ記して終了。追加課題はない。
 */
#include <stddef.h>
#include <string.h>

typedef struct { const char *key; const char *note; } NoteEntry;

/* 見本：int変数の中身を変更する。&numberを渡すのでint *で足りる。 */
static void example_set_number(int *out_number)
{
    if (out_number != NULL) *out_number = 7;
}

/* 補完する1関数 */
int find_builtin_note(const NoteEntry *entries, size_t count,
                   const char *key, const char **out_note)
{
    if (out_note == NULL) return 0;
    /* TODO 1: 呼び出し元の出力ポインタをNULLに初期化する。 */
    if (key == NULL || (entries == NULL && count != 0)) return 0;
    for (size_t i = 0; i < count; ++i) {
        if (strcmp(entries[i].key, key) == 0) {
            /* TODO 2: この行のnoteを、コピーせず借用で返す。 */
            return 1;
        }
    }
    return 0;
}

/* 実行の代わりに、変数の変化を紙やエディター上で追う場面。 */
static void walkthrough(void)
{
    const NoteEntry entries[] = {
        {"help", "show command help"},
        {"exit", "another fixed note"}
    };
    const char *selected = "before";
    int number = 0;
    example_set_number(&number);
    int found = find_builtin_note(entries, 2, "help", &selected);
    /* 予想：number、found、selectedの値を答える。 */
    /* 次にkeyを"missing"へ変えた場合を、実行せず手で追う。 */
    (void)number; (void)found; (void)selected;
}

/* メモ：誰の何を変更したか／借用か所有か／未解決点（この欄だけでよい）。 */
