#ifndef CLOWNDOOM_LIBRETRO_H
#define CLOWNDOOM_LIBRETRO_H

#include "libretro.h"

#include "../ib_sound.h"

#define LIBRETRO_SAMPLE_RATE 48000
#define LIBRETRO_CHANNEL_COUNT 2

typedef struct LibretroCallbacks
{
	retro_environment_t        environment;
	retro_audio_sample_t       audio;
	retro_audio_sample_batch_t audio_batch;
	retro_input_poll_t         input_poll;
	retro_input_state_t        input_state;
	retro_video_refresh_t      video;
	struct
	{
		IB_AudioCallback callback;
		void *user_data;
	} generate_audio;
} LibretroCallbacks;

extern LibretroCallbacks libretro;

void IB_Yield(void);

#endif /* CLOWNDOOM_LIBRETRO_H */
