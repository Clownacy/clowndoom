#include "../ib_sound.h"

#include <stddef.h>

#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#include "miniaudio.h"

/* The function that actually produces the output audio */
static void (*audio_callback)(short* output_buffer, size_t frames_to_do, void *user_data);

/* miniaudio context */
static ma_context       context;

/* miniaudio context */
static ma_mutex         mutex;

/* The actual output device. */
static ma_device        audio_device;

static void Callback(ma_device* device, void* output_buffer, const void* input_buffer, ma_uint32 frames_to_do)
{
	(void)input_buffer;

	ma_mutex_lock(&mutex);

	audio_callback((short*)output_buffer, (size_t)frames_to_do, device->pUserData);

	ma_mutex_unlock(&mutex);
}

int IB_StartupSound(void (*initial_callback)(unsigned int output_sample_rate, void *user_data), void (*_audio_callback)(short* output_buffer, size_t frames_to_do, void *user_data), void *user_data)
{
	audio_callback = _audio_callback;

	ma_context_init(NULL, 0, NULL, &context);

	ma_mutex_init(&mutex);

	ma_device_config config = ma_device_config_init(ma_device_type_playback);
	config.playback.pDeviceID = NULL;
	config.playback.format = ma_format_s16;
	config.playback.channels = 2;
	config.sampleRate = 0; /* Let miniaudio decide what sample rate to use */
	config.dataCallback = Callback;
	config.pUserData = user_data;
	config.noPreZeroedOutputBuffer = ma_true;

	ma_device_init(&context, &config, &audio_device);

	initial_callback(audio_device.sampleRate, user_data);

	ma_device_start(&audio_device);

	return 1;
}

void IB_ShutdownSound(void)
{
	ma_device_uninit(&audio_device);
	ma_mutex_uninit(&mutex);
	ma_context_uninit(&context);
}

void IB_LockSound(void)
{
	ma_mutex_lock(&mutex);
}

void IB_UnlockSound(void)
{
	ma_mutex_unlock(&mutex);
}
