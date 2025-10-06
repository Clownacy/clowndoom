#include "../ib_sound.h"

#include "../ib_system/libretro-callbacks.h"

int IB_StartupSound(IB_InitialCallback initial_callback, IB_AudioCallback audio_callback, void *user_data)
{
	initial_callback(LIBRETRO_SAMPLE_RATE, user_data);

	libretro.generate_audio.callback = audio_callback;
	libretro.generate_audio.user_data = user_data;

	return 1;
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
