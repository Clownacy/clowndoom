# clowndoom

Yet another purist DOOM port.

This project aims to repair the Linux Doom source code, restoring features that
were lost from the DOS version. Additionally, minor quality-of-life
improvements have been made, such as the addition of a `novert` option.

## Features
- Music support (through WildMIDI).
  - Make sure to specify the location of your GUS patches in the 'clowndoomrc'/
    'default.cfg' configuration file.
- Proper sound effect support (the original source release lacked features like
  stopping mid-playback and updating positional effects).
- New configuration options:
  - `novert` - Prevents the player character from walking when the mouse is
    moved.
  - `always_run` - Makes the player character run instead of walk, and vice
    versa.
  - `aspect_ratio_correction` - Restores the original 4:3 aspect ratio by
    making pixels rectangular.
  - `full_colour` - Render with more than 256 colours.
  - `prototype_light_amplification_visor_effect` - Restore the 'night vision'
    effect for the light amplification visor from the Press Release Pre-Beta.
  - `resolution_scale` - Multiplies the rendering resolution. 1 is 320x200,
    2 is 640x400, etc. The maximum value is 8.
  - `hud_scale` - Multiplies the size of the user interface. 1 renders elements
    at their native resolutions, 2 renders them at double their resolution,
    etc. The maximum value is 8.
- The 'iddt' cheat enables kill/item/secret totals in the automap, as well as
  notifications when finding secrets.
- Compiler warnings have been addressed.
- Support for 64-bit CPUs.
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
- Better than CuckyDOOM.

## Configuration
Additional settings are found in the configuration file - 'clowndoomrc'. On
Unix platforms, it can be found in the user's standard configuration directory
(`XDG_CONFIG_HOME`, or `~/.config/` if it is undefined) named 'clowndoomrc'. On
other platforms, it can be found in the same directory as the executable (or
whichever directory the executable was invoked from) with the name
'default.cfg'.

WildMIDI requires a collection of GUS patches in order to work. The patches
should come with a Timidity-compatible '.cfg'. file. Set the
`wildmidi_config_path` option in 'clowndoomrc' to the path of this file, and
music should now work.
