#include <stdio.h>
#include <windows.h>
#include <wchar.h>
#include <locale.h>

int main(void)
{
	_wsetlocale(LC_ALL, L"");

	wchar_t path[MAX_PATH] = { 0 };
	wchar_t command[100] = { 0 };


	if (fgetws(command, _countof(command), stdin) != NULL) {   //1行、空白含む読み込み
		size_t len = wcslen(command);   //文字数チェック
		if (len > 0 && command[len - 1] == L'\n') {   //-1検索除け、末端文字確認
			command[len - 1] = L'\0';   //末端が\0なら削除
		}
	}
    
	if (wcscmp(command, L"pwd") == 0 || wcscmp(command, L"PWD") == 0) {   //入力値がpwdまたはPWDの時
			if (GetCurrentDirectoryW(MAX_PATH, path) != 0)   //現在のディレクトリ取得
		{
			wprintf(L"%ls\n", path);
		}
			else {   //pwdを認識後取得できなかった場合
			wprintf("現在の位置を取得できませんでした");
			}
	}
	else {   //pwd以外の値が入力されたとき
		wprintf("無効なコマンドです");
	}

	return 0;
}