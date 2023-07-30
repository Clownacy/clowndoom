# clowndoom

Yet another purist DOOM port.

This project aims to repair the Linux Doom source code, restoring features that
were lost from the DOS version. Additionally, minor quality-of-life
improvements have been made, such as the addition of a `novert` option.

## Features
- Music support (through WildMIDI).
  - Make sure to specify the location of your GUS patches in the 'clowndoomrc'
    configuration file.
- Proper sound effect support (the original source release lacked features like
  stopping mid-playback and updating positional effects).
- New configuration options:
  - `novert` - Prevents the player character from walking when the mouse is
    moved.
  - `always_run` - Makes the player character run instead of walk, and vice
    versa.
  - `aspect_ratio_correction` - Restores the original 4:3 aspect ratio by
    making pixels rectangular.
- Compiler warnings have been addressed.
- 64-bit support (warning: save files are incompatible with 32-bit).
- Support for Windows and modern Linux.
  - The X11 code has been converted from 8bpp to 24bpp, which is supported by
    modern X11 servers.
  - The audio code has been migrated from OSS to the miniaudio middleware
    library (supports OSS, ALSA, PulseAudio, JACK, and more).
  - As an alternative to X11 and miniaudio, SDL1 and SDL2 backends are
    available.
  - The codebase can be compiled with MSVC.
- Assorted bug fixes.
- Low-detail mode has been restored to full working order.
- Mouse-grab is no longer optional, and is instead toggled by the menu being
  open.
- Better than CuckyDOOM.

## Configuration
Additional settings are found in the configuration file - 'clowndoomrc'. On
Unix platforms, it can be found in the user's standard configuration directory
(`XDG_CONFIG_HOME`, or `~/.config/` if it is undefined). On other platforms, it
can be found in the same directory as the executable (or whichever directory
the executable was invoked from).

WildMIDI requires a collection of GUS patches in order to work. The patches
should come with a Timidity-compatible '.cfg'. file. Set the
`wildmidi_config_path` option in 'clowndoomrc' to the path of this file, and
music should now work.
