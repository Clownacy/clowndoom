#include "../ib_sound.h"

int IB_StartupSound(void (*initial_callback)(unsigned int output_sample_rate, void *user_data), void (*_audio_callback)(short* output_buffer, size_t frames_to_do, void *user_data), void *user_data)
{
	(void)initial_callback;
	(void)_audio_callback;
	(void)user_data;
	return 0;
}

void IB_ShutdownSound(void)
{
	
}

void IB_LockSound(void)
{
	
}

void IB_UnlockSound(void)
{
	
}
