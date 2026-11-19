# 図入り閲覧ページの更新

Markdownが正本です。読むだけならNode.js、Mermaid、ネット接続、Orcaは不要です。取得した `docs/spec/index.html` をブラウザーで開きます。ページ上部で資料を切り替え、図は拡大と左右スクロールができます。SVGは `diagrams/` にも保存します。

## 更新する人の手順

1. 4資料と必要なら検証記録を編集する。
2. Orcaの作業ターミナルから、リポジトリ直下で次を実行する（Node.jsと初回描画時のネット接続が必要）。

~~~powershell
node docs/spec/tools/build-preview.cjs
node docs/spec/tools/build-preview.cjs --check
~~~

描画はOrca内蔵ブラウザーで行います。`ORCA_CLI_COMMAND` があればその実行ファイルを使います。描画だけに使うMermaid 11.12.1とMarked 15.0.12をjsDelivrから読みます。npm installや本体ビルドへの依存追加はしません。描画後のHTMLにはライブラリ・外部フォント・外部画像への読み込みを残しません。公式サイト等のリンクはクリック時に開きます。

3. 作成された `index.html` を開き、5図の日本語・矢印・表・リンクを確認する。デスクトップ幅と狭い幅で横スクロールと図の拡大を確認する。
4. Markdown、HTML、SVG、manifest.json、検証記録を同じ変更に含める。

`--check` はオフラインで、正本と生成物のSHA-256、相対リンク先、表の列数、図の数を検査します。Mermaid構文の再解析は生成時に行います。文面の意味やChell本体の正しさを自動判定するものではありません。

描画用タブと一時HTMLは確認用に残します。生成スクリプトはタブIDを表示します。作業後は自分が生成したタブを閉じて構いません。共有・公開の操作は行いません。
