#include <stdio.h>
#include <windows.h>
#include "pwd.h"

int pwd(void)
{
	wchar_t path[MAX_PATH] = { 0 };

    if (GetCurrentDirectoryW(MAX_PATH, path) != 0) //現在のディレクトリ取得
    {
        wprintf(L"%ls\n", path);
        return 0;
    }
    else { //pwdを認識後取得できなかった場合
        wprintf(L"現在の位置を取得できませんでした");
        return 1;
    }
}