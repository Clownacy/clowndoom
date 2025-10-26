# ClownDoom

Limit-removing purist Doom port with enhancements and an emphasis on being
lightweight and highly portable. Think 'Crispy Doom minus the bloat'.

## Features
### Repaired Downgrades
- Features that were broken by the original Linux port:
  - Music support (through WildMIDI).
  - Complete sound effect support (restored missing features such as stopping
    mid-playback and updating positional effects).
  - Low-detail mode.
- Features that were broken by the 1997 source release:
  - Animated Doom 1 intermission screens.
  - Ultimate Doom wall switch textures.
  - Volume levels and volume control.
  - Demo compatibility.
### Portability Improvements
- Addressed various compiler warnings.
- Support for 64-bit CPUs.
- Support for Windows and modern Linux:
  - The X11 code has been converted from 8bpp to 24bpp, which is supported by
    modern X11 servers.
  - The audio code has been migrated from OSS to the miniaudio middleware
    library (supports OSS, ALSA, PulseAudio, JACK, and more).
  - As an alternative to X11 and miniaudio, SDL1 and SDL2 backends are
    available.
  - The codebase can be compiled with MSVC.
  - Windows networking code has been added.
- Eliminated reliance on C99 and C extensions, making the codebase pure ANSI C.
- Avoided reliance on implementation-defined order of `P_Random` calls.
- Experimental libretro port.
### Quality-of-Life Improvements
- New configuration options:
  - `novert` - Disables being able to make the player character walk by moving
    the mouse up and down.
  - `always_run` - Makes the player character run instead of walk, and vice
    versa.
  - `aspect_ratio_correction` - Restores the original 4:3 aspect ratio by
    making pixels rectangular.
  - `full_colour` - Render with more than 256 colours.
  - `prototype_light_amplification_visor_effect` - Restore the 'night vision'
    effect for the light amplification visor from the Press Release Pre-Beta.
  - `screen_width` - The screen's horizontal resolution. The maximum is 5040.
  - `screen_height` - The screen's vertical resolution. The maximum is 1800.
  - `hud_scale` - Multiplies the size of the user interface. 1 renders elements
    at their native resolutions, 2 renders them at double their resolution,
    etc. The maximum value is 9.
- Command line parameters:
  - `-iwad`, for loading a specific primary WAD file.
  - `-complevel`, supporting options 2, 3, and 4.
- Support for Doom 2's 'No Rest for the Living' expansion.
- The 'iddt' cheat enables kill/item/secret totals in the automap, as well as
  notifications when finding secrets.
- Assorted bug fixes.
- Better than CuckyDOOM.

## Configuration
Additional settings are found in the configuration file. In POSIX builds, this
file can be found in the user's standard configuration directory
(`XDG_CONFIG_HOME`, or `~/.config/` if it is undefined), named `clowndoomrc`.
In SDL builds, it can be found in the ['pref dir'](https://wiki.libsdl.org/SDL2/SDL_GetPrefPath),
at `clownacy/clowndoom/default.cfg`.

WildMIDI requires a collection of GUS patches in order to work. The patches
should come with a Timidity-compatible '.cfg'. file. Set the
`wildmidi_config_path` option in 'clowndoomrc' to the path of this file, and
music should now work.
