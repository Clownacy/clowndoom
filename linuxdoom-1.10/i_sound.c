// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
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
// $Log:$
//
// DESCRIPTION:
//	System interface for sound.
//
//-----------------------------------------------------------------------------

#include <stddef.h>
#include <stdio.h>
#include <math.h>

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

// The number of internal mixing channels,
//  the samples calculated for each mixing step,
//  the size of the 16bit, 2 hardware channel (stereo)
//  mixing buffer, and the samplerate of the raw data.


// Needed for calling the actual sound output.
#define NUM_CHANNELS		8

const char *wildmidi_config_path;

static unsigned int	output_sample_rate;

// Maximum volume of music. Useless so far.
//static int		snd_MusicVolume;

// The actual lengths of all sound effects.
static size_t		lengths[NUMSFX];

// miniaudio context
static ma_context	context;

// miniaudio context
static ma_mutex		mutex;

// The actual output device.
static ma_device	audio_device;

// The channel step amount...
static unsigned int	channelstep[NUM_CHANNELS];
// ... and a 0.16 bit remainder of last step.
static unsigned int	channelstepremainder[NUM_CHANNELS];


// The channel data pointers, start and end.
static unsigned char*	channels[NUM_CHANNELS];
static unsigned char*	channelsend[NUM_CHANNELS];


// Time/gametic that the channel started playing,
//  used to determine oldest, which automatically
//  has lowest priority.
// In case number of active sounds exceeds
//  available channels.
static int		channelstart[NUM_CHANNELS];

// The sound in channel handles,
//  determined on registration,
//  might be used to unregister/stop/modify,
//  currently unused.
static int 		channelhandles[NUM_CHANNELS];

// SFX id of the playing sound effect.
// Used to catch duplicates (like chainsaw).
static int		channelids[NUM_CHANNELS];			

// Pitch to stepping lookup.
static int		steptable[256];

// Volume lookups.
static int		vol_lookup[128*256];

// Hardware left and right channel volume lookup.
static int*		channelleftvol_lookup[NUM_CHANNELS];
static int*		channelrightvol_lookup[NUM_CHANNELS];

// Music stuff
#ifdef WILDMIDI
static boolean music_initialised;
static midi *music_midi;
static boolean music_playing;
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
static void Callback(ma_device *device, void *output_buffer_void, const void *input_buffer, ma_uint32 frames_to_do)
{
  short *output_buffer = output_buffer_void;
#ifdef WILDMIDI
  const size_t bytes_to_do = frames_to_do * 2 * 2;
#endif
  unsigned char		interpolation_scale;

  // Mix current sound data.
  // Data, from raw sound, for right and left.
  register unsigned int	sample;
  register int		dl;
  register int		dr;
  
  // Pointers in global mixbuffer, left, right, end.
  signed short*		leftout;
  signed short*		rightout;
  signed short*		leftend;
  // Step in mixbuffer, left and right, thus two.
  int				step;

  // Mixing channel index.
  int				chan;

  (void)device;
  (void)input_buffer;

    ma_mutex_lock(&mutex);

#ifdef WILDMIDI
    if (music_playing)
    {
	const size_t bytes_done = WildMidi_GetOutput(music_midi, output_buffer_void, bytes_to_do);

	if (bytes_done < bytes_to_do)
	    music_playing = false;
    }
#endif

    // Left and right channel
    //  are in global mixbuffer, alternating.
    leftout = output_buffer;
    rightout = output_buffer+1;
    step = 2;

    // Determine end, for left channel only
    //  (right channel is implicit).
    leftend = output_buffer + frames_to_do*step;

    // Mix sounds into the mixing buffer.
    // Loop over step*SAMPLECOUNT,
    //  that is 512 values for two channels.
    while (leftout != leftend)
    {
	// Reset left/right value. 
	dl = *leftout;
	dr = *rightout;

	// Love thy L2 chache - made this a loop.
	// Now more channels could be set at compile time
	//  as well. Thus loop those  channels.
	for ( chan = 0; chan < NUM_CHANNELS; chan++ )
	{
	    // Check channel, if active.
	    if (channels[ chan ])
	    {
		interpolation_scale = channelstepremainder[ chan ] >> 8;
		// Get the interpolated sample. 
		sample = ((channels[ chan ][0] * (0x100 - interpolation_scale)) + (channels[ chan ][1] * interpolation_scale)) >> 8;
		// Add left and right part
		//  for this channel (sound)
		//  to the current data.
		// Adjust volume accordingly.
		dl += channelleftvol_lookup[ chan ][sample];
		dr += channelrightvol_lookup[ chan ][sample];
		// Increment index ???
		channelstepremainder[ chan ] += channelstep[ chan ];
		// MSB is next sample???
		channels[ chan ] += channelstepremainder[ chan ] >> 16;
		// Limit to LSB???
		channelstepremainder[ chan ] &= 65536-1;

		// Check whether we are done.
		if (channels[ chan ] >= channelsend[ chan ])
		    channels[ chan ] = 0;
	    }
	}
	
	// Clamp to range. Left hardware channel.
	// Has been char instead of short.
	// if (dl > 127) *leftout = 127;
	// else if (dl < -128) *leftout = -128;
	// else *leftout = dl;

	if (dl > 0x7fff)
	    *leftout = 0x7fff;
	else if (dl < -0x8000)
	    *leftout = -0x8000;
	else
	    *leftout = dl;

	// Same for right hardware channel.
	if (dr > 0x7fff)
	    *rightout = 0x7fff;
	else if (dr < -0x8000)
	    *rightout = -0x8000;
	else
	    *rightout = dr;

	// Increment current pointers in mixbuffer.
	leftout += step;
	rightout += step;
    }

    ma_mutex_unlock(&mutex);
}




static void
UpdateSoundParams
( int	slot,
  int	vol,
  int	sep,
  int	pitch)
{
    int		rightvol;
    int		leftvol;

    // Set stepping???
    // Kinda getting the impression this is never used.
    channelstep[slot] = S_sfx[channelids[slot]].sample_rate * steptable[pitch] / output_sample_rate;
    // ???
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
    //  for this volume level???
    channelleftvol_lookup[slot] = &vol_lookup[leftvol*256];
    channelrightvol_lookup[slot] = &vol_lookup[rightvol*256];
}




//
// This function loads the sound data from the WAD lump,
//  for single sound.
//
static void
getsfx
( sfxinfo_t*   sfxinfo,
  size_t*      len )
{
    unsigned char*      sfx;
    int                 size;
    char                name[20];
    int                 sfxlump;

    
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
    if ( W_CheckNumForName(name) == -1 )
      sfxlump = W_GetNumForName("dspistol");
    else
      sfxlump = W_GetNumForName(name);
    
    size = W_LumpLength( sfxlump );

    // Debug.
    // fprintf( stderr, "." );
    //fprintf( stderr, " -loading  %s (lump %d, %d bytes)\n",
    //	     sfxname, sfxlump, size );
    //fflush( stderr );
    
    sfx = (unsigned char*)W_CacheLumpNum( sfxlump, PU_STATIC );
    
    *len = size-8-16-16;

    sfxinfo->data = (void *) (sfx+8+16);

    sfxinfo->sample_rate = sfx[2] | (sfx[3] << 8);
}





//
// SFX API
// Note: this was called by S_Init.
// However, whatever they did in the
// old DPMS based DOS version, this
// were simply dummies in the Linux
// version.
// See soundserver initdata().
//
void I_SetChannels(void)
{
  // Init internal lookups (raw data, mixing buffer, channels).
  // This function sets up internal lookups used during
  //  the mixing process. 
  int		i;
  int		j;
    
  int*	steptablemid = steptable + 128;
  
  // Okay, reset internal mixing channels to zero.
  /*for (i=0; i<NUM_CHANNELS; i++)
  {
    channels[i] = 0;
  }*/

  // This table provides step widths for pitch parameters.
  for (i=-128 ; i<128 ; i++)
    steptablemid[i] = (int)(pow(2.0, (i/64.0))*65536.0);
  
  
  // Generates volume lookup tables
  //  which also turn the unsigned samples
  //  into signed samples.
  for (i=0 ; i<128 ; i++)
    for (j=0 ; j<256 ; j++)
      vol_lookup[i*256+j] = (i*(j-128)*256)/127;
}	

 
// MUSIC API - dummy. Some code from DOS version.
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
// Pitching (that is, increased speed of playback)
//  is set, but currently not used by mixing.
//
int
I_StartSound
( int		id,
  int		vol,
  int		sep,
  int		pitch,
  int		priority )
{
    // UNUSED
    (void)priority;
    
    static unsigned short	handlenums = 0;
 
    int		i;
    int		rc = -1;
    
    int		oldest = gametic;
    int		oldestnum = 0;
    int		slot;

    ma_mutex_lock(&mutex);

    // Chainsaw troubles.
    // Play these sound effects only one at a time.
    if ( id == sfx_sawup
	 || id == sfx_sawidl
	 || id == sfx_sawful
	 || id == sfx_sawhit
	 || id == sfx_stnmov
	 || id == sfx_pistol	 )
    {
	// Loop all channels, check.
	for (i=0 ; i<NUM_CHANNELS ; i++)
	{
	    // Active, and using the same SFX?
	    if ( (channels[i])
		 && (channelids[i] == id) )
	    {
		// Reset.
		channels[i] = 0;
		// We are sure that iff,
		//  there will only be one.
		break;
	    }
	}
    }

    // Loop all channels to find oldest SFX.
    for (i=0; (i<NUM_CHANNELS) && (channels[i]); i++)
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
    channels[slot] = (unsigned char *) S_sfx[id].data;
    // Set pointer to end of raw data.
    channelsend[slot] = channels[slot] + lengths[id];

    // Reset current handle number, limited to 0..100.
    if (!handlenums)
	handlenums = 100;

    // Assign current handle number.
    // Preserved so sounds could be stopped (unused).
    channelhandles[slot] = rc = handlenums++;

    // Should be gametic, I presume.
    channelstart[slot] = gametic;

    // Preserve sound SFX id,
    //  e.g. for avoiding duplicates of chainsaw.
    channelids[slot] = id;

    UpdateSoundParams(slot, vol, sep, pitch);

    ma_mutex_unlock(&mutex);

    // You tell me.
    return rc;
}



void I_StopSound (int handle)
{
  int i;

  for (i=0;i<NUM_CHANNELS;i++)
  {
    if (channelhandles[i] == handle)
    {
      channels[i] = NULL;
      break;
    }
  }
}


int I_SoundIsPlaying(int handle)
{
    int i;

    for (i=0;i<NUM_CHANNELS;i++)
	if (channelhandles[i] == handle)
	    return channels[i] != NULL;

    return 0; // Sound doesn't exist
}




void
I_UpdateSoundParams
( int	handle,
  int	vol,
  int	sep,
  int	pitch)
{
    int i;

    for (i=0;i<NUM_CHANNELS;i++)
    {
	if (channelhandles[i] == handle)
	{
	    UpdateSoundParams(i, vol, sep, pitch);
	    break;
	}
    }
}




void I_ShutdownSound(void)
{    
  ma_device_uninit(&audio_device);
  ma_mutex_uninit(&mutex);
  ma_context_uninit(&context);
}






void
I_InitSound(void)
{     
  int i;

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

  ma_device_start(&audio_device);
    
  // Initialize external data (all sounds) at start, keep static.
  fprintf( stderr, "I_InitSound: ");
  
  for (i=1 ; i<NUMSFX ; i++)
  { 
    // Alias? Example is the chaingun sound linked to pistol.
    if (!S_sfx[i].link)
    {
      // Load data from WAD file.
      getsfx( &S_sfx[i], &lengths[i] );
    }	
    else
    {
      // Previously loaded already?
      S_sfx[i].data = S_sfx[i].link->data;
      lengths[i] = lengths[(S_sfx[i].link - S_sfx)/sizeof(sfxinfo_t)];
    }
  }

  fprintf( stderr, " pre-cached all sound data\n");
  
  // Finished initialization.
  fprintf(stderr, "I_InitSound: sound module ready\n");
    
}




//
// MUSIC API.
//

void I_InitMusic(void)
{
#ifdef WILDMIDI
    ma_mutex_lock(&mutex);

    if (WildMidi_Init(wildmidi_config_path, output_sample_rate, 0) == 0)
	music_initialised = true;

    ma_mutex_unlock(&mutex);
#endif
}

void I_ShutdownMusic(void)
{
#ifdef WILDMIDI
    if (music_initialised)
    {
	ma_mutex_lock(&mutex);

	WildMidi_Shutdown();
	music_initialised = false;

	ma_mutex_unlock(&mutex);
    }
#endif
}

void I_PlaySong(int handle, int looping)
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
