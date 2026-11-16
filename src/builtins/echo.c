#include <stdio.h>
#include <wchar.h>
//TODO: ヘッダファイルを作成して契約に則った責任分割を行う

int echo(int argc, const wchar_t *const argv[]) 
{
    int i; //ループカウンタ

    for(i = 0; i < argc; i++) {
        // fputwcの戻り値が0以下なら、書き込み失敗。
        if(i > 1 && fputwc(L' ', stdout) < 0) {
            // 出力に失敗したので、残りを出力せず関数を終了する。
        return 1;
        }
        // fputwcの戻り値がWEOFなら、書き込み失敗。
        if(fputws(argv[i], stdout) < 0) {
            // 出力に失敗したので、残りを出力せず関数を終了する。
            return 1;
        }
    }

    // 全引数の出力後に、改行を1文字書く。
    // 改行の出力に失敗したら1、成功したら0を返す。
    return fputwc(L'\n', stdout) == WEOF ? 1 : 0;
}

//テストコード　実環境ではコメントアウトまたは削除してください
/*
int main(void)
{
    const wchar_t *args[] = {L"echo", L"hello", L"world"};

    return echo(3, args);
}
*/
