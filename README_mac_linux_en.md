# Rogue Clone III JEC 004 — macOS / Linux (English)

This document explains building, launching, installing, and file locations on **macOS, Linux, and Raspberry Pi OS**. For the game overview and JEC changes, see the [repository README](README.md); for game commands and items, see the [Dungeon Guide](https://leopard-gecko.github.io/jrogue/guide.html?version=clone3).

Japanese version: [README_mac_linux_ja.md](README_mac_linux_ja.md)

## 1. Requirements

Extract the source and open a terminal in that directory.

**macOS:** Install Apple's command-line tools:

```sh
xcode-select --install
```

**Ubuntu / Debian / Raspberry Pi OS:**

```sh
sudo apt update
sudo apt install build-essential libncurses-dev
```

**Fedora:** `sudo dnf install gcc make ncurses-devel`

**Arch Linux:** `sudo pacman -S base-devel ncurses`

## 2. Build

**macOS:**

```sh
make clean
make
```

**Linux / Raspberry Pi OS:**

```sh
./configure
make clean
make
```

This produces the `rogue-clone` executable. Building with `make` alone, without `configure`, is also supported.

| Purpose | Command |
|---|---|
| Name the executable `rogue3` | `./configure --program=rogue3` → `make clean` → `make` |
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

You can select a different curses library with `configure` options such as `--with-curses` and `--with-curses-libs` if required by your system.

## 3. Launch

```sh
./rogue-clone
```

The dungeon display remains fixed at 80 columns × 24 rows. Use a terminal at least that large. To show scores only, run `./rogue-clone -s`.

## 4. Install (optional)

You can play directly from the build directory. To install system-wide:

```sh
sudo make install
```

The default locations are `/usr/local/bin/rogue-clone` for the executable and `/usr/local/share/doc/rogue-clone/` for documentation. For a home-directory install, configure with `./configure --prefix="$HOME/.local"`, rebuild, then run `make install`. Use `make uninstall` to remove an installed copy.

## 5. Save and score file locations

By default, `rogue-clone.save` and `rogue-clone.scores` are written to the game's working directory. For a single custom data directory:

```sh
./configure --file-dir="$HOME/.local/share/rogue-clone"
make clean
make
```

The directory selected with `--file-dir` is created automatically when needed **only during `make install`**. If you run the game without installing, create it first (for example, `mkdir -p "$HOME/.local/share/rogue-clone"`). The directory must be writable.

Use `--program=NAME` to rename the executable, or `--save-file=FILE` or `--score-file=FILE` to choose filenames. The `make` variables `PROGRAM` (or `PROG`), `FILEDIR`, `SAVEFILE`, and `SCOREFILE` are also supported.

## 6. Source distribution

Create a source archive with `make dist.src`. By default it creates `rogue-clone-iii-004-src.tar.gz` in the parent directory.

For the game guide and other platform READMEs, see the [repository README](README.md).
