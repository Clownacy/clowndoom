#ifndef __IB_SOUND__
#define __IB_SOUND__

#include <stddef.h>

typedef void (*IB_InitialCallback)(unsigned int output_sample_rate, void *user_data);
typedef void (*IB_AudioCallback)(short* output_buffer, size_t frames_to_do, void *user_data);

int IB_StartupSound(IB_InitialCallback initial_callback, IB_AudioCallback audio_callback, void *user_data);
void IB_ShutdownSound(void);
void IB_LockSound(void);
void IB_UnlockSound(void);

#endif
