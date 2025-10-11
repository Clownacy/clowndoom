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
#include "streams/file_stream.h"
#include "libretro_core_options.h"

#include "../doomdef.h"
#include "../d_main.h"
#include "../i_system.h"
#include "../ib_video.h"
#include "../m_menu.h"
#include "../m_misc.h"
#include "../r_data.h"
#include "../r_main.h"
#include "../v_video.h"

/******************/
/* libretro stuff */
/******************/

#include "libco.h"

LibretroCallbacks libretro;

static cothread_t main_coroutine, game_coroutine;
static char *iwad_path;
static char *pwad_paths[1];
static bool game_exited;

static int SCREENWIDTH_OPTION;
static int HUD_SCALE_OPTION;
static double aspect_ratio_option;

static bool DoOptionBoolean(const char* const key, const char* const true_value)
{
	struct retro_variable variable;
	variable.key = key;
	return libretro.environment(RETRO_ENVIRONMENT_GET_VARIABLE, &variable) && variable.value != NULL && strcmp(variable.value, true_value) == 0;
}

static int DoOptionNumerical(const char* const key)
{
	struct retro_variable variable;

	variable.key = key;
	if (libretro.environment(RETRO_ENVIRONMENT_GET_VARIABLE, &variable) && variable.value != NULL)
		return atoi(variable.value);

	return 0;
}

static double DoOptionFloating(const char* const key)
{
	struct retro_variable variable;

	variable.key = key;
	if (libretro.environment(RETRO_ENVIRONMENT_GET_VARIABLE, &variable) && variable.value != NULL)
		return atof(variable.value);

	return 0;
}

static void DoOptionBooleanWithCallback(const bool initial, int* const option, const char* const name, const char* const true_value, void (* const callback)(void))
{
	const bool new_setting = DoOptionBoolean(name, true_value);

	if (*option != new_setting)
	{
		*option = new_setting;

		if (!initial && callback != NULL)
			callback();
	}
}

static void DoOptionNumericalWithCallback(const bool initial, int* const option, const char* const name, void (* const callback)(void))
{
	const int new_setting = DoOptionNumerical(name);

	if (*option != new_setting)
	{
		*option = new_setting;

		if (!initial && callback != NULL)
			callback();
	}
}

static void DoOptionFloatingWithCallback(const bool initial, double* const option, const char* const name, void (* const callback)(void))
{
	const double new_setting = DoOptionFloating(name);

	if (*option != new_setting)
	{
		*option = new_setting;

		if (!initial && callback != NULL)
			callback();
	}
}

static void UpdateOptions(const bool initial)
{
	/* General. */
	DoOptionBooleanWithCallback(initial,
		&showMessages,
		"clowndoom_show_messages",
		"enabled",
		M_ChangedShowMessages);
	DoOptionNumericalWithCallback(initial,
		&mouseSensitivity,
		"clowndoom_mouse_sensitivity",
		M_ChangedMouseSensitivity);
	DoOptionBooleanWithCallback(initial,
		&novert,
		"clowndoom_move_with_mouse",
		"disabled",
		NULL);
	DoOptionBooleanWithCallback(initial,
		&always_run,
		"clowndoom_always_run",
		"enabled",
		NULL);
	DoOptionBooleanWithCallback(initial,
		&always_strafe,
		"clowndoom_always_strafe",
		"enabled",
		NULL);

	/* Video. */
	DoOptionBooleanWithCallback(initial,
		&detailLevel,
		"clowndoom_graphic_detail",
		"Low",
		M_ChangedGraphicDetail);
	DoOptionNumericalWithCallback(initial,
		&screenblocks,
		"clowndoom_screen_size",
		M_ChangedScreenBlocks);
	DoOptionNumericalWithCallback(initial,
		&usegamma,
		"clowndoom_use_gamma",
		M_ChangedUseGamma);
	DoOptionBooleanWithCallback(initial,
		&aspect_ratio_correction,
		"clowndoom_aspect_ratio_correction",
		"enabled",
		M_ChangedAspectRatioCorrection);
	DoOptionBooleanWithCallback(initial,
		&full_colour,
		"clowndoom_full_colour",
		"enabled",
		M_ChangedFullColour);
	DoOptionBooleanWithCallback(initial,
		&prototype_light_amplification_visor_effect,
		"clowndoom_prototype_light_amplification_visor_effect",
		"enabled",
		M_ChangedPrototypeLightAmplificationVisorEffect);
	DoOptionNumericalWithCallback(initial,
		&SCREENWIDTH_OPTION,
		"clowndoom_horizontal_resolution",
		NULL);
	DoOptionNumericalWithCallback(initial,
		&HUD_SCALE_OPTION,
		"clowndoom_hud_scale",
		NULL);
	DoOptionFloatingWithCallback(initial,
		&aspect_ratio_option,
		"clowndoom_aspect_ratio",
		NULL);

	/* Audio. */
	DoOptionNumericalWithCallback(initial,
		&sfxVolume,
		"clowndoom_sfx_volume",
		M_ChangedSFXVolume);
	DoOptionNumericalWithCallback(initial,
		&musicVolume,
		"clowndoom_music_volume",
		M_ChangedMusicVolume);
}

void IB_ChangedShowMessages(void)
{
	struct retro_variable variable;
	variable.key = "clowndoom_show_messages";
	variable.value = showMessages ? "enabled" : "disabled";

	libretro.environment(RETRO_ENVIRONMENT_SET_VARIABLE, &variable);
}

void IB_ChangedMouseSensitivity(void)
{
	char buffer[2];
	struct retro_variable variable;

	sprintf(buffer, "%d", mouseSensitivity);

	variable.key = "clowndoom_mouse_sensitivity";
	variable.value = buffer;

	libretro.environment(RETRO_ENVIRONMENT_SET_VARIABLE, &variable);
}

void IB_ChangedGraphicDetail(void)
{
	struct retro_variable variable;
	variable.key = "clowndoom_graphic_detail";
	variable.value = detailLevel ? "Low" : "High";

	libretro.environment(RETRO_ENVIRONMENT_SET_VARIABLE, &variable);
}

void IB_ChangedScreenBlocks(void)
{
	char buffer[3];
	struct retro_variable variable;

	sprintf(buffer, "%d", screenblocks);

	variable.key = "clowndoom_screen_size";
	variable.value = buffer;

	libretro.environment(RETRO_ENVIRONMENT_SET_VARIABLE, &variable);
}

void IB_ChangedUseGamma(void)
{
	char buffer[2];
	struct retro_variable variable;

	sprintf(buffer, "%d", usegamma);

	variable.key = "clowndoom_use_gamma";
	variable.value = buffer;

	libretro.environment(RETRO_ENVIRONMENT_SET_VARIABLE, &variable);
}

void IB_ChangedAspectRatioCorrection(void)
{
	struct retro_variable variable;
	variable.key = "clowndoom_aspect_ratio_correction";
	variable.value = aspect_ratio_correction ? "enabled" : "disabled";

	libretro.environment(RETRO_ENVIRONMENT_SET_VARIABLE, &variable);
}

void IB_ChangedFullColour(void)
{
	struct retro_variable variable;
	variable.key = "clowndoom_full_colour";
	variable.value = full_colour ? "enabled" : "disabled";

	libretro.environment(RETRO_ENVIRONMENT_SET_VARIABLE, &variable);
}

void IB_ChangedPrototypeLightAmplificationVisorEffect(void)
{
	struct retro_variable variable;
	variable.key = "clowndoom_prototype_light_amplification_visor_effect";
	variable.value = prototype_light_amplification_visor_effect ? "enabled" : "disabled";

	libretro.environment(RETRO_ENVIRONMENT_SET_VARIABLE, &variable);
}

void IB_ChangedSFXVolume(void)
{
	char buffer[3];
	struct retro_variable variable;

	sprintf(buffer, "%d", sfxVolume);

	variable.key = "clowndoom_sfx_volume";
	variable.value = buffer;

	libretro.environment(RETRO_ENVIRONMENT_SET_VARIABLE, &variable);
}

void IB_ChangedMusicVolume(void)
{
	char buffer[3];
	struct retro_variable variable;

	sprintf(buffer, "%d", musicVolume);

	variable.key = "clowndoom_music_volume";
	variable.value = buffer;

	libretro.environment(RETRO_ENVIRONMENT_SET_VARIABLE, &variable);
}

static void GameEntryPoint(void)
{
	static char *argv[5];
	size_t argc = 0;

	argv[ 0] = "";
	argv[ 1] = "-iwad";
	argv[ 2] = iwad_path;
	argv[ 3] = "-file";
	argv[ 4] = pwad_paths[0];

	while (argc < D_COUNT_OF(argv) && argv[argc] != NULL)
		++argc;

	D_DoomMain(argc, argv);
}

void retro_init(void)
{
	/* Doom uses 'main-loops', which are incompatible with libretro's cooperative multitasking model.
	   So, we use libretro's 'libco' library to work-around this issue with coroutines. */
	main_coroutine = co_active();
	game_coroutine = co_create(64 * 1024, GameEntryPoint);

	UpdateOptions(true);

	SCREENWIDTH = SCREENWIDTH_OPTION;
	SCREENHEIGHT = SCREENWIDTH / aspect_ratio_option;
	if (aspect_ratio_correction)
		SCREENHEIGHT = SCREENHEIGHT * 5 / 6;

	HUD_SCALE = HUD_SCALE_OPTION;
}

void retro_deinit(void)
{
	if (!game_exited)
		I_Quit(0);

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
	info->geometry.base_width   = SCREENWIDTH;
	info->geometry.base_height  = SCREENHEIGHT;
	info->geometry.max_width    = MAXIMUM_SCREENWIDTH;
	info->geometry.max_height   = MAXIMUM_SCREENHEIGHT;
	info->geometry.aspect_ratio = (float)info->geometry.base_width / info->geometry.base_height;
	if (aspect_ratio_correction)
		info->geometry.aspect_ratio = info->geometry.aspect_ratio * 5 / 6;

	info->timing.fps            = TICRATE;
	info->timing.sample_rate    = LIBRETRO_SAMPLE_RATE;

	{
		enum retro_pixel_format format;

		format = RETRO_PIXEL_FORMAT_XRGB8888;
		if (!libretro.environment(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &format))
		{
			format = RETRO_PIXEL_FORMAT_RGB565;
			if (!libretro.environment(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &format))
			{
				format = RETRO_PIXEL_FORMAT_0RGB1555;
			}
		}

		libretro.framebuffer_format = format;
	}
}

static unsigned int KeycodeToNative(const unsigned int keycode)
{
	switch (keycode)
	{
		default:
			/* ASCII that can be passed through unmodified. */
			if (keycode < ' ' || keycode > '~')
				break;

			return keycode;

		case RETROK_RIGHT:
			return KEY_RIGHTARROW;
		case RETROK_LEFT:
			return KEY_LEFTARROW;
		case RETROK_UP:
			return KEY_UPARROW;
		case RETROK_DOWN:
			return KEY_DOWNARROW;
		case RETROK_ESCAPE:
			return KEY_ESCAPE;
		case RETROK_RETURN:
			return KEY_ENTER;
		case RETROK_TAB:
			return KEY_TAB;
		case RETROK_F1:
			return KEY_F1;
		case RETROK_F2:
			return KEY_F2;
		case RETROK_F3:
			return KEY_F3;
		case RETROK_F4:
			return KEY_F4;
		case RETROK_F5:
			return KEY_F5;
		case RETROK_F6:
			return KEY_F6;
		case RETROK_F7:
			return KEY_F7;
		case RETROK_F8:
			return KEY_F8;
		case RETROK_F9:
			return KEY_F9;
		case RETROK_F10:
			return KEY_F10;
		case RETROK_F11:
			return KEY_F11;
		case RETROK_F12:
			return KEY_F12;

		case RETROK_BACKSPACE:
			return KEY_BACKSPACE;
		case RETROK_PAUSE:
			return KEY_PAUSE;

		case RETROK_EQUALS:
			return KEY_EQUALS;
		case RETROK_MINUS:
			return KEY_MINUS;

		case RETROK_RSHIFT:
			return KEY_RSHIFT;
		case RETROK_RCTRL:
			return KEY_RCTRL;
		case RETROK_RALT:
			return KEY_RALT;
	}

	return 0;
}

static void KeyboardCallback(const bool down, const unsigned int keycode, const uint32_t character, const uint16_t key_modifiers)
{
	event_t event;

	(void)character;
	(void)key_modifiers;

	event.type = down ? ev_keydown : ev_keyup;
	event.data1 = KeycodeToNative(keycode);

	if (event.data1 != 0)
		D_PostEvent(&event);
}

void retro_set_environment(const retro_environment_t cb)
{
	libretro.environment = cb;

	libretro_set_core_options(libretro.environment);

	{
		const struct retro_keyboard_callback callback = {KeyboardCallback};

		/* TODO: Handle this not being available! */
		libretro.environment(RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK, (void*)&callback);
	}

	/* Get filesystem callbacks. */
	{
		struct retro_vfs_interface_info info;
		info.required_interface_version = 1;

		if (libretro.environment(RETRO_ENVIRONMENT_GET_VFS_INTERFACE, &info))
			filestream_vfs_init(&info);
	}

	/* Declare subsystems. */
	{
		static const struct retro_subsystem_rom_info rom_info[] = {
			{ "IWAD", "wad", true, false,  true, NULL, 0 },
			{ "PWAD", "wad", true, false, false, NULL, 0 },
		};

		static const struct retro_subsystem_info info[] = {
			{ "Mod", "mod", rom_info, D_COUNT_OF(rom_info), 0 },
			{ NULL, NULL, NULL, 0, 0 }
		};

		libretro.environment(RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO, (void*)&info);
	}
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
	bool options_updated;

	/* TODO: Use 'libretro.audio' instead, so that this buffer is not needed. */
	int16_t audio_buffer[LIBRETRO_SAMPLE_RATE / TICRATE][LIBRETRO_CHANNEL_COUNT];

	if (libretro.environment(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &options_updated) && options_updated)
		UpdateOptions(false);

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
	size_t i;

	free(iwad_path);

	for (i = 0; i < D_COUNT_OF(pwad_paths); ++i)
		free(pwad_paths[i]);
}

unsigned int retro_get_region(void)
{
	/* Irrelevant, but `libretro.h` says that this should be used as a fallback. */
	return RETRO_REGION_NTSC;
}

bool retro_load_game_special(const unsigned int type, const struct retro_game_info* const info, const size_t num)
{
	size_t i;
	char **pwad_path;

	(void)type;

	iwad_path = M_strdup(info[0].path);

	pwad_path = pwad_paths;

	for (i = 1; i < num; ++i)
		if (info[i].path != NULL)
			*pwad_path++ = M_strdup(info[i].path);

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
/* returns time in 1/35th second tics */
int  IB_GetTime (void)
{
	return clock() / (CLOCKS_PER_SEC / TICRATE);
}


/* IB_Init */
void IB_Init (void)
{
	
}


/* IB_Quit */
void IB_Quit (int exit_code)
{
	(void)exit_code;

	game_exited = true;

	libretro.environment(RETRO_ENVIRONMENT_SHUTDOWN, NULL);
	IB_Yield();
}


void IB_WaitFrames(int count)
{
	while (count-- != 0)
		IB_Yield();
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

I_File* I_FileOpen(const char* const path, const I_FileMode mode)
{
	unsigned int vfs_mode = 0;

	switch (mode)
	{
		case I_FILE_MODE_READ:
			vfs_mode = RETRO_VFS_FILE_ACCESS_READ;
			break;

		case I_FILE_MODE_WRITE:
			vfs_mode = RETRO_VFS_FILE_ACCESS_WRITE;
			break;
	}

	return (I_File*)filestream_open(path, vfs_mode, 0);
}

void I_FileClose(I_File* const file)
{
	filestream_close((RFILE*)file);
}

size_t I_FileSize(I_File* const file)
{
	return filestream_get_size((RFILE*)file);
}

size_t I_FileRead(I_File* const file, void* const buffer, const size_t size)
{
	return filestream_read((RFILE*)file, buffer, size);
}

size_t I_FileWrite(I_File* const file, const void* const buffer, const size_t size)
{
	return filestream_write((RFILE*)file, buffer, size);
}

size_t I_FilePut(I_File* const file, const char character)
{
	return filestream_putc((RFILE*)file, character) == character ? 1 : 0;
}

size_t I_FileSeek(I_File* const file, const size_t offset, const I_FilePosition position)
{
	int vfs_position;

	switch (position)
	{
		case I_FILE_POSITION_START:
			vfs_position = RETRO_VFS_SEEK_POSITION_START;
			break;

		case I_FILE_POSITION_CURRENT:
			vfs_position = RETRO_VFS_SEEK_POSITION_CURRENT;
			break;

		case I_FILE_POSITION_END:
			vfs_position = RETRO_VFS_SEEK_POSITION_END;
			break;
	}

	return filestream_seek((RFILE*)file, offset, vfs_position);
}
