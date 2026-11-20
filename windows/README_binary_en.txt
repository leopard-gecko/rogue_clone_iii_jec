Rogue Clone III JEC 004  Windows Edition
==========================================

Rogue is a computer role-playing game that originated around 1980. You
explore a dungeon drawn with characters and symbols, fight monsters,
and use various items while searching for the Amulet of Yendor.

Rogue Clone III is the final installment of a series of Rogue clones.
Compared with the original Rogue 5.4 line, it has no dark rooms, and
scrolls and potions are automatically identified after use. These
simpler mechanics make the game straightforward and fast-paced.

This package contains Rogue Clone III JEC 004 for Windows 10 / 11
(64-bit). It preserves the basic gameplay of Clone III while adding
Japanese/English language switching, Unicode and emoji input, color
display, control options, and compatibility with modern Windows systems.

[Included Files]

  @PROGRAM@.exe             Standard edition
  @ORIGINAL@.exe    Original-settings edition
  README_ja.txt           Japanese README
  README_en.txt           This file (English README)
  LICENSE.txt             Copyright and license

Both executables contain the same game, but have different defaults.
The standard edition favors convenient play. The original-settings
edition starts in English and uses display and control defaults closer
to the original Clone III. You can change the language in either edition.

  Option                          Standard            Original settings
  ----------------------------------------------------------------------
  Startup language                System locale       English
  Color                           On                  Off
  Follow corridor bends           On                  Off
  Update during a long run        On                  Off
  Cursor                          Off                 On
  Multiple scores per player      On                  Off

You can change the color, movement, and cursor options in the game.
Multiple-score recording is a build-time setting.

[Starting the Game]

1. Extract the ZIP archive into a folder of your choice. Use a folder
   in which you can write save files.
   (Example: C:\Games\RogueCloneIII)

2. In File Explorer, double-click @PROGRAM@.exe for the standard edition
   or @ORIGINAL@.exe for the original-settings edition.

   If the default save file exists, a launch menu appears:

     [R/Enter] Resume the saved adventure
     [N]       Start a new adventure
     [F]       Choose another save file
     [Q]       Quit

   If the default save does not exist, a new game starts without the
   menu. Pressing F opens the Windows file picker; cancelling returns
   to the menu. Starting a new game does not delete the existing save.

3. Drag and drop a saved-game file onto @PROGRAM@.exe (or
   @ORIGINAL@.exe) to resume that specific file directly.

4. To launch from a terminal, open the extracted folder. On Windows 11,
   right-click and select "Open in Terminal". If that option is not
   available, type powershell in the folder's address bar and press
   Enter. Then run one of these commands:

   Standard edition:
       .\@PROGRAM@.exe

   Original-settings edition:
       .\@ORIGINAL@.exe

   You can also use Command Prompt (cmd.exe). Launching without an
   argument from an existing terminal starts a new game.

Use a terminal at least 80 columns wide and 24 rows high. The game
display is fixed at 80x24. A direct double-click launch attempts to
resize Windows Terminal or Console Host to 80x24, though the request
may not work in every display environment. The program does not resize
an existing terminal from which it is launched manually.

[Save Files and Scores]

Use the in-game save command to save your adventure. The standard and
original-settings editions share these default file names:

  Save file               @SAVEFILE@
  Score file              @SCOREFILE@

These files are stored in the working directory. When launched directly
from File Explorer, this is normally the executable's folder. Both
editions can resume from the same save file and share the high scores.
The score file itself is locked during updates; no separate lock
file is created. Do not access the same save file from multiple
game processes at once.

To resume the shared default save from a terminal, specify
the file name:

       .\@PROGRAM@.exe "@SAVEFILE@"

To resume a save file in another location:

       .\@PROGRAM@.exe "C:\Games\RogueCloneIII\my-save.save"

To display the high-score list only:

       .\@PROGRAM@.exe -s

For the original-settings edition, replace @PROGRAM@.exe with
@ORIGINAL@.exe. After a successful restore, the original save
file is normally deleted by design. Copy any save you wish to keep
before resuming it.

A save filename specified with file= in ROGUEOPTS takes precedence.

[Setting Environment Variables in Windows]

Use ROGUEOPTS to set startup options. For explanations of individual
options, see the game guide linked below. This section explains how to
set the environment variable in Windows.

Temporary PowerShell setting:

       $env:ROGUEOPTS = "name=Hero,language=en"
       .\@PROGRAM@.exe

Temporary Command Prompt (cmd.exe) setting:

       set "ROGUEOPTS=name=Hero,language=en"
       @PROGRAM@.exe

Use language=ja for Japanese or language=en for English. These settings
apply to the current terminal and programs started from it, not to
separate File Explorer double-click launches.

To apply the same settings to double-click launches, add ROGUEOPTS to
your Windows user environment variables:

  1. Press Win + R, enter sysdm.cpl, and press Enter.
  2. Open "Advanced" -> "Environment Variables".
  3. Under "User variables", choose "New" (or "Edit").
  4. Enter ROGUEOPTS as the name and your settings as the value.
  5. Save with "OK". Sign out and back in if the change is not applied.

Delete the variable in the same dialog when it is no longer needed.

[Game Guide and License]

For game controls, commands, items, and options, see the Rogue Clone III
section of the "Guide to the Dungeons of Doom":

  https://leopard-gecko.github.io/jrogue/guide.html?version=clone3

This is an online guide and requires an Internet connection. For
copyright information and redistribution terms, see the included
LICENSE.txt file.
