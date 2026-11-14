# Rogue Clone III JEC 004 — Windows（日本語）

Rogue Clone IIIは、古典的なRPG「Rogue」をもとに作られたクローン系統の最終版です。オリジナル系統のRogue 5.4と比べると、暗い部屋が存在せず、巻き物や水薬が使用後に自動的に識別されるなど、仕組みがシンプルでテンポよく遊べるのが特徴です。プレイヤーは文字で描かれる迷宮で怪物と戦い、さまざまな道具を使いながら「イェンダーの魔除け」を探します。

このJEC版は、日本語・英語の切り替えと現代のOSでの動作に対応しています。

ゲームの説明や遊び方については、[「【運命の洞窟】への案内書」](https://leopard-gecko.github.io/jrogue/guide.html?version=clone3)をご覧ください。本READMEではWindowsでのビルド・起動・設定方法を説明します。macOS / Linux版は [`README_mac_linux_ja.md`](README_mac_linux_ja.md)、英語版は [`README_windows_en.md`](README_windows_en.md) を参照してください。

## 1. 開発環境の準備

Windows版のビルドには [MSYS2](https://www.msys2.org/) の **UCRT64** を使います。インストール後、スタートメニューから「MSYS2 UCRT64」を開きます。

```sh
pacman -Syu
```

必要に応じてUCRT64を開き直し、続けて開発ツールとPDCursesModをインストールしてください。

```sh
pacman -S --needed make mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-binutils mingw-w64-ucrt-x86_64-pdcurses
```

ソースを展開し、UCRT64でそのディレクトリを開いてください。

## 2. ビルド

```sh
make clean
make
```

Windowsでは `Makefile.win` が自動選択され、`rogue-clone.exe` が生成されます。`configure` や `make install` は不要です。

| 目的 | コマンド |
|---|---|
| 実行ファイルを `rogue3.exe` という名前にする | `make PROGRAM=rogue3`（または `make PROG=rogue3`） |
| 日本語を初期表示にする | `make japanese` |
| 英語を初期表示にする | `make english` |
| オリジナルに近い設定でビルド | `make original` |
| VTバックエンドを使用する | `make clean` → `make PDC_BACKEND=vt` |

通常の **`make`** は、カラー表示などを有効にした遊びやすい初期設定です。**`make original`** は、元のClone IIIに近い操作・表示設定で、英語を初期言語とします。どちらもゲーム内容は同じで、日英の表示切り替え機能は利用できます。

| 設定 | `make` | `make original` |
|---|---|---|
| 初期言語 | OSの設定から自動選択 | 英語 |
| `jump` | off（連続移動中も位置を更新） | on（移動終了時にまとめて更新） |
| `passgo` | on（通路の角を自動で曲がる） | off |
| `color` | on | off |
| `cursor` | off | on |
| `ALLSCORES` | on（同じプレイヤーの複数スコアを記録） | off |

`jump`・`passgo`・`color`・`cursor` などはゲーム中でも変更できますが、`ALLSCORES` はビルド時の設定です。設定を変更してビルドし直す場合は `make clean` を実行してください。

通常はPDCursesModの **WinCon** バックエンドを使用します。

## 3. 起動

エクスプローラーから `rogue-clone.exe` をダブルクリックするか、PowerShellなどで起動します。

```powershell
.\rogue-clone.exe
```

ゲーム画面は80列×24行固定です。ダブルクリックで直接起動するとWindows TerminalまたはConsole Hostのウィンドウを80×24へ調整するよう試みます。すでに開いているコマンドプロンプト・PowerShell・UCRT64から起動した場合は変更しません。スコア一覧のみを見る場合は `rogue-clone.exe -s` を指定します。

ダブルクリック起動時、既定のセーブファイルが見つかった場合は、起動メニューが表示されます。`R` またはEnterで再開、`N` で新規ゲーム、`F` で別のセーブファイルを選択、`Q` で終了します。既定のセーブファイルがない場合は、そのまま新規ゲームを始めます。新規ゲームを選んでも既存のセーブファイルは削除されません。

セーブファイルを `rogue-clone.exe` にドラッグ＆ドロップすると、そのファイルを指定して再開します。日本語や空白を含むファイル名にも対応しています。PowerShellなどからファイル名を引数で渡す従来の起動方法も使用できます。


## 4. Windowsでの環境変数の設定

環境変数 **`ROGUEOPTS`** で起動時の設定を指定できます。設定項目の意味や遊び方は[「【運命の洞窟】への案内書」](https://leopard-gecko.github.io/jrogue/guide.html?version=clone3)を参照してください。ここではWindowsでの指定方法だけを説明します。

**PowerShellで一時的に設定：**

```powershell
$env:ROGUEOPTS = 'name=勇者'
.\rogue-clone.exe
```

**コマンドプロンプトで一時的に設定：**

```cmd
set "ROGUEOPTS=name=Hero"
rogue-clone.exe
```

どちらもそのシェルから起動したゲームにのみ適用されます。

**ダブルクリック起動にも適用する場合：** `Win + R` → `sysdm.cpl` → ［詳細設定］→［環境変数］を開き、［ユーザー環境変数］で `ROGUEOPTS` を新規作成または編集します。変更後はゲームと既存の端末を開き直してください。反映されない場合はWindowsにサインインし直します。設定を解除する際は同じ画面で変数を削除します。

## 5. セーブ・スコアなどの保存先

標準では `rogue-clone.save`、`rogue-clone.scores`、`rogue-clone.lock` がゲームの作業ディレクトリに作られます。**配布用のEXEは `FILEDIR` を指定せずにビルド**し、ビルドしたPC固有の保存先を埋め込まないようにしてください。

個人用にWindowsの「保存したゲーム」フォルダを使う場合：

```sh
make clean
make FILEDIR=saved-games
```

この指定では、ビルドしたユーザーの「保存したゲーム」の下に `Rogue Clone III` フォルダを作成し、その実際のパスをEXEに埋め込みます。**他のPCに配布するEXEには使用しないでください。**

任意のディレクトリを使う場合：

```sh
make clean
make FILEDIR='C:/Games/Rogue Clone III'
```

通常の `FILEDIR` 指定ではフォルダは自動作成されません。事前に作成し、書き込み権限を確認してください。`SAVEFILE=FILE`・`SCOREFILE=FILE`・`LOCKFILE=FILE` で個々のファイル名を指定でき、`FILEDIR` より優先されます。

## 6. ファイルと配布

| ファイル | 用途 |
|---|---|
| `rogue-clone` / `rogue-clone.exe` | ビルド後のゲーム本体（OSにより異なる） |
| `rogue-clone.save` / `.scores` / `.lock` | 既定のゲームデータファイル |
| `configure` / `Makefile` / `Makefile.win` | ビルド・インストールの設定 |
| `*.c` / `*.h` | ソースコード |
| `README_mac_linux_ja.md` / `README_mac_linux_en.md` | macOS / Linux用の日本語・英語説明書 |
| `README_windows_ja.md` / `README_windows_en.md` | Windows用の日本語・英語説明書 |

Windows版のアイコンには `windows/rogue-clone-iii.ico` と `windows/rogue-icon.rc` を使用します。`windows/windows-saved-games.ps1` は「保存したゲーム」フォルダを取得するビルド用スクリプトです。EXEを別のPCに配布するときは、必要な実行時DLLも確認してください。

## 7. オリジナルのRogue Clone IIIからの主な変更点

- macOS・Linux・Raspberry Pi OS・Windowsへの対応
- 日本語・英語の表示とUnicode入力（UCRT64での絵文字入力を含む）への対応
- カラー表示、操作設定、Windows用透過アイコンの追加
- Windowsでのダブルクリック起動時の80列×24行への調整
- スコアと保存先の設定、描画・戦闘・アイテム・階層生成などの不具合修正

