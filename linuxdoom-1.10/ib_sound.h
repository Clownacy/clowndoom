#ifndef __IB_SOUND__
#define __IB_SOUND__

#include <stddef.h>

int IB_StartupSound(void (*initial_callback)(unsigned int output_sample_rate, void *user_data), void (*audio_callback)(short* output_buffer, size_t frames_to_do, void *user_data), void *user_data);
void IB_ShutdownSound(void);
void IB_LockSound(void);
void IB_UnlockSound(void);

#endif
