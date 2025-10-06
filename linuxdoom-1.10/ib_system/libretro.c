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

******************************************************************************/

#include "../ib_system.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "libretro-callbacks.h"

#include "../doomdef.h"
#include "../d_main.h"
#include "../m_misc.h"
#include "../i_system.h"

/******************/
/* libretro stuff */
/******************/

#include "libco.h"

LibretroCallbacks libretro;

static cothread_t main_coroutine, game_coroutine;
static char *iwad_path;

static void GameEntryPoint(void)
{
	static char *argv[3];
	argv[0] = "";
	argv[1] = "-iwad";
	argv[2] = iwad_path;
	D_DoomMain(D_COUNT_OF(argv), argv);
}

void retro_init(void)
{
	/* Doom uses 'main-loops', which are incompatible with libretro's cooperative multitasking model.
	   So, we use libretro's 'libco' library to work-around this issue with coroutines. */
	main_coroutine = co_active();
	game_coroutine = co_create(64 * 1024, GameEntryPoint);
}

void retro_deinit(void)
{
	co_delete(game_coroutine);
}

unsigned int retro_api_version(void)
{
	return RETRO_API_VERSION;
}

void retro_set_controller_port_device(const unsigned int port, const unsigned int device)
{
	(void)port;
	(void)device;
}

#ifndef GIT_VERSION
#define GIT_VERSION ""
#endif

void retro_get_system_info(struct retro_system_info* const info)
{
	memset(info, 0, sizeof(*info));
	info->library_name     = "ClownDoom";
	info->library_version  = "v1.1" GIT_VERSION;
	info->need_fullpath    = true;
	info->valid_extensions = "wad";
}

void retro_get_system_av_info(struct retro_system_av_info* const info)
{
	info->geometry.base_width   = ORIGINAL_SCREEN_WIDTH;
	info->geometry.base_height  = ORIGINAL_SCREEN_HEIGHT;
	info->geometry.max_width    = MAXIMUM_SCREENWIDTH;
	info->geometry.max_height   = MAXIMUM_SCREENHEIGHT;
	info->geometry.aspect_ratio = 0.0f;

	info->timing.fps            = TICRATE;
	info->timing.sample_rate    = LIBRETRO_SAMPLE_RATE;
}

void retro_set_environment(const retro_environment_t cb)
{
	libretro.environment = cb;
}

void retro_set_audio_sample(const retro_audio_sample_t cb)
{
	libretro.audio = cb;
}

void retro_set_audio_sample_batch(const retro_audio_sample_batch_t cb)
{
	libretro.audio_batch = cb;
}

void retro_set_input_poll(const retro_input_poll_t cb)
{
	libretro.input_poll = cb;
}

void retro_set_input_state(const retro_input_state_t cb)
{
	libretro.input_state = cb;
}

void retro_set_video_refresh(const retro_video_refresh_t cb)
{
	libretro.video = cb;
}

void retro_reset(void)
{
	/* TODO: It would be a good idea to return to the title screen here. */
}

void retro_run(void)
{
	/* TODO: Use 'libretro.audio' instead, so that this buffer is not needed. */
	int16_t audio_buffer[LIBRETRO_SAMPLE_RATE / TICRATE][LIBRETRO_CHANNEL_COUNT];

	/* `libretro.h` says this must be called every time that `retro_run` is called. */
	libretro.input_poll();

	co_switch(game_coroutine);

	libretro.generate_audio.callback(&audio_buffer[0][0], D_COUNT_OF(audio_buffer), libretro.generate_audio.user_data);
	libretro.audio_batch(&audio_buffer[0][0], D_COUNT_OF(audio_buffer));
}

bool retro_load_game(const struct retro_game_info* const info)
{
	return retro_load_game_special(0, info, 1);
}

void retro_unload_game(void)
{
	/* Shut-down the game. */
	I_Quit();
}

unsigned int retro_get_region(void)
{
	/* Irrelevant, but `libretro.h` says that this should be used as a fallback. */
	return RETRO_REGION_NTSC;
}

bool retro_load_game_special(const unsigned int type, const struct retro_game_info* const info, const size_t num)
{
	/* TODO: Allow PWADs to be loaded here. */
	(void)type;
	(void)num;

	iwad_path = M_strdup(info[0].path);

	return true;
}

size_t retro_serialize_size(void)
{
	/* TODO: Use Doom's save system for this? Those are basically save states. */
	return 0;
}

bool retro_serialize(void* const data, const size_t size)
{
	(void)data;
	(void)size;
	return false;
}

bool retro_unserialize(const void* const data, const size_t size)
{
	(void)data;
	(void)size;
	return false;
}

void* retro_get_memory_data(const unsigned int id)
{
	(void)id;
	return NULL;
}

size_t retro_get_memory_size(const unsigned int id)
{
	(void)id;
	return 0;
}

void retro_cheat_reset(void)
{
	
}

void retro_cheat_set(const unsigned int index, const bool enabled, const char* const code)
{
	(void)index;
	(void)enabled;
	(void)code;
}

/**************/
/* Doom stuff */
/**************/

/* IB_GetTime */
/* returns time in 1/70th second tics */
int  IB_GetTime (void)
{
	return clock() / (CLOCKS_PER_SEC / TICRATE);
}


/* IB_Init */
void IB_Init (void)
{
	
}


/* IB_Quit */
void IB_Quit (void)
{
	
}


void IB_WaitVBL(int count)
{
	(void)count;
}


void IB_Sleep(void)
{
	
}


size_t IB_GetConfigPath(char* const buffer, const size_t size)
{
	const char *system_directory;

	if (libretro.environment(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &system_directory))
	{
		size_t copy_length = M_StringCopy(buffer, size, system_directory);
		copy_length += M_StringCopyOffset(buffer, size, copy_length, "/");
		return copy_length;
	}

	return 0;
}

void IB_Yield(void)
{
	co_switch(main_coroutine);
}
