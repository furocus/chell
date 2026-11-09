# Contributing

このリポジトリで開発するときのルールです。

## ブランチ

作業ごとにブランチを分けます。

```bash
git switch -c feature/<issue-number>-short-title
```


`short-title` は英語の kebab-case を基本とします。

## コミットメッセージ

コミットメッセージは、変更の種類が分かる prefix を付けます。

よく使う prefix:

`feat`: 機能追加や画面追加。

`fix`: 不具合修正。

`docs`: README や説明文など、ドキュメントの変更。

`chore`: 設定、依存関係、開発環境などの変更。`chore`は雑用という意味。

例:

```text
feat: ホーム画面の土台を追加する
fix: 人数選択の表示崩れを修正する
docs: READMEにセットアップ手順を追加する
chore: TailwindとPrettierの設定を追加する
```

コミットは、できるだけ1つの目的ごとに分けます。
同じコミットに関係ない変更を混ぜないようにしてください。

## Pull Request

PR には、何を変更したか、なぜ変更したか、どう確認したかを書きます。

Issue を完了する PR では、PR の description に次のように書きます。

```text
Closes #1
```

関連だけ示したい場合は `refs #1` や `related #1` を使います。

PR を作る前に、変更したところを手元で確認してください。

## レビュー

マージ前にレビューを受けます。

レビューコメントでは、必要に応じて prefix を付けます。

`[must]` : 要修正という意味。

`[ask]` : ただの質問です、という意味。

`[nit]` : 細かい指摘で修正は不要、という意味。

`[imo]` : In My Opinionの略。私はこう思います、という意味。

`[fyi]` : For Your Informationの略。参考情報などを貼るときなどに使う。

`[good]` : 良かったところを伝える際などに使う。
