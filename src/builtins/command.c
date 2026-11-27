#include <stdio.h>
#include <windows.h>
#include <wchar.h>
#include <locale.h>
#include "check.h"
#include "pwd.h"
#include "cd.h"

//int main(void)   //テスト用
int Command(void)
{
    _wsetlocale(LC_ALL, L""); //日本語文字列、ワイド文字用

    wchar_t command[512] = { 0 };
    wchar_t cmd1[256] = { 0 };
    wchar_t cmd2[256] = { 0 };
    int count;

    if (!CommandCheck(command, _countof(command))) { //関数呼び出し
        wprintf(L"入力エラーです。\n");
        return 1;
    }

    count = swscanf_s(
        command,
        L"%255s %255s", //入力された文字列を空白で区切って格納する
        cmd1,
        (unsigned)_countof(cmd1), //空白前格納場所
        cmd2,
        (unsigned)_countof(cmd2)); //空白後格納場所

    if (count >= 1 &&
        (wcscmp(cmd1, L"pwd") == 0 ||
            wcscmp(cmd1, L"PWD") == 0)) { //入力値がpwdまたはPWDの時
        if (count > 1) {
            wprintf(L"引数エラー");
            return 1;
        }
        return pwd();
    }

    if (count >= 1 &&
        (wcscmp(cmd1, L"cd") == 0 ||
        wcscmp(cmd1, L"CD") == 0)) {
      
        return cd(count == 1 ? L"" : cmd2);

    }

    else { //処理できないコマンドが入力されたとき
        wprintf(L"無効なコマンドです");
        return 1;
    }

    return 0;
}
