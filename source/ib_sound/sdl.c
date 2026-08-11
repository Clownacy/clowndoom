#include "../ib_sound.h"

#include <stddef.h>

#ifdef USE_SDL3
	#include <SDL3/SDL.h>
#else
	#include "SDL.h"
#endif

#define DEFAULT_SAMPLE_RATE 48000

/* The function that actually produces the output audio */
static IB_AudioCallback audio_callback;

#if SDL_MAJOR_VERSION >= 3
static SDL_AudioStream *audio_stream;
#elif SDL_MAJOR_VERSION >= 2
static SDL_AudioDeviceID audio_device;
#endif

#if SDL_MAJOR_VERSION >= 3
static void Callback(void *user_data, SDL_AudioStream *stream, int bytes_to_do, int total_bytes)
#else
static void Callback(void *user_data, Uint8 *output_buffer, int bytes_to_do)
#endif
{
	const int frames_to_do = bytes_to_do / 2 / sizeof(short);

#if SDL_MAJOR_VERSION >= 3
	Uint8 *output_buffer;

	(void)total_bytes;

	output_buffer = SDL_stack_alloc(Uint8, bytes_to_do);
	if (output_buffer == NULL)
		return;
#endif

	audio_callback((short*)output_buffer, (size_t)frames_to_do, user_data);

#if SDL_MAJOR_VERSION >= 3
	SDL_PutAudioStreamData(stream, output_buffer, bytes_to_do);
	SDL_stack_free(output_buffer);
#endif
}

int IB_StartupSound(IB_InitialCallback initial_callback, IB_AudioCallback _audio_callback, void *user_data)
{
	audio_callback = _audio_callback;

#if SDL_MAJOR_VERSION >= 3
	if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
#else
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
#endif
	{
		/* TODO: Error message. */
	}
	else
	{
		SDL_AudioSpec audio_specification;

#if SDL_MAJOR_VERSION >= 3
		/* With SDL3, we can use the native sample rate. */
		if (!SDL_GetAudioDeviceFormat(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio_specification, NULL))
			audio_specification.freq = DEFAULT_SAMPLE_RATE;
		audio_specification.channels = 2;
		audio_specification.format = SDL_AUDIO_S16;

		audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio_specification, Callback, user_data);

		if (audio_stream == NULL)
#elif SDL_MAJOR_VERSION >= 2
		SDL_AudioSpec obtained_audio_specification;

		audio_specification.freq = DEFAULT_SAMPLE_RATE;
		audio_specification.channels = 2;
		audio_specification.format = AUDIO_S16;
		audio_specification.samples = 0x200; /* About 10ms at 48000Hz. */
		audio_specification.callback = Callback;
		audio_specification.userdata = user_data;

		/* With SDL2, we can use the native sample rate and buffer size. */
		audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_specification, &obtained_audio_specification, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
		audio_specification = obtained_audio_specification;

		if (audio_device == 0)
#else
		if (SDL_OpenAudio(&audio_specification, NULL) < 0)
#endif
		{
			/* TODO: Error message. */
		}
		else
		{
			initial_callback(audio_specification.freq, user_data);

#if SDL_MAJOR_VERSION >= 3
			SDL_ResumeAudioStreamDevice(audio_stream);
#elif SDL_MAJOR_VERSION >= 2
			SDL_PauseAudioDevice(audio_device, 0);
#else
			SDL_PauseAudio(0);
#endif

			return 1;
		}

		SDL_QuitSubSystem(SDL_INIT_AUDIO);
	}

	return 0;
}

void IB_ShutdownSound(void)
{
#if SDL_MAJOR_VERSION >= 3
	SDL_DestroyAudioStream(audio_stream);
#elif SDL_MAJOR_VERSION >= 2
	SDL_CloseAudioDevice(audio_device);
#else
	SDL_CloseAudio();
#endif
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void IB_LockSound(void)
{
#if SDL_MAJOR_VERSION >= 3
	SDL_LockAudioStream(audio_stream);
#elif SDL_MAJOR_VERSION >= 2
	SDL_LockAudioDevice(audio_device);
#else
	SDL_LockAudio();
#endif
}

void IB_UnlockSound(void)
{
#if SDL_MAJOR_VERSION >= 3
	SDL_UnlockAudioStream(audio_stream);
#elif SDL_MAJOR_VERSION >= 2
	SDL_UnlockAudioDevice(audio_device);
#else
	SDL_UnlockAudio();
#endif
}
