#include <stdio.h>
#include <wchar.h>  /*ワイド文字列を使う関数*/
#include <locale.h> /*文字の処理に関する環境設定*/

#define MAX_LINE_LENGTH                                                        \
    4096 /*入力できる最大文字数を4096に設定するマイクロ定義*/

enum {            /*enum(列挙型)、関係する関数に名前を付けてまとめるもの*/
    LINE_OK = 0,  /*正常に読み取れた*/
    LINE_EOF,     /*ファイルの終わりに達した*/
    LINE_ERROR,   /*エラーが発生した*/
    LINE_TOO_LONG /*入力された文字列が長すぎる*/
};

int console_read_line(wchar_t buffer[], int size); /*プロトタイプ宣言*/

int console_read_line(
    wchar_t buffer[],
    int size) /*キーボードから入力された文字列を関数に保存する*/

{
    int count = 0;    /*入力された文字数を数える関数*/
    int max_length;   /*入力できる最大文字数*/
    int too_long = 0; /*入力された文字列が長すぎるかどうかを判定する関数*/
    int saw_char = 0; /*入力された文字があるかどうかを判定する関数*/
    wint_t ch;        /*ワイド文字列を扱う関数*/

    if (buffer == NULL || size <= 0) {
        return LINE_ERROR;
    }

    max_length = size - 1;

    if (max_length > MAX_LINE_LENGTH - 1) {
        max_length = MAX_LINE_LENGTH - 1;
    }

    while (1) /*無限ループ（１）なので*/
    {
        ch = fgetwc(stdin); /*キーボードからワイド文字列を読み取る*/

        if (ch == WEOF) /*もしエラーならループを出る*/
        {
            break;
        }
        if (ch == L'\n') /*もし改行ならループを出る*/
        {
            break;
        }

        saw_char = 1; /*入力された文字があることを示す*/

        if (!too_long) /*もし文字列が長すぎないなら（！はNOT）*/
        {
            if (count < max_length) /*入力された文字が最大文字数未満なら*/
            {
                buffer[count] =
                    (wchar_t)ch; /*入力されたワイド文字をバッファに保存する*/
                count++;         /*カウントを＋１する*/
            } else {
                too_long = 1; /*容量が足りないとき１を入れる*/
            }
        }
    }

    if (ferror(stdin)) {
        buffer[count] = L'\0';
        return LINE_ERROR;
    }
    if (ch == WEOF && !saw_char && !too_long) {
        buffer[0] = L'\0';
        return LINE_EOF;
    }
    if (count > 0 &&
        buffer[count - 1] == L'\r') /*もし最後の文字が改行コードなら*/
    {
        count--;
    }

    buffer[count] = L'\0'; /*文字列の最後にNULLを入れる*/
    /*NULLを入れないとどこまで表示していいかわからないから*/
    if (too_long) {
        return LINE_TOO_LONG;
    }

    return LINE_OK; /*正常に読み取れた*/
}

int main(void)
{
    wchar_t name[100];
    int result;

    setlocale(LC_ALL, "");

    wprintf(L"文字を入力してください：");

    result = console_read_line(name, 100);

    wprintf(L"\n戻り値：%d\n", result);

    wprintf(L"\n読み取り結果：");

    if (result == LINE_OK) {
        wprintf(L"正常に読み取りました\n");
    } else if (result == LINE_EOF) {
        wprintf(L"入力が終了しました\n");
    } else if (result == LINE_ERROR) {
        wprintf(L"読み取りエラーです\n");
    } else if (result == LINE_TOO_LONG) {
        wprintf(L"入力文字数が多すぎます\n");
    }

    return 0;
}