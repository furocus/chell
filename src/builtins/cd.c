#include <stdio.h>
#include <windows.h>
#include <wchar.h>
#include "cd.h"

int cd(const wchar_t* path)
{
    wchar_t currentpath[MAX_PATH] = { 0 };
    
    if (path[0] == L'\0') {

        if (GetCurrentDirectoryW(MAX_PATH, currentpath) != 0) {  //現在位置取得
            wprintf(L"%ls\n", currentpath);
            return 0;
        }
        else {
            wprintf(L"現在の位置を取得できませんでした。\n");
            return 1;
        }
    }

    // 引数がある場合
    if (SetCurrentDirectoryW(path)) {  //ディレクトリ変更
        wprintf(L"ディレクトリを変更しました。\n");
        return 0;
    }
    //検索に失敗した場合
    else {
        wprintf(L"指定されたディレクトリが見つかりません。\n");
        return 1;
    }
}