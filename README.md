# [Gates of Skeldal](https://en.wikipedia.org/wiki/Gates_of_Skeldal)

Classic Czech dungeon crawler. Commercially released in 1998, open-sourced by the original authors in 2008.

The island of Rovenland is being consumed by darkness. Powerful wizard named Freghar searched for the source of this menace and found it. But he was unable to stop it himself. Instead, he has sacrificed his life to summon three heroes - you. Can you save the world from evil magic?

## Dependencies

- libSDL 1.2
- SDL\_mixer 1.2

## Installation

Clone this repository and build the sources:

    ./bootstrap
    ./configure
    make
    make install

Original game files are required to play the game. Copy the following files and directories to `/usr/local/share/skeldal`:
- ENDTEXT.ENC
- MAPS
- POPISY.ENC
- SKELDAL.DDL
- TITULKY.ENC
- VIDEO

In addition, download `flute.zip` and `music.zip` from [SourceForge](https://sourceforge.net/projects/skeldal/files/) and extract both archives to the above directory.

## Adventures Howto

If you want to play an external adventure, here's a short howto:
- First, unpack the adventure anywhere.
- Use demus.sh in tools directory to recode any MUS files to MP3. You'll need the Lame MP3 encoder. Then you can delete the MUS files.
- Run `skeldal path/to/adventure/config/file.adv` and have fun
- Optional: If the adventure contains additional saved games, you can copy those to `~/.skeldal/ADV/<adventure dir>/SAVEGAME/` or whatever other directory the adventure keeps its savegames in. The relative path is preserved but the basedir is moved to `~/.skeldal/`. If you start Skeldal with the adventure before copying the savegames, it will create the directory tree for you.

Note: all relative paths and filenames in the adventure have to be upper case on Unix. That includes savegame files in ~/.skeldal/.
