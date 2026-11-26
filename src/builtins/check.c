#include <stdio.h>
#include <wchar.h>
#include "check.h"

int CommandCheck(wchar_t* command, size_t size)   //コマンドチェック関数 別ファイルに分けたい
{
	if (fgetws(command, (int)size, stdin) == NULL) {   //1行、空白含む読み込み
		return 0;
	}
	size_t len = wcslen(command);   //文字数チェック

	while (len > 0 && (command[len - 1] == L'\n' || command[len - 1] == L'\r')) {   //-1検索除け、末端文字確認
		command[--len] = L'\0';  //末端が\0なら削除
	}

	return 1;
}
