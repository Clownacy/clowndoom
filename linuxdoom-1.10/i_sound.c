/******************************************************************************

   Copyright (C) 1993-1996 by id Software, Inc.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   DESCRIPTION:
    System interface for sound.

******************************************************************************/

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef WILDMIDI
#include "wildmidi_lib.h"
#endif

#include "z_zone.h"

#include "i_system.h"
#include "i_sound.h"
#include "ib_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"

#include "doomdef.h"

#define NUM_CHANNELS    8

static unsigned int     output_sample_rate;


/* The actual lengths of all sound effects. */
static size_t           lengths[NUMSFX];


/* The channel step amount... */
static unsigned long    channelstep[NUM_CHANNELS];
/* ... and a 0.16 bit remainder of last step. */
static unsigned long    channelstepremainder[NUM_CHANNELS];

/* The channel data pointers, start and end. */
static unsigned char*   channels[NUM_CHANNELS];
static unsigned char*   channelsend[NUM_CHANNELS];

/* Time/gametic that the channel started playing, */
/*  used to determine oldest, which automatically */
/*  has lowest priority. */
/* In case number of active sounds exceeds */
/*  available channels. */
static int              channelstart[NUM_CHANNELS];

/* The sound in channel handles, */
/*  determined on registration. */
static int              channelhandles[NUM_CHANNELS];

/* SFX id of the playing sound effect. */
/* Used to catch duplicates (like chainsaw). */
static int              channelids[NUM_CHANNELS];

/* Hardware left and right channel volume lookup. */
static short*           channelleftvol_lookup[NUM_CHANNELS];
static short*           channelrightvol_lookup[NUM_CHANNELS];

/* Pitch to stepping lookup. */
static long             steptable[256];

/* Volume lookups. */
static short            vol_lookup[S_TOTAL_VOLUMES][0x100];


/* Music stuff */

/* Maximum volume of music. */
/* static int            snd_MusicVolume; */

const char*             wildmidi_config_path;

#ifdef WILDMIDI
static d_bool          music_initialised;
static midi*            music_midi;
static d_bool          music_playing;
#endif




/* This function loops all active (internal) sound */
/*  channels, retrieves a given number of samples */
/*  from the raw sound data, modifies it according */
/*  to the current (internal) channel parameters, */
/*  mixes the per channel samples into the global */
/*  mixbuffer, clamping it to the allowed range, */
/*  and sets up everything for transferring the */
/*  contents of the mixbuffer to the (two) */
/*  hardware channels (left and right, that is). */
/* This function currently supports only 16bit. */
static void AudioCallback(short* output_buffer, size_t frames_to_do, void *user_data)
{
	/* Mix current sound data. */

	/* Pointers in mixbuffer. */
	short* output_buffer_end;

	size_t bytes_done;

	const size_t bytes_to_do = frames_to_do * sizeof(short) * 2;

	(void)user_data;

	bytes_done = 0;

#ifdef WILDMIDI
	if (music_playing)
	{
		bytes_done = (size_t)WildMidi_GetOutput(music_midi, (int8_t*)output_buffer, bytes_to_do);

		if (bytes_done < bytes_to_do)
			music_playing = d_false;
	}
#endif

	memset((char*)output_buffer + bytes_done, 0, bytes_to_do - bytes_done);

	/* Determine where the sample ends */
	output_buffer_end = output_buffer + frames_to_do * 2;

	/* Mix sounds into the mix buffer */
	while (output_buffer != output_buffer_end)
	{
		size_t chan;

		/* Obtain base values for mixing */
		long dl = output_buffer[0];
		long dr = output_buffer[1];

		for (chan = 0; chan < NUM_CHANNELS; ++chan)
		{
			/* Check if channel is playing anything */
			if (channels[chan] != NULL)
			{
				/* Get interpolated sample */
				const int interpolation_scale = channelstepremainder[chan] / (1 << 8);
				const int sample = channels[chan][0] + (((channels[chan][1] - channels[chan][0]) * interpolation_scale) / (1 << 8));

				/* Add volume-adjusted sample to mix buffer */
				dl += channelleftvol_lookup[chan][sample];
				dr += channelrightvol_lookup[chan][sample];

				/* Increment sample position */
				channelstepremainder[chan] += channelstep[chan];
				channels[chan] += channelstepremainder[chan] / (1 << 16);
				channelstepremainder[chan] %= 1 << 16;

				/* Disable channel if sound has finished */
				if (channels[chan] >= channelsend[chan])
					channels[chan] = NULL;
			}
		}

#define CAP ((1 << 15) - 1)

		/* Clamp mixed samples to 16-bit range and write them back to the buffer */

		/* Left channel */
		if (dl > CAP)
			*output_buffer++ = CAP;
		else if (dl < -CAP)
			*output_buffer++ = -CAP;
		else
			*output_buffer++ = dl;

		/* Right channel */
		if (dr > CAP)
			*output_buffer++ = CAP;
		else if (dr < -CAP)
			*output_buffer++ = -CAP;
		else
			*output_buffer++ = dr;

#undef CAP
	}
}




static void UpdateSoundParams(int slot, int vol, int sep, int pitch)
{
	int rightvol;
	int leftvol;

	/* Set stepping */
	channelstep[slot] = S_sfx[channelids[slot]].sample_rate * steptable[pitch] / output_sample_rate;
	channelstepremainder[slot] = 0;

	/* Separation, that is, orientation/stereo. */
	/*  range is: 1 - 256 */
	sep += 1;

	/* Per left/right channel. */
	/*  x^2 seperation, */
	/*  adjust volume properly. */
	leftvol =
		vol - ((vol*sep*sep) / (1 << 16)); /* /(256*256); */
	sep = sep - 257;
	rightvol =
		vol - ((vol*sep*sep) / (1 << 16));

#ifdef RANGECHECK
	/* Sanity check, clamp volume. */
	if (rightvol < 0 || rightvol > S_MAX_VOLUME)
		I_Error("rightvol out of bounds");

	if (leftvol < 0 || leftvol > S_MAX_VOLUME)
		I_Error("leftvol out of bounds");
#endif

	/* Get the proper lookup table piece */
	/*  for this volume level */
	channelleftvol_lookup[slot] = vol_lookup[leftvol];
	channelrightvol_lookup[slot] = vol_lookup[rightvol];
}




/* This function loads the sound data from the WAD lump, */
/*  for a single sound. */
static void getsfx(sfxinfo_t* sfxinfo, size_t* len)
{
	unsigned char* sfx;
	size_t         size;
	char           name[20];
	int            sfxlump;

	/* Get the sound data from the WAD, allocate lump */
	/*  in zone memory. */
	sprintf(name, "ds%s", sfxinfo->name);

	/* Now, there is a severe problem with the */
	/*  sound handling, in it is not (yet/anymore) */
	/*  gamemode aware. That means, sounds from */
	/*  DOOM II will be requested even with DOOM */
	/*  shareware. */
	/* The sound list is wired into sounds.c, */
	/*  which sets the external variable. */
	/* I do not do runtime patches to that */
	/*  variable. Instead, we will use a */
	/*  default sound for replacement. */
	if (W_CheckNumForName(name) == -1)
	  sfxlump = W_GetNumForName("dspistol");
	else
	  sfxlump = W_GetNumForName(name);

	size = W_LumpLength(sfxlump);

	sfx = (unsigned char*)W_CacheLumpNum(sfxlump, PU_STATIC);

	*len = size-8-16-16;

	sfxinfo->data = (void*)(sfx+8+16);

	sfxinfo->sample_rate = sfx[2] | (sfx[3] << 8);
}




/* SFX API */

/* This used to set DMX's internal */
/* channel count and sample rate, it seems. */
void I_SetChannels(int channels)
{
	(void)channels;
}




/* Retrieve the raw data lump index */
/*  for a given SFX name. */
int I_GetSfxLumpNum(const sfxinfo_t* sfx)
{
	char namebuf[9];
	sprintf(namebuf, "ds%s", sfx->name);
	return W_GetNumForName(namebuf);
}




/* Starting a sound means adding it */
/*  to the current list of active sounds */
/*  in the internal channels. */
/* As the SFX info struct contains */
/*  e.g. a pointer to the raw data, */
/*  it is ignored. */
/* As our sound handling does not handle */
/*  priority, it is ignored. */
int I_StartSound(int id, int vol, int sep, int pitch)
{
	static unsigned short handlenums = 0;

	int i;
	int rc = -1;

	int oldest = gametic;
	int oldestnum = 0;
	int slot;

	IB_LockSound();

	/* Chainsaw troubles. */
	/* Play these sound effects only one at a time. */
	if (id == sfx_sawup
	 || id == sfx_sawidl
	 || id == sfx_sawful
	 || id == sfx_sawhit
	 || id == sfx_stnmov
	 || id == sfx_pistol)
	{
		/* Loop all channels, check. */
		for (i=0 ; i<NUM_CHANNELS ; ++i)
		{
			/* Active, and using the same SFX? */
			if (channels[i] != NULL && channelids[i] == id)
			{
				/* Reset. */
				channels[i] = NULL;
				break;
			}
		}
	}

	/* Loop all channels to find oldest SFX. */
	for (i=0; i<NUM_CHANNELS && channels[i] != NULL; ++i)
	{
		if (channelstart[i] < oldest)
		{
			oldestnum = i;
			oldest = channelstart[i];
		}
	}

	/* Tales from the cryptic. */
	/* If we found a channel, fine. */
	/* If not, we simply overwrite the first one, 0. */
	/* Probably only happens at startup. */
	if (i == NUM_CHANNELS)
		slot = oldestnum;
	else
		slot = i;

	/* Okay, in the less recent channel, */
	/*  we will handle the new SFX. */
	/* Set pointer to raw data. */
	channels[slot] = (unsigned char*)S_sfx[id].data;
	/* Set pointer to end of raw data. */
	channelsend[slot] = channels[slot] + lengths[id];

	/* Disable handle numbers 0-99 (were they error values in DMX?). */
	if (handlenums == 0)
		handlenums = 100;

	/* Assign current handle number. */
	/* Preserved so sounds could be stopped. */
	channelhandles[slot] = rc = handlenums++;

	/* Should be gametic, I presume. */
	channelstart[slot] = gametic;

	/* Preserve sound SFX id, */
	/*  e.g. for avoiding duplicates of chainsaw. */
	channelids[slot] = id;

	UpdateSoundParams(slot, vol, sep, pitch);

	IB_UnlockSound();

	return rc;
}




void I_StopSound(int handle)
{
	int i;

	for (i=0;i<NUM_CHANNELS;++i)
	{
		if (channelhandles[i] == handle)
		{
			IB_LockSound();

			channels[i] = NULL;

			IB_UnlockSound();
			break;
		}
	}
}




d_bool I_SoundIsPlaying(int handle)
{
	int i;

	for (i=0;i<NUM_CHANNELS;++i)
	{
		if (channelhandles[i] == handle)
		{
			d_bool playing;

			IB_LockSound();

			playing = channels[i] != NULL;

			IB_UnlockSound();

			return playing;
		}
	}

	return 0; /* Sound doesn't exist */
}




void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
	int i;

	for (i=0;i<NUM_CHANNELS;++i)
	{
		if (channelhandles[i] == handle)
		{
			IB_LockSound();

			UpdateSoundParams(i, vol, sep, pitch);

			IB_UnlockSound();

			break;
		}
	}
}




static void StartupCallback(unsigned int _output_sample_rate, void *user_data)
{
	(void)user_data;

	output_sample_rate = _output_sample_rate;

#ifdef WILDMIDI
	if (WildMidi_Init(wildmidi_config_path, output_sample_rate, 0) == 0)
		music_initialised = d_true;
#endif
}

void I_StartupSound(void)
{
	int i;
	int j;

	 /* Init internal lookups (raw data, mixing buffer, channels). */

	/* This table provides step widths for pitch parameters. */
	for (i=0 ; i<256 ; ++i)
		steptable[i] = (long)(pow(2.0, ((i-128)/64.0))*65536.0);

	/* Generates volume lookup tables */
	/*  which also turn the unsigned samples */
	/*  into signed samples. */
	for (i=0 ; i<=S_MAX_VOLUME ; ++i)
		for (j=0 ; j<0x100 ; ++j)
			vol_lookup[i][j] = (i*(j-(0x100/2))*0x100)/S_MAX_VOLUME;

	if (!IB_StartupSound(StartupCallback, AudioCallback, NULL))
		I_Error("I_StartupSound: Failed to initialize backend");

	 /* Cache sounds. */

	/* Initialize external data (all sounds) at start, keep static. */

	for (i=1 ; i<NUMSFX ; ++i)
	{
		/* Alias? Example is the chaingun sound linked to pistol. */
		if (S_sfx[i].link == NULL)
		{
			/* Load data from WAD file. */
			getsfx(&S_sfx[i], &lengths[i]);
		}
		else
		{
			/* Previously loaded already? */
			S_sfx[i].data = S_sfx[i].link->data;
			lengths[i] = lengths[(S_sfx[i].link - S_sfx)/sizeof(sfxinfo_t)];
		}
	}
}




void I_ShutdownSound(void)
{
	IB_ShutdownSound();

#ifdef WILDMIDI
	if (music_initialised)
	{
		WildMidi_Shutdown();
		music_initialised = d_false;
	}
#endif
}




/* MUSIC API. */

void I_PlaySong(int handle, d_bool looping)
{
	/* UNUSED. */
	(void)handle;

#ifndef WILDMIDI
	(void)looping;
#else
	if (music_initialised)
	{
		IB_LockSound();

		music_playing = d_true;
		WildMidi_SetOption(music_midi, WM_MO_LOOP, looping ? WM_MO_LOOP : 0);

		IB_UnlockSound();
	}
#endif
}




void I_PauseSong (int handle)
{
	/* UNUSED. */
	(void)handle;

#ifdef WILDMIDI
	if (music_initialised)
	{
		IB_LockSound();

		music_playing = d_false;

		IB_UnlockSound();
	}
#endif
}




void I_ResumeSong (int handle)
{
	/* UNUSED. */
	(void)handle;

#ifdef WILDMIDI
	if (music_initialised)
	{
		IB_LockSound();

		music_playing = d_true;

		IB_UnlockSound();
	}
#endif
}




void I_StopSong(int handle)
{
	/* UNUSED. */
	(void)handle;

#ifdef WILDMIDI
	if (music_initialised)
	{
		IB_LockSound();

		music_playing = d_false;
		WildMidi_FastSeek(music_midi, 0);

		IB_UnlockSound();
	}
#endif
}




void I_UnRegisterSong(int handle)
{
	/* UNUSED. */
	(void)handle;

#ifdef WILDMIDI
	if (music_initialised)
	{
		IB_LockSound();

		WildMidi_Close(music_midi);

		IB_UnlockSound();
	}
#endif
}




int I_RegisterSong(const void* data, size_t size)
{
#ifndef WILDMIDI
	(void)data;
	(void)size;
#else
	if (music_initialised)
	{
		IB_LockSound();

		music_midi = WildMidi_OpenBuffer((const uint8_t*)data, size);

		IB_UnlockSound();
	}
#endif

	return 1;
}




/* Is the song playing? (unused) */
int I_QrySongPlaying(int handle)
{
#ifdef WILDMIDI
	d_bool playing;
#endif

	/* UNUSED. */
	(void)handle;

#ifndef WILDMIDI
	return 0;
#else
	IB_LockSound();

	playing = music_playing;

	IB_UnlockSound();

	return playing;
#endif
}




void I_SetMusicVolume(int volume)
{
	/* Internal state variable. */
   /* snd_MusicVolume = volume; */
	/* Now set volume on output device. */
#ifndef WILDMIDI
	(void)volume;
#else
	if (music_initialised)
	{
		IB_LockSound();

		WildMidi_MasterVolume(volume);

		IB_UnlockSound();
	}
#endif
}
