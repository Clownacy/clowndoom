clowndoom

Better than CuckyDOOM.

This is a modified version of Linux Doom that runs on modern Linux.
Namely, its Xorg drawing code has been converted from 8bpp to 24bpp,
and the sound code has been converted from OSS to the miniaudio
intermediary library, which itself outputs to OSS, ALSA, JACK,
PulseAudio, and so on.

Additionally, bugs and compiler warnings have been addressed,
quality-of-life improvements have been made, and music support has
been restored.
