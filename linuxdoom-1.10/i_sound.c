//-----------------------------------------------------------------------------
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//  System interface for sound.
//
//-----------------------------------------------------------------------------

#include <math.h>
#include <stddef.h>
#include <stdio.h>

#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#include "miniaudio.h"

#ifdef WILDMIDI
#include "wildmidi_lib.h"
#endif

#include "z_zone.h"

#include "i_system.h"
#include "i_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"

#include "doomdef.h"

#define NUM_CHANNELS    8

static unsigned int     output_sample_rate;

// miniaudio context
static ma_context       context;

// miniaudio context
static ma_mutex         mutex;

// The actual output device.
static ma_device        audio_device;


// The actual lengths of all sound effects.
static size_t           lengths[NUMSFX];


// The channel step amount...
static unsigned long    channelstep[NUM_CHANNELS];
// ... and a 0.16 bit remainder of last step.
static unsigned long    channelstepremainder[NUM_CHANNELS];

// The channel data pointers, start and end.
static unsigned char*   channels[NUM_CHANNELS];
static unsigned char*   channelsend[NUM_CHANNELS];

// Time/gametic that the channel started playing,
//  used to determine oldest, which automatically
//  has lowest priority.
// In case number of active sounds exceeds
//  available channels.
static int              channelstart[NUM_CHANNELS];

// The sound in channel handles,
//  determined on registration.
static int              channelhandles[NUM_CHANNELS];

// SFX id of the playing sound effect.
// Used to catch duplicates (like chainsaw).
static int              channelids[NUM_CHANNELS];

// Hardware left and right channel volume lookup.
static short*           channelleftvol_lookup[NUM_CHANNELS];
static short*           channelrightvol_lookup[NUM_CHANNELS];

// Pitch to stepping lookup.
static long             steptable[256];

// Volume lookups.
static short            vol_lookup[128][256];


// Music stuff

// Maximum volume of music.
//static int            snd_MusicVolume;

const char*             wildmidi_config_path;

#ifdef WILDMIDI
static boolean          music_initialised;
static midi*            music_midi;
static boolean          music_playing;
#endif




//
// This function loops all active (internal) sound
//  channels, retrieves a given number of samples
//  from the raw sound data, modifies it according
//  to the current (internal) channel parameters,
//  mixes the per channel samples into the global
//  mixbuffer, clamping it to the allowed range,
//  and sets up everything for transferring the
//  contents of the mixbuffer to the (two)
//  hardware channels (left and right, that is).
//
// This function currently supports only 16bit.
//
static void Callback(ma_device* device, void* output_buffer_void, const void* input_buffer, ma_uint32 frames_to_do)
{
    // Mix current sound data.
    // Data, from raw sound, for right and left.
    register unsigned char sample;
    register long          dl;
    register long          dr;

    // Pointers in mixbuffer.
    short*                 output_buffer;
    short*                 output_buffer_end;

    // Mixing channel index.
    size_t                 chan;

#ifdef WILDMIDI
    size_t                 bytes_to_do;
#endif
    unsigned char          interpolation_scale;

    (void)device;
    (void)input_buffer;

    ma_mutex_lock(&mutex);

#ifdef WILDMIDI
    if (music_playing)
    {
        bytes_to_do = frames_to_do * sizeof(short) * 2;

        if ((size_t)WildMidi_GetOutput(music_midi, output_buffer_void, bytes_to_do) < bytes_to_do)
            music_playing = false;
    }
#endif

    output_buffer = (short*)output_buffer_void;

    // Determine where the sample ends
    output_buffer_end = output_buffer + frames_to_do * 2;

    // Mix sounds into the mix buffer
    while (output_buffer != output_buffer_end)
    {
        // Obtain base values for mixing
        dl = output_buffer[0];
        dr = output_buffer[1];

        for (chan = 0; chan < NUM_CHANNELS; ++chan)
        {
            // Check if channel is playing anything
            if (channels[chan] != NULL)
            {
                // Get interpolated sample
                interpolation_scale = channelstepremainder[chan] >> 8;
                sample = ((channels[chan][0] * (0x100 - interpolation_scale)) + (channels[chan][1] * interpolation_scale)) >> 8;

                // Add volume-adjusted sample to mix buffer
                dl += channelleftvol_lookup[chan][sample];
                dr += channelrightvol_lookup[chan][sample];

                // Increment sample position
                channelstepremainder[chan] += channelstep[chan];
                channels[chan] += channelstepremainder[chan] >> 16;
                channelstepremainder[chan] &= 0xFFFF;

                // Disable channel if sound has finished
                if (channels[chan] >= channelsend[chan])
                    channels[chan] = NULL;
            }
        }

        // Clamp mixed samples to 16-bit range and write them back to the buffer

        // Left channel
        if (dl > 0x7FFF)
            *output_buffer++ = 0x7FFF;
        else if (dl < -0x7FFF)
            *output_buffer++ = -0x7FFF;
        else
            *output_buffer++ = dl;

        // Right channel
        if (dr > 0x7FFF)
            *output_buffer++ = 0x7FFF;
        else if (dr < -0x7FFF)
            *output_buffer++ = -0x7FFF;
        else
            *output_buffer++ = dr;
    }

    ma_mutex_unlock(&mutex);
}




static void UpdateSoundParams(int slot, int vol, int sep, int pitch)
{
    int rightvol;
    int leftvol;

    // Set stepping
    channelstep[slot] = S_sfx[channelids[slot]].sample_rate * steptable[pitch] / output_sample_rate;
    channelstepremainder[slot] = 0;

    // Separation, that is, orientation/stereo.
    //  range is: 1 - 256
    sep += 1;

    // Per left/right channel.
    //  x^2 seperation,
    //  adjust volume properly.
    leftvol =
        vol - ((vol*sep*sep) >> 16); ///(256*256);
    sep = sep - 257;
    rightvol =
        vol - ((vol*sep*sep) >> 16);

#ifdef RANGECHECK
    // Sanity check, clamp volume.
    if (rightvol < 0 || rightvol > 127)
        I_Error("rightvol out of bounds");

    if (leftvol < 0 || leftvol > 127)
        I_Error("leftvol out of bounds");
#endif

    // Get the proper lookup table piece
    //  for this volume level
    channelleftvol_lookup[slot] = vol_lookup[leftvol];
    channelrightvol_lookup[slot] = vol_lookup[rightvol];
}




//
// This function loads the sound data from the WAD lump,
//  for a single sound.
//
static void getsfx(sfxinfo_t* sfxinfo, size_t* len)
{
    unsigned char* sfx;
    size_t         size;
    char           name[20];
    int            sfxlump;

    // Get the sound data from the WAD, allocate lump
    //  in zone memory.
    sprintf(name, "ds%s", sfxinfo->name);

    // Now, there is a severe problem with the
    //  sound handling, in it is not (yet/anymore)
    //  gamemode aware. That means, sounds from
    //  DOOM II will be requested even with DOOM
    //  shareware.
    // The sound list is wired into sounds.c,
    //  which sets the external variable.
    // I do not do runtime patches to that
    //  variable. Instead, we will use a
    //  default sound for replacement.
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




//
// SFX API
//

//
// This used to set DMX's internal
// channel count and sample rate, it seems.
//
void I_SetChannels(int channels)
{
    (void)channels;
}




//
// Retrieve the raw data lump index
//  for a given SFX name.
//
int I_GetSfxLumpNum(const sfxinfo_t* sfx)
{
    char namebuf[9];
    sprintf(namebuf, "ds%s", sfx->name);
    return W_GetNumForName(namebuf);
}




//
// Starting a sound means adding it
//  to the current list of active sounds
//  in the internal channels.
// As the SFX info struct contains
//  e.g. a pointer to the raw data,
//  it is ignored.
// As our sound handling does not handle
//  priority, it is ignored.
//
int I_StartSound(int id, int vol, int sep, int pitch)
{
    static unsigned short handlenums = 0;

    int i;
    int rc = -1;

    int oldest = gametic;
    int oldestnum = 0;
    int slot;

    ma_mutex_lock(&mutex);

    // Chainsaw troubles.
    // Play these sound effects only one at a time.
    if (id == sfx_sawup
     || id == sfx_sawidl
     || id == sfx_sawful
     || id == sfx_sawhit
     || id == sfx_stnmov
     || id == sfx_pistol)
    {
        // Loop all channels, check.
        for (i=0 ; i<NUM_CHANNELS ; ++i)
        {
            // Active, and using the same SFX?
            if (channels[i] != NULL && channelids[i] == id)
            {
                // Reset.
                channels[i] = NULL;
                break;
            }
        }
    }

    // Loop all channels to find oldest SFX.
    for (i=0; i<NUM_CHANNELS && channels[i] != NULL; ++i)
    {
        if (channelstart[i] < oldest)
        {
            oldestnum = i;
            oldest = channelstart[i];
        }
    }

    // Tales from the cryptic.
    // If we found a channel, fine.
    // If not, we simply overwrite the first one, 0.
    // Probably only happens at startup.
    if (i == NUM_CHANNELS)
        slot = oldestnum;
    else
        slot = i;

    // Okay, in the less recent channel,
    //  we will handle the new SFX.
    // Set pointer to raw data.
    channels[slot] = (unsigned char*)S_sfx[id].data;
    // Set pointer to end of raw data.
    channelsend[slot] = channels[slot] + lengths[id];

    // Disable handle numbers 0-99 (were they error values in DMX?).
    if (handlenums == 0)
        handlenums = 100;

    // Assign current handle number.
    // Preserved so sounds could be stopped.
    channelhandles[slot] = rc = handlenums++;

    // Should be gametic, I presume.
    channelstart[slot] = gametic;

    // Preserve sound SFX id,
    //  e.g. for avoiding duplicates of chainsaw.
    channelids[slot] = id;

    UpdateSoundParams(slot, vol, sep, pitch);

    ma_mutex_unlock(&mutex);

    return rc;
}




void I_StopSound(int handle)
{
    int i;

    for (i=0;i<NUM_CHANNELS;++i)
    {
        if (channelhandles[i] == handle)
        {
	    ma_mutex_lock(&mutex);

            channels[i] = NULL;

	    ma_mutex_unlock(&mutex);
            break;
        }
    }
}




boolean I_SoundIsPlaying(int handle)
{
    int i;

    for (i=0;i<NUM_CHANNELS;++i)
    {
        if (channelhandles[i] == handle)
	{
	    ma_mutex_lock(&mutex);

            boolean playing = channels[i] != NULL;

	    ma_mutex_unlock(&mutex);

	    return playing;
	}
    }

    return 0; // Sound doesn't exist
}




void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
    int i;

    for (i=0;i<NUM_CHANNELS;++i)
    {
        if (channelhandles[i] == handle)
        {
	    ma_mutex_lock(&mutex);

            UpdateSoundParams(i, vol, sep, pitch);

	    ma_mutex_unlock(&mutex);

            break;
        }
    }
}




void I_StartupSound(void)
{
    int i;
    int j;

      //
     // Init internal lookups (raw data, mixing buffer, channels).
    //

    // This table provides step widths for pitch parameters.
    for (i=0 ; i<256 ; ++i)
        steptable[i] = (long)(pow(2.0, ((i-128)/64.0))*65536.0);

    // Generates volume lookup tables
    //  which also turn the unsigned samples
    //  into signed samples.
    for (i=0 ; i<128 ; ++i)
        for (j=0 ; j<256 ; ++j)
            vol_lookup[i][j] = (i*(j-128)*256)/127;

      //
     // Set up miniaudio and WildMIDI.
    //

    ma_context_init(NULL, 0, NULL, &context);

    ma_mutex_init(&mutex);

    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.pDeviceID = NULL;
    config.playback.format = ma_format_s16;
    config.playback.channels = 2;
    config.sampleRate = 0; // Let miniaudio decide what sample rate to use
    config.dataCallback = Callback;
    config.pUserData = NULL;

    ma_device_init(&context, &config, &audio_device);

    output_sample_rate = audio_device.sampleRate;

#ifdef WILDMIDI
    if (WildMidi_Init(wildmidi_config_path, output_sample_rate, 0) == 0)
        music_initialised = true;
#endif

    ma_device_start(&audio_device);

      //
     // Cache sounds.
    //

    // Initialize external data (all sounds) at start, keep static.

    for (i=1 ; i<NUMSFX ; ++i)
    {
        // Alias? Example is the chaingun sound linked to pistol.
        if (S_sfx[i].link == NULL)
        {
            // Load data from WAD file.
            getsfx(&S_sfx[i], &lengths[i]);
        }
        else
        {
            // Previously loaded already?
            S_sfx[i].data = S_sfx[i].link->data;
            lengths[i] = lengths[(S_sfx[i].link - S_sfx)/sizeof(sfxinfo_t)];
        }
    }
}




void I_ShutdownSound(void)
{
    ma_device_uninit(&audio_device);
    ma_mutex_uninit(&mutex);
    ma_context_uninit(&context);

#ifdef WILDMIDI
    if (music_initialised)
    {
	WildMidi_Shutdown();
	music_initialised = false;
    }
#endif
}




//
// MUSIC API.
//

void I_PlaySong(int handle, boolean looping)
{
    // UNUSED.
    (void)handle;

#ifndef WILDMIDI
    (void)looping;
#else
    if (music_initialised)
    {
        ma_mutex_lock(&mutex);

        music_playing = true;
        WildMidi_SetOption(music_midi, WM_MO_LOOP, looping ? WM_MO_LOOP : 0);

        ma_mutex_unlock(&mutex);
    }
#endif
}




void I_PauseSong (int handle)
{
    // UNUSED.
    (void)handle;

#ifdef WILDMIDI
    if (music_initialised)
    {
        ma_mutex_lock(&mutex);

        music_playing = false;

        ma_mutex_unlock(&mutex);
    }
#endif
}




void I_ResumeSong (int handle)
{
    // UNUSED.
    (void)handle;

#ifdef WILDMIDI
    if (music_initialised)
    {
        ma_mutex_lock(&mutex);

        music_playing = true;

        ma_mutex_unlock(&mutex);
    }
#endif
}




void I_StopSong(int handle)
{
    // UNUSED.
    (void)handle;

#ifdef WILDMIDI
    if (music_initialised)
    {
        ma_mutex_lock(&mutex);

        music_playing = false;
        WildMidi_FastSeek(music_midi, 0);

        ma_mutex_unlock(&mutex);
    }
#endif
}




void I_UnRegisterSong(int handle)
{
    // UNUSED.
    (void)handle;

#ifdef WILDMIDI
    if (music_initialised)
    {
        ma_mutex_lock(&mutex);

        WildMidi_Close(music_midi);

        ma_mutex_unlock(&mutex);
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
        ma_mutex_lock(&mutex);

        music_midi = WildMidi_OpenBuffer((void*)data, size);

        ma_mutex_unlock(&mutex);
    }
#endif

    return 1;
}




// Is the song playing? (unused)
int I_QrySongPlaying(int handle)
{
    // UNUSED.
    (void)handle;

#ifndef WILDMIDI
    return 0;
#else
    ma_mutex_lock(&mutex);

    boolean playing = music_playing;

    ma_mutex_unlock(&mutex);

    return playing;
#endif
}




void I_SetMusicVolume(int volume)
{
    // Internal state variable.
    //snd_MusicVolume = volume;
    // Now set volume on output device.
#ifndef WILDMIDI
    (void)volume;
#else
    if (music_initialised)
    {
        ma_mutex_lock(&mutex);

        WildMidi_MasterVolume(volume);

        ma_mutex_unlock(&mutex);
    }
#endif
}
