#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <windows.h>
#include <shellapi.h>

#define MAX_LINE_LENGTH 1000 //仮で数字決めてる

/* Command構造体 */
typedef struct {
    int argc;          //コマンドの引数の数
    wchar_t **argv;    //コマンドの引数を入れる配列
    wchar_t *raw_line; //コマンドの元の文字列
} Command;

/* parse_command */
int parse_command(const wchar_t *line, Command *out_command)
{
    if (out_command == NULL) // 構造体のアドレスがNULLかどうか確認
    {
        return 1; // 構造体にアクセスできないため失敗を返す
    }

    *out_command = (Command){0}; // 構造体の各メンバーを初期化

    if (line == NULL) // 入力文字列のアドレスがNULLかどうか確認
    {
        return 1; // 入力文字列がないため失敗を返す
    }

    const wchar_t *trimmed_line = line; // 先頭の空白を読み飛ばすためのポインタ

    while (*trimmed_line == L' ' ||
           *trimmed_line == L'\t') // 文字列の先頭にある空白とタブを読み飛ばす
    {
        trimmed_line++;
    }

    if (*trimmed_line ==
        L'\0') // 空白とタブを読み飛ばした結果、文字列が空なら失敗
    {
        return 1; // 失敗を返す（0が成功、1が失敗）
    }
    if (wcslen(trimmed_line) > MAX_LINE_LENGTH) {
        return 1; // 文字数の上限を超えているため失敗
    }
    int argc = 0;          //引数の数を数える変数（初期化）
    wchar_t **argv = NULL; //引数の文字列を格納する配列のアドレス（初期化）

    size_t bytes =
        (wcslen(trimmed_line) + 1) *
        sizeof(
            wchar_t); // 文字列の長さに終端文字L'\0'を1文字分加え、必要なバイト数を計算

    wchar_t *copy =
        malloc(bytes); //メモリの確保、メモリの先頭アドレスをcopyに保存

    if (copy == NULL) //メモリの確保に失敗したか確認
    {
        return 1; //失敗を返す
    }

    memcpy(copy, trimmed_line, bytes); //入力した文字列を確保したメモリにコピー

    out_command->raw_line = copy; //コピーした文字列のアドレスをraw_lineに保存

    int in_quotes =
        0; // 引用符の内側にいるかどうかを管理する変数（0：外側、1：内側）

    for (int i = 0; copy[i] != L'\0';
        i++) // 文字列の先頭から終端まで1文字ずつ確認
    {
        if (copy[i] == L'"') // ダブルクォーテーションが見つかったか確認
        {
            int j = i - 1;           // 引用符の1つ前の位置を設定
            int backslash_count = 0; // バックスラッシュの数を数える変数

            while (j >= 0 &&
                   copy[j] == L'\\') // 直前のバックスラッシュが続く間、繰り返す
            {
                backslash_count++; // バックスラッシュの数を1つ増やす
                j--;               // 1つ前の文字へ移動
            }

            if (backslash_count % 2 == 0) // バックスラッシュの数が偶数か確認
            {
                in_quotes = !in_quotes; // 引用符の内側と外側を切り替える
            }
        }
        if (in_quotes == 0 &&      // 引用符の外側か確認
            (copy[i] == L'|' ||    // パイプか確認
                copy[i] == L'&' || // アンパサンドか確認
                copy[i] == L'<' || // 小なり記号か確認
                copy[i] == L'>'))  // 大なり記号か確認
        {
            free(copy);                  // raw_line用に確保したメモリを解放
            *out_command = (Command){0}; // Command構造体を空の状態に戻す
            return 1;                    // 演算子が見つかったため失敗を返す
        }
    }

    if (in_quotes == 1) // 文字列の最後まで確認しても引用符の内側だった場合
    {
        free(copy);                  // raw_line用に確保したメモリを解放
        *out_command = (Command){0}; // Command構造体を空の状態に戻す
        return 1;                    // 引用符が閉じられていないため失敗を返す
    }

    argv = CommandLineToArgvW(
        trimmed_line,
        &argc); // 入力文字列を引数ごとに分解し、配列のアドレスをargvに、引数の数をargcに保存

    if (argv == NULL) // 引数の解析に失敗したか確認
    {
        free(copy);                  // raw_line用に確保したメモリを解放
        *out_command = (Command){0}; // Command構造体を空の状態に戻す
        return 1;                    // 失敗を返す
    }

    if (argc > 64) // コマンド名を含めた引数の数が64個を超えていないか確認
    {
        LocalFree(argv); // CommandLineToArgvWが確保した引数の配列を解放
        free(copy);      // raw_line用に確保したメモリを解放
        *out_command = (Command){0}; // Command構造体を空の状態に戻す
        return 1;                    // 失敗を返す
    }

    if (argc == 0 || argv[0][0] == L'\0') //頭文字チェック
    {
        LocalFree(argv); //CommandLineToArgvWが確保した引数の配列を解放
        free(copy);      // raw_line用に確保したメモリを解放
        *out_command = (Command){0}; // Command構造体を空の状態に戻す
        return 1;                    // 失敗を返す
    }

    out_command->argc = argc; // 解析した引数の数をCommand構造体のargcに保存
    out_command->argv =
        argv; // 解析した引数の配列のアドレスをCommand構造体のargvに保存

    return 0; //成功を返す
}

/* command_free */
void command_free(Command *command)
{
    if (command == NULL) // Command構造体のアドレスがNULLか確認
    {
        return; // 解放対象が存在しないため処理を終了
    }

    if (command->argv != NULL) // 引数配列のアドレスがNULLではないか確認
    {
        LocalFree(command
                ->argv); // CommandLineToArgvWで確保された引数配列のメモリを解放
    }

    if (command->raw_line !=
        NULL) // 元の入力文字列のアドレスがNULLではないか確認
    {
        free(command->raw_line); // mallocで確保した元の入力文字列のメモリを解放
    }

    *command = (Command){0}; // 解放後、各メンバーを初期状態に戻す
}
int main(void)
{
