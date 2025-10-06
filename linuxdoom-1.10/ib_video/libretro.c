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
        DOOM graphics stuff for X11, UNIX.

******************************************************************************/

#include "../ib_video.h"

#include <stdlib.h>

#include "../ib_system/libretro-callbacks.h"

#include "../doomdef.h"
#include "../d_main.h"
#include "../d_event.h"

#define ANALOGUE_MAX 0x7FFF

typedef uint16_t Pixel;

static Pixel *framebuffer;

static int joystick_button_state, joystick_x_left, joystick_y_left, joystick_x_right, joystick_x_dpad, joystick_y_dpad;

static void SetJoystickButton(const unsigned int button_index, const d_bool pressed)
{
	const unsigned int mask = 1U << button_index;

	if (pressed)
		joystick_button_state |= mask;
	else
		joystick_button_state &= ~mask;
}

static void SubmitJoystickEvent(void)
{
	event_t event;
	event.type = ev_joystick;
	event.data1 = joystick_button_state;
	event.data2 = D_CLAMP(-ANALOGUE_MAX, ANALOGUE_MAX, joystick_x_left + joystick_x_dpad);
	event.data3 = D_CLAMP(-ANALOGUE_MAX, ANALOGUE_MAX, joystick_y_left + joystick_y_dpad);
	event.data4 = joystick_x_right;
	D_PostEvent(&event);
}

static int16_t GetJoypadButton(unsigned int button_id)
{
	return libretro.input_state(0, RETRO_DEVICE_JOYPAD, 0, button_id);
}

static void DoJoypadInput(unsigned int button_index, unsigned int button_id)
{
	SetJoystickButton(button_index, GetJoypadButton(button_id));
}

static int16_t ApplyDeadzone(const int16_t value)
{
	const int16_t threshold = ANALOGUE_MAX / 0x10;

	if (value < threshold && value > -threshold)
		return 0;

	return value;
}

/* IB_StartTic */
void IB_StartTic (void)
{
	DoJoypadInput(0, RETRO_DEVICE_ID_JOYPAD_R);
	DoJoypadInput(1, RETRO_DEVICE_ID_JOYPAD_L);
	DoJoypadInput(2, RETRO_DEVICE_ID_JOYPAD_Y);
	DoJoypadInput(3, RETRO_DEVICE_ID_JOYPAD_B);
	DoJoypadInput(4, RETRO_DEVICE_ID_JOYPAD_X);
	DoJoypadInput(5, RETRO_DEVICE_ID_JOYPAD_A);
	DoJoypadInput(6, RETRO_DEVICE_ID_JOYPAD_START);
	DoJoypadInput(7, RETRO_DEVICE_ID_JOYPAD_SELECT);
	DoJoypadInput(8, RETRO_DEVICE_ID_JOYPAD_L3);
	DoJoypadInput(9, RETRO_DEVICE_ID_JOYPAD_R3);
	DoJoypadInput(10, RETRO_DEVICE_ID_JOYPAD_L2);
	DoJoypadInput(11, RETRO_DEVICE_ID_JOYPAD_R2);

	joystick_x_dpad = 0;
	if (GetJoypadButton(RETRO_DEVICE_ID_JOYPAD_LEFT))
		joystick_x_dpad -= ANALOGUE_MAX;
	if (GetJoypadButton(RETRO_DEVICE_ID_JOYPAD_RIGHT))
		joystick_x_dpad += ANALOGUE_MAX;

	joystick_y_dpad = 0;
	if (GetJoypadButton(RETRO_DEVICE_ID_JOYPAD_UP))
		--joystick_y_dpad;
	if (GetJoypadButton(RETRO_DEVICE_ID_JOYPAD_DOWN))
		++joystick_y_dpad;

	joystick_x_left  = ApplyDeadzone(libretro.input_state(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT,  RETRO_DEVICE_ID_ANALOG_X));
	joystick_y_left  = ApplyDeadzone(libretro.input_state(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT,  RETRO_DEVICE_ID_ANALOG_Y));
	joystick_x_right = ApplyDeadzone(libretro.input_state(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X));

	SubmitJoystickEvent();
}


void IB_GetFramebuffer(unsigned char **pixels, size_t *pitch)
{
	*pixels = (unsigned char*)framebuffer;
	*pitch = SCREENWIDTH * sizeof(*framebuffer);
}


/* IB_FinishUpdate */
void IB_FinishUpdate (void)
{
	libretro.video(framebuffer, SCREENWIDTH, SCREENHEIGHT, SCREENWIDTH * sizeof(*framebuffer));

	IB_Yield();
}


static unsigned int Get5BitColourChannel(const unsigned int channel, const unsigned int index)
{
	const unsigned int bits_per_source_colour_channel = 8;
	const unsigned int bits_per_destination_colour_channel = 5;
	const unsigned int colour_shift = bits_per_source_colour_channel - bits_per_destination_colour_channel;
	const unsigned int colour_mask = (1 << bits_per_destination_colour_channel) - 1;

	return ((channel >> colour_shift) & colour_mask) << (bits_per_destination_colour_channel * index);
}

void IB_GetColor(unsigned char* const bytes, const unsigned char red, const unsigned char green, const unsigned char blue)
{
	const Pixel pixel = Get5BitColourChannel(red,   2)
	                  | Get5BitColourChannel(green, 1)
	                  | Get5BitColourChannel(blue,  0);

	/* Do this trick to write the pixel in native-endian byte ordering, as required by the libretro API. */
	*(Pixel*)bytes = pixel;
}

void IB_InitGraphics(const char *title, size_t screen_width, size_t screen_height, size_t *bytes_per_pixel, void (*output_size_changed_callback)(size_t width, size_t height, d_bool aspect_ratio_correction))
{
	(void)title;

	/* libretro will handle scaling the framebuffer, so we are ignoring these. */
	(void)screen_width;
	(void)screen_height;

	/* TODO: Handle failed allocation. */
	framebuffer = (Pixel*)malloc(SCREENWIDTH * SCREENHEIGHT * sizeof(*framebuffer));

	*bytes_per_pixel = sizeof(Pixel);

	output_size_changed_callback(SCREENWIDTH, SCREENHEIGHT, d_false);

	/* Update the frontend with the game's chosen resolution. */
	{
		struct retro_game_geometry geometry;
		geometry.base_width = SCREENWIDTH;
		geometry.base_height = SCREENHEIGHT;
		geometry.aspect_ratio = (float)SCREENWIDTH / SCREENHEIGHT;
		if (aspect_ratio_correction)
			geometry.aspect_ratio = geometry.aspect_ratio * 5 / 6;
		libretro.environment(RETRO_ENVIRONMENT_SET_GEOMETRY, &geometry);
	}
}


void IB_ShutdownGraphics(void)
{
	free(framebuffer);
}


void IB_GrabMouse(d_bool grab)
{
	(void)grab;
}

void IB_ToggleFullscreen(void)
{
	
}
