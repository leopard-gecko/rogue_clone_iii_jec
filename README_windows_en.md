# Rogue Clone III JEC 004 — Windows (English)

Rogue Clone III is the final installment of the clone series based on the classic RPG Rogue. Compared with the original Rogue 5.4 line, it has no dark rooms, and scrolls and potions are automatically identified after use. These simpler mechanics make the game straightforward and fast-paced. Explore a dungeon rendered in text, battle monsters, use a variety of items, and search for the Amulet of Yendor.

This JEC edition supports Japanese and English and runs on modern operating systems.

For the game overview and how to play, see the [「【運命の洞窟】への案内書」 (Dungeon Guide)](https://leopard-gecko.github.io/jrogue/guide.html?version=clone3). This README covers building, launching, and configuring the Windows edition. For macOS/Linux see [`README_mac_linux_en.md`](README_mac_linux_en.md); for Japanese see [`README_windows_ja.md`](README_windows_ja.md).

## 1. Requirements

Build the Windows edition in **MSYS2 UCRT64**. Install [MSYS2](https://www.msys2.org/), open **MSYS2 UCRT64** from the Start menu, and update it:

```sh
pacman -Syu
```

Reopen UCRT64 if required, then install the build tools and PDCursesMod:

```sh
pacman -S --needed make mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-binutils mingw-w64-ucrt-x86_64-pdcurses
```

Extract the source and open its directory in the UCRT64 shell.

## 2. Build

```sh
make clean
make
```

Windows automatically selects `Makefile.win` and creates `rogue-clone.exe`. Neither `configure` nor `make install` is needed.

| Purpose | Command |
|---|---|
| Name the executable `rogue3.exe` | `make PROGRAM=rogue3` (or `make PROG=rogue3`) |
| Start in Japanese | `make japanese` |
| Start in English | `make english` |
| Use defaults closer to the original | `make original` |

The regular **`make`** build enables convenient defaults such as color. **`make original`** uses display and control defaults closer to the original Clone III and starts in English. Both builds retain the same game and bilingual display support.

| Setting | `make` | `make original` |
|---|---|---|
| Startup language | Automatic (system locale) | English |
| `jump` | off (update position while running) | on (update at the end of a run) |
| `passgo` | on (follow corridor bends) | off |
| `color` | on | off |
| `cursor` | off | on |
| `ALLSCORES` | on (keep multiple scores per player) | off |

`jump`, `passgo`, `color`, and `cursor` can be changed during play. `ALLSCORES` is a build-time setting. Run `make clean` before rebuilding with different defaults.


## 3. Launch

Double-click `rogue-clone.exe` in Explorer, or start it from PowerShell or another shell:

```powershell
.\rogue-clone.exe
```

The game display is fixed at 80 columns × 24 rows. A direct double-click launch attempts to resize Windows Terminal or Console Host to 80×24; launching from an existing Command Prompt, PowerShell, or UCRT64 shell leaves that window alone. To show scores only, use `rogue-clone.exe -s`.

When launched by double-clicking, if the default save file exists, a menu offers `R` or Enter to resume, `N` to start a new game, `F` to choose a different save file, and `Q` to quit. If there is no default save, the game starts normally. Starting a new game does not delete the existing save.

You can also drag a save file onto `rogue-clone.exe` to resume that specific file. Unicode file names (including Japanese text and spaces) are supported. Passing a save filename as an argument from PowerShell or another shell continues to work as before.

## 4. Setting Windows environment variables

Use the **`ROGUEOPTS`** environment variable for startup options. The [「【運命の洞窟】への案内書」 (Dungeon Guide)](https://leopard-gecko.github.io/jrogue/guide.html?version=clone3) explains the settings themselves; this section covers only how to set the variable on Windows.

**Temporary setting in PowerShell:**

```powershell
$env:ROGUEOPTS = 'name=Hero'
.\rogue-clone.exe
```

**Temporary setting in Command Prompt:**

```cmd
set "ROGUEOPTS=name=Hero"
rogue-clone.exe
```

These values apply only to programs launched from that shell.

**For double-click launches:** Press `Win + R`, run `sysdm.cpl`, and open **Advanced → Environment Variables**. Under **User variables**, create or edit `ROGUEOPTS`. Restart the game and any open shells. If the value is not picked up, sign out of Windows and sign in again. Delete the variable in the same dialog to remove the setting.

## 5. Save and score file locations

By default, `rogue-clone.save` and `rogue-clone.scores` are written to the game's working directory. **For a redistributable EXE, leave `FILEDIR` unset** so that the build computer's private path is not embedded.


To use the current user's Windows Saved Games folder for a personal build:

```sh
make clean
make FILEDIR=saved-games
```

This creates a `Rogue Clone III` folder under the build user's actual Saved Games location and embeds that path in the EXE. **Do not use this build option for an EXE distributed to another computer.**

To use another directory:

```sh
make clean
make FILEDIR='C:/Games/Rogue Clone III'
```

A normal `FILEDIR` value does not create its directory. Create it first and ensure write permission. Use `SAVEFILE=FILE` or `SCOREFILE=FILE` to set individual filenames; these take precedence over `FILEDIR`.

## 6. Files and distribution

| File | Purpose |
|---|---|
| `rogue-clone` / `rogue-clone.exe` | Built executable (depending on OS) |
| `rogue-clone.save` / `.scores` | Default game data files |
| `configure` / `Makefile` / `Makefile.win` | Build and installation configuration |
| `*.c` / `*.h` | Source files |
| `README_mac_linux_ja.md` / `README_mac_linux_en.md` | macOS/Linux guides (Japanese/English) |
| `README_windows_ja.md` / `README_windows_en.md` | Windows guides (Japanese/English) |

The Windows icon uses `windows/rogue-clone-iii.ico` and `windows/rogue-icon.rc`. `windows/windows-saved-games.ps1` locates the Saved Games folder at build time. When distributing the EXE, check for any required runtime DLLs.

## 7. Main changes from the original Rogue Clone III

- Support for macOS, Linux, Raspberry Pi OS, and Windows
- Japanese/English display and Unicode input, including emoji in UCRT64
- Color, control options, and a transparent Windows application icon
- An 80×24 terminal resize attempt for direct Windows double-click launches
- Configurable score and data file locations, plus fixes to drawing, combat, items, and dungeon generation

