  ///////////////
 // clowndoom //
///////////////

Yet another purist DOOM port.

This project aims to repair the Linux Doom source code, restoring
features that were lost from the DOS version. Additionally, minor
quality-of-life improvements have been made, such as a `novert` option.

Features:
* Music support (through WildMIDI)
* Proper sound effect support (the original source release lacked
  features like stopping mid-playback and updating positional effects)
* `novert` and `always_run` options have been added to the configuration
  file
* Compiler warnings have been addressed
* 64-bit support (warning: save files are incompatible with 32-bit)
* X11 code converted from 8bpp to 24bpp (supported by modern X11
  servers)
* OSS audio code converted to miniaudio middleware library (supports
  OSS, ALSA, PulseAudio, JACK, and more)
* Assorted bug fixes
* Low-detail mode has been restored to full working order
* Mouse-grab is no longer optional, and is instead toggled by the menu
  being open
* Better than CuckyDOOM
