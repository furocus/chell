# A：制御・外部実行の解答・解説

[全体説明](../README.md)。各日の概念説明は読解30分の中で読みます。完成コードは補完した後に開き、実践90分の中で照合してください。実行結果・コンパイラの赤線による採点は行いません。

解答は一例です。式や変数名が違っても、出力先・成否・所有権・終端・失敗時の後片付けが一致すれば構いません。各ファイルはその日だけで読める見本で、前日のソースや外部の補助ファイルへ依存しません。

## Day 1：固定文を借用して返す

[課題コード](day1.c)

### 使用意図

selectedというポインタ変数の値を変更するため、&selectedを渡します。out_noteはその変数の場所、*out_noteは呼び出し元のselectedです。文字列は表が持つ固定文を借りるだけなので、呼び出し元は解放しません。

別の書き方としてポインタを戻り値にすることも可能。今回は成否を戻り値、データを出力引数に分ける設計である。

### 補完後に一致する状態

| 条件 | 正しい状態 |
|---|---|
| run | 戻り値1、selectedは「launch external program」の場所 |
| missing／0件 | 戻り値0、selectedはNULL |
| NULLの出力先 | 戻り値0、間接参照しない |

<details>
<summary>補完後に開く：完成した1関数</summary>

```c
int find_process_note(const NoteEntry *entries, size_t count,
                   const char *key, const char **out_note)
{
    if (out_note == NULL) return 0;
    *out_note = NULL;
    if (key == NULL || (entries == NULL && count != 0)) return 0;
    for (size_t i = 0; i < count; ++i) {
        if (strcmp(entries[i].key, key) == 0) {
            *out_note = entries[i].note;
            return 1;
        }
    }
    return 0;
}
```

</details>

### 照合と説明

- 呼び出し元のどの変数／データが変わるか、コードの代入先を指して説明する。
- out_note = entries[i].noteではなく、*out_noteへ代入するのはなぜか。
- 正常系だけでなく、上表の失敗条件を行単位で手で追う。
- 借用したものを解放していないか、所有したものの解放担当が決まっているか確認する。

### 詰まったとき

「どの変数の中身を変えるか」を見る。out_noteは関数内の引数変数、*out_noteは呼び出し元の変数。

TODOの補完後はこの完成関数と行ごとに照合し、不一致の理由を一つ説明します。別のファイルにレポートやテストは作らず、課題Cの最後のメモへ短く残せば十分です。

## Day 2：所有コピーを呼び出し元へ渡す

[課題コード](day2.c)

### 使用意図

出力は文字の書き換えではなく、保存用ポインタの受け渡しです。成功した領域の所有者は呼び出し元へ移ります。元配列とは別の領域なので、元入力の変更に引きずられません。strlenに終端1バイトを足し、本文と終端を合わせてコピーします。

この出力引数は、生きた所有領域を保持していない変数を受け取る契約。既存の所有ポインタを上書きすると以前の領域を失う。

### 補完後に一致する状態

| 条件 | 正しい状態 |
|---|---|
| 通常入力 | 成功1、元入力とは独立した所有コピー |
| 空文字列 | 終端1バイトを持つコピー |
| 確保失敗／NULL入力 | 失敗0、出力NULL |

<details>
<summary>補完後に開く：完成した1関数</summary>

```c
int copy_launch_line(const char *source, char **out_owned)
{
    if (out_owned == NULL) return 0;
    *out_owned = NULL;
    if (source == NULL) return 0;
    size_t bytes = strlen(source) + 1;
    char *copy = malloc(bytes);
    if (copy == NULL) return 0;
    memcpy(copy, source, bytes);
    *out_owned = copy;
    return 1;
}
```

</details>

### 照合と説明

- 呼び出し元のどの変数／データが変わるか、コードの代入先を指して説明する。
- char *out_ownedへmallocの結果を代入するだけでは、呼び出し元のsavedが変わらないのはなぜか。
- 正常系だけでなく、上表の失敗条件を行単位で手で追う。
- 借用したものを解放していないか、所有したものの解放担当が決まっているか確認する。

### 詰まったとき

見本のintの確保を、文字列の長さ＋終端の確保に読み替える。自動変数の配列へのポインタは返さない。

TODOの補完後はこの完成関数と行ごとに照合し、不一致の理由を一つ説明します。別のファイルにレポートやテストは作らず、課題Cの最後のメモへ短く残せば十分です。

## Day 3：解放とポインタのリセットを分ける

[課題コード](day3.c)

### 使用意図

ownedは呼び出し元のポインタ変数の場所です。解放対象は*ownedが指す確保領域であり、ownedそのものではありません。freeはポインタ変数へNULLを代入しないため、別に*out相当の代入が必要です。

同じ領域への別名aliasは自動でNULLにならない。別名からの参照も禁止になる。固定文やargvの借用へこの関数を使わない。

### 補完後に一致する状態

| 条件 | 正しい状態 |
|---|---|
| 確保成功後に2回呼ぶ | 実領域は1回解放、savedはNULL |
| 最初からsavedがNULL | 何も解放せずNULLのまま |
| 引数自体がNULL | 即return |

<details>
<summary>補完後に開く：完成した1関数</summary>

```c
void release_launch_line(char **owned)
{
    if (owned == NULL) return;
    free(*owned);
    *owned = NULL;
}
```

</details>

### 照合と説明

- 呼び出し元のどの変数／データが変わるか、コードの代入先を指して説明する。
- free(owned)とfree(*owned)では、対象の場所がどう違うか。owned = NULLでは誰の変数だけが変わるか。
- 正常系だけでなく、上表の失敗条件を行単位で手で追う。
- 借用したものを解放していないか、所有したものの解放担当が決まっているか確認する。

### 詰まったとき

Day 2で所有権を受け取った変数そのものの場所を渡す。領域を解放する操作と、変数へNULLを入れる操作を1行ずつ書く。

TODOの補完後はこの完成関数と行ごとに照合し、不一致の理由を一つ説明します。別のファイルにレポートやテストは作らず、課題Cの最後のメモへ短く残せば十分です。

## Day 4：ポインタの表を読む

[課題コード](day4.c)

### 使用意図

引数のargv[]は関数の引数ではポインタの表を受ける形へ調整されます。各要素は別々の長さの文字列の場所です。一方out_argumentは、呼び出し元の1本のポインタ変数を書き換える場所です。前者は表を読む用途、後者は出力の用途です。

const char *const argv[]は、文字列も表の要素もこの関数から変更しない契約。二重ポインタは常に出力引数、常に二次元配列、とは限らない。

### 補完後に一致する状態

| 条件 | 正しい状態 |
|---|---|
| index=1 | 成功1、"--label"の場所を借用 |
| index=3／count=0 | 失敗0、selectedはNULL |
| 空文字列の要素 | 有効な1要素。NULLとは異なる |

<details>
<summary>補完後に開く：完成した1関数</summary>

```c
int select_process_argument(const char *const argv[], size_t count,
                   size_t index, const char **out_argument)
{
    if (out_argument == NULL) return 0;
    *out_argument = NULL;
    if (argv == NULL || index >= count) return 0;
    *out_argument = argv[index];
    return 1;
}
```

</details>

### 照合と説明

- 呼び出し元のどの変数／データが変わるか、コードの代入先を指して説明する。
- argvの二重ポインタ相当の型と、out_argumentの二重ポインタは、同じ用途か。char text[3][16]と同じ配置か。
- 正常系だけでなく、上表の失敗条件を行単位で手で追う。
- 借用したものを解放していないか、所有したものの解放担当が決まっているか確認する。

### 詰まったとき

argv[index]はポインタ、argv[index][0]は文字。コピー、解放、文字列の正規化は不要。

TODOの補完後はこの完成関数と行ごとに照合し、不一致の理由を一つ説明します。別のファイルにレポートやテストは作らず、課題Cの最後のメモへ短く残せば十分です。

## Day 5：Win32の可変コマンドラインと出力先

[課題コード](day5.c)

### 使用意図

out_lineは呼び出し元のポインタ変数を差し替えます。一方、out_processは既に用意された構造体のフィールドを埋めます。どちらも出力ですが、変更する対象の型が違うため段数が違います。

lpApplicationNameへ解決済みexeパスを明示する本体方針は維持するが、PATH探索・起動・待機・中断の実装は今日の範囲外。

### 補完後に一致する状態

| 条件 | 正しい状態 |
|---|---|
| 通常入力 | 独立した可変UTF-16コピー。呼び出し元がfree |
| 確保失敗 | 失敗0、lineはNULL |
| PROCESS_INFORMATION | 構造体の中身の更新には*で足りる |

<details>
<summary>補完後に開く：完成した1関数</summary>

```c
int prepare_win32_line(const wchar_t *raw, wchar_t **out_line)
{
    if (out_line == NULL) return 0;
    *out_line = NULL;
    if (raw == NULL) return 0;
    size_t bytes = (wcslen(raw) + 1) * sizeof(wchar_t);
    wchar_t *copy = malloc(bytes);
    if (copy == NULL) return 0;
    memcpy(copy, raw, bytes);
    *out_line = copy;
    return 1;
}
```

</details>

### 照合と説明

- 呼び出し元のどの変数／データが変わるか、コードの代入先を指して説明する。
- out_lineは**なのに、out_processは*でよいのはなぜか。
- 正常系だけでなく、上表の失敗条件を行単位で手で追う。
- 借用したものを解放していないか、所有したものの解放担当が決まっているか確認する。

### 詰まったとき

Day 2のchar版を、UTF-16の要素数からバイト数への換算へ読み替える。

TODOの補完後はこの完成関数と行ごとに照合し、不一致の理由を一つ説明します。別のファイルにレポートやテストは作らず、課題Cの最後のメモへ短く残せば十分です。

## 5日間を通じた判断

二重ポインタは、ポインタ変数を書き換える出力先にも、ポインタの表を読む場面にも現れます。単ポインタで文字や整数・構造体の中身を更新できるなら、段数を増やす必要はありません。型だけで所有権は決まらないため、呼び出し側と受け取る側の契約で判断します。
