#include <stdio.h>
#include <wchar.h>
//TODO: ヘッダファイルを作成して契約に則った責任分割を行う

int echo(int argc, const wchar_t *const argv[]) 
{
    int i; //ループカウンタ

    for(i = 0; i < argc; i++) {
        if(i > 1 && fputwc(L' ', stdout) < 0) {
        return 1;
        }
        if(fputws(argv[i], stdout) < 0) {
            return 1;
        }
    }

    return fputwc(L'\n', stdout) == WEOF ? 1 : 0;
}

//テストコード　実環境ではコメントアウトまたは削除します

int main(void)
{
    const wchar_t *args[] = {L"echo", L"hello", L"world"};

    return echo(3, args);
}