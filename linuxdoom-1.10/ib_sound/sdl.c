#include "../ib_sound.h"

#include <stddef.h>

#include "SDL.h"

/* The function that actually produces the output audio */
static void (*audio_callback)(short* output_buffer, size_t frames_to_do, void *user_data);

#if SDL_MAJOR_VERSION >= 2
static SDL_AudioDeviceID audio_device;
#endif

static void Callback(void *user_data, Uint8 *output_buffer, int bytes_to_do)
{
    const int frames_to_do = bytes_to_do / 2 / sizeof(short);

    audio_callback((short*)output_buffer, (size_t)frames_to_do, user_data);
}

int IB_StartupSound(void (*initial_callback)(unsigned int output_sample_rate, void *user_data), void (*_audio_callback)(short* output_buffer, size_t frames_to_do, void *user_data), void *user_data)
{
    SDL_AudioSpec desired_audio_specification;
#if SDL_MAJOR_VERSION >= 2
    SDL_AudioSpec obtained_audio_specification;
#endif

    audio_callback = _audio_callback;

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
    {
        /* TODO: Error message. */
    }
    else
    {
        desired_audio_specification.freq = 48000;
        desired_audio_specification.format = AUDIO_S16;
        desired_audio_specification.channels = 2;
        desired_audio_specification.samples = 0x200; /* About 10ms at 48000Hz. */
        desired_audio_specification.callback = Callback;
        desired_audio_specification.userdata = user_data;

#if SDL_MAJOR_VERSION >= 2
	/* With SDL2, we can use the native sample rate and buffer size. */
        audio_device = SDL_OpenAudioDevice(NULL, 0, &desired_audio_specification, &obtained_audio_specification, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE);

        if (audio_device == 0)
#else
        if (SDL_OpenAudio(&desired_audio_specification, NULL) < 0)
#endif
        {
            /* TODO: Error message. */
        }
        else
        {
            initial_callback(desired_audio_specification.freq, user_data);

#if SDL_MAJOR_VERSION >= 2
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
#if SDL_MAJOR_VERSION >= 2
    SDL_CloseAudioDevice(audio_device);
#else
    SDL_CloseAudio();
#endif
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void IB_LockSound(void)
{
#if SDL_MAJOR_VERSION >= 2
    SDL_LockAudioDevice(audio_device);
#else
    SDL_LockAudio();
#endif
}

void IB_UnlockSound(void)
{
#if SDL_MAJOR_VERSION >= 2
    SDL_UnlockAudioDevice(audio_device);
#else
    SDL_UnlockAudio();
#endif
}
