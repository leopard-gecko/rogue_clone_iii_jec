# Rogue Clone III JEC

**Rogue Clone III JEC 005**

Rogueは、1980年頃に誕生した、文字や記号で描かれた迷宮を探索するコンピューターRPGです。プレイヤーは怪物と戦い、さまざまな道具を使いながら、迷宮の奥に隠された「イェンダーの魔除け」を探します。Rogue Clone IIIは、Rogueをもとに独自に発展したクローン版の最終バージョンです。Rogue 5.4と比べると、暗い部屋がなく、巻き物や水薬が使用後に基本的に自動識別されるなどの違いがあります。

**Rogue Clone III JEC** は、元のRogue Clone IIIのゲーム内容はそのままに、日本語・英語の切り替え、カラー表示、現代のOSへの対応、不具合修正などを加えた修正版です。

*Rogue Clone III JEC is a modified edition of Rogue Clone III. It retains the original game's fundamental gameplay while adding Japanese/English language switching, color display, support for modern operating systems, and bug fixes.*

## 対応環境とREADME / Platforms and documentation

macOS、Linux（Raspberry Pi OSを含む）、Windows 10 / 11に対応しています。ビルド方法、起動方法、オプションやセーブファイルの扱いについては、OS・言語別のREADMEをご覧ください。

*For build instructions, launching, options, and save-file locations, choose the README for your operating system and language.*

| 対象OS / Platform | 日本語 | English |
| --- | --- | --- |
| macOS / Linux / Raspberry Pi OS | [README（日本語）](README_mac_linux_ja.md) | [README (English)](README_mac_linux_en.md) |
| Windows 10 / 11 | [README（日本語）](README_windows_ja.md) | [README (English)](README_windows_en.md) |

macOS / Linuxでは通常 `make`、WindowsではMSYS2 UCRT64環境からビルドできます。必要な準備や詳しい手順は各READMEを参照してください。

*On macOS/Linux, the standard build uses `make`. Windows builds use MSYS2 UCRT64; see the platform-specific README for prerequisites and details.*

## JEC版の主な変更点 / Main changes in JEC

- 日本語・英語の表示切り替えとUnicode文字の入力・表示 / Japanese/English language switching and Unicode input/display
- カラー表示と、通常版・オリジナル寄りの初期設定 / Color display and modern/original-style defaults
- macOS、Linux、Raspberry Pi OS、Windowsへの対応 / Support for modern operating systems
- Windowsでのダブルクリック起動、セーブファイルからの再開、画面サイズ調整 / Windows double-click launching, save-file resume, and terminal-size adjustment
- 描画、操作、戦闘、アイテム、階層生成などの不具合修正 / Fixes for display, controls, combat, items, and dungeon generation

## 【運命の洞窟】への案内書 / A Guide to the Dungeons of Doom

ゲームの操作、画面表示、アイテム、オプションなどについては、[公開サイトのClone III用案内書](https://leopard-gecko.github.io/jrogue/guide.html?version=clone3)をご覧ください。

*For game commands, screen layout, items, and options, see the [online guide for Rogue Clone III](https://leopard-gecko.github.io/jrogue/guide.html?version=clone3).*

## 著作権・ライセンス / Copyright and license

元のRogue Clone IIIの利用・再配布条件と、JEC版での変更・追加機能およびアイコンの扱いについては、[LICENSE.txt](LICENSE.txt)をご覧ください。

*See [LICENSE.txt](LICENSE.txt) for the original Rogue Clone III license notices and the terms covering JEC modifications, additions, and icon artwork.*
