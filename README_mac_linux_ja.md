# Rogue Clone III JEC 004 — macOS / Linux（日本語）

この文書では、**macOS・Linux・Raspberry Pi OS**でのビルド・起動・インストール・保存先を説明します。ゲームの紹介とJEC版の変更点は[リポジトリのREADME](README.md)、操作やアイテムの説明は[「【運命の洞窟】への案内書」](https://leopard-gecko.github.io/jrogue/guide.html?version=clone3)をご覧ください。

英語版：[README_mac_linux_en.md](README_mac_linux_en.md)

## 1. 開発環境の準備

ソースを展開し、ターミナルでそのディレクトリを開いてください。

**macOS：** Appleのコマンドラインツールをインストールします。

```sh
xcode-select --install
```

**Ubuntu / Debian / Raspberry Pi OS：**

```sh
sudo apt update
sudo apt install build-essential libncurses-dev
```

**Fedora：** `sudo dnf install gcc make ncurses-devel`

**Arch Linux：** `sudo pacman -S base-devel ncurses`

## 2. ビルド

**macOS：**

```sh
make clean
make
```

**Linux / Raspberry Pi OS：**

```sh
./configure
make clean
make
```

実行ファイル `rogue-clone` が作られます。`configure` を使わず `make` のみでビルドすることも可能です。

| 目的 | コマンド |
|---|---|
| 実行ファイルを `rogue3` という名前にする | `./configure --program=rogue3` → `make clean` → `make` |
| 日本語を初期表示にする | `make japanese` |
| 英語を初期表示にする | `make english` |
| オリジナルに近い設定でビルド | `make original` |

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

環境に応じて、`configure` の `--with-curses` や `--with-curses-libs` で使用するcursesライブラリを変更できます。

## 3. 起動

```sh
./rogue-clone
```

ダンジョンの描画範囲は80列×24行で固定されています。ターミナルも80列×24行以上にしてください。スコア一覧のみを見る場合は `./rogue-clone -s` を実行します。

## 4. インストール（任意）

ビルドしたディレクトリから直接遊ぶ場合はインストール不要です。標準のインストールは次のとおりです。

```sh
sudo make install
```

既定のインストール先は `/usr/local/bin/rogue-clone`、READMEの保存先は `/usr/local/share/doc/rogue-clone/` です。ホームディレクトリにインストールする場合は、先に `./configure --prefix="$HOME/.local"` で設定し、再ビルドしてから `make install` してください。削除には `make uninstall` を使います。

## 5. セーブ・スコアなどの保存先

標準では `rogue-clone.save`、`rogue-clone.scores` がゲームの作業ディレクトリに作られます。まとめて別の保存先を指定する例：


```sh
./configure --file-dir="$HOME/.local/share/rogue-clone"
make clean
make
```

`--file-dir` で指定したディレクトリは、**`make install` 実行時にのみ**必要に応じて自動作成されます。インストールせずに実行する場合は、先に `mkdir -p "$HOME/.local/share/rogue-clone"` などで作成してください。保存先には書き込み権限が必要です。

実行ファイル名は `--program=NAME` で、個々のファイル名は `--save-file=FILE`、`--score-file=FILE` で変更できます。`make` の変数 `PROGRAM`（または `PROG`）、`FILEDIR`、`SAVEFILE`、`SCOREFILE` も使用できます。

## 6. ソース配布

ソース配布用アーカイブは `make dist.src` で作成できます。既定では親ディレクトリに `rogue-clone-iii-004-src.tar.gz` を出力します。

案内書と他のOS向けのREADMEは[リポジトリのREADME](README.md)から参照できます。
