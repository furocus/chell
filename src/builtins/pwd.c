#include <stdio.h>
#include <windows.h>
#include <wchar.h>
#include <locale.h>

//int main(void)   //テスト用
int pwd(void)
{
	_wsetlocale(LC_ALL, L"");   //日本語文字列、ワイド文字用

	wchar_t path[MAX_PATH] = { 0 };
	wchar_t command[512] = { 0 };
	wchar_t cmd1[256] = { 0 };
	wchar_t cmd2[256] = { 0 };
	int count;

	if (!CommandCheck(command, _countof(command))) {   //関数呼び出し
		wprintf(L"入力エラーです。\n");
		return 1;
	}
    
	   count = swscanf_s(command, L"%255s %255s",   //入力された文字列を空白で区切って格納する
		cmd1, (unsigned)_countof(cmd1),   //空白前格納場所
		cmd2, (unsigned)_countof(cmd2));   //空白後格納場所

	   if (count >= 1 &&(wcscmp(cmd1, L"pwd") == 0 ||wcscmp(cmd1, L"PWD") == 0)) {   //入力値がpwdまたはPWDの時
		if (count > 1) {
			wprintf(L"引数エラー");
			return 1;
		}
		
		if (GetCurrentDirectoryW(MAX_PATH, path) != 0)   //現在のディレクトリ取得
		{
			wprintf(L"%ls\n", path);
		}
			else {   //pwdを認識後取得できなかった場合
			wprintf(L"現在の位置を取得できませんでした");
			return 1;
		}
	}
	else {   //pwd以外の値が入力されたとき
		wprintf(L"無効なコマンドです");
		return 1;
	}

	return 0;
}


int CommandCheck(wchar_t* command, size_t size)   //コマンドチェック関数 別ファイルに分けたい
{
	if (fgetws(command, (int)size, stdin) == NULL) {   //1行、空白含む読み込み
		return 0;
	}
	size_t len = wcslen(command);   //文字数チェック

	while (len > 0 &&(command[len - 1] == L'\n' || command[len - 1] == L'\r')) {   //-1検索除け、末端文字確認
		command[--len] = L'\0';  //末端が\0なら削除
	}   
	
	return 1;
}
