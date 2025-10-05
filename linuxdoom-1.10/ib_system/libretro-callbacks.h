#ifndef CLOWNDOOM_LIBRETRO_H
#define CLOWNDOOM_LIBRETRO_H

#include "libretro.h"

typedef struct LibretroCallbacks
{
	retro_environment_t        environment;
	retro_audio_sample_t       audio;
	retro_audio_sample_batch_t audio_batch;
	retro_input_poll_t         input_poll;
	retro_input_state_t        input_state;
	retro_video_refresh_t      video;
} LibretroCallbacks;

extern LibretroCallbacks libretro;

#endif /* CLOWNDOOM_LIBRETRO_H */
