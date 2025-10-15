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

#include <assert.h>
#include <stdlib.h>

#include "../ib_system/libretro-callbacks.h"

#include "../doomdef.h"
#include "../d_main.h"
#include "../d_event.h"
#include "../m_misc.h"

#define ANALOGUE_MAX 0x7FFF

typedef uint16_t Pixel16;
typedef uint32_t Pixel32;

static void *framebuffer;

static int joystick_button_state, joystick_x_left, joystick_y_left, joystick_x_right, joystick_x_dpad, joystick_y_dpad;
static int mouse_button_state;

static void SetButton(const unsigned int button_index, const d_bool pressed, int* const state)
{
	const unsigned int mask = 1U << button_index;

	if (pressed)
		*state |= mask;
	else
		*state &= ~mask;
}

static void SetJoystickButton(const unsigned int button_index, const d_bool pressed)
{
	SetButton(button_index, pressed, &joystick_button_state);
}

static void SetMouseButton(const unsigned int button_index, const d_bool pressed)
{
	SetButton(button_index, pressed, &mouse_button_state);
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

static void DoMouseInput(unsigned int button_index, unsigned int button_id)
{
	SetMouseButton(button_index, libretro.input_state(0, RETRO_DEVICE_MOUSE, 0, button_id));
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

	DoMouseInput(0, RETRO_DEVICE_ID_MOUSE_LEFT);
	DoMouseInput(1, RETRO_DEVICE_ID_MOUSE_RIGHT);
	DoMouseInput(2, RETRO_DEVICE_ID_MOUSE_MIDDLE);

	{
		event_t event;
		event.type = ev_mouse;
		event.data1 = mouse_button_state;
		event.data2 =  libretro.input_state(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X) * (1 << 3);
		event.data3 = -libretro.input_state(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y) * (1 << 3);
		D_PostEvent(&event);
	}
}


static unsigned int GetPixelSize(void)
{
	switch (libretro.framebuffer_format)
	{
		default:
			assert(false);
			/* Fallthrough */
		case RETRO_PIXEL_FORMAT_0RGB1555:
		case RETRO_PIXEL_FORMAT_RGB565:
			return sizeof(Pixel16);

		case RETRO_PIXEL_FORMAT_XRGB8888:
			return sizeof(Pixel32);
	}
}


void IB_GetFramebuffer(unsigned char **pixels, size_t *pitch)
{
	*pixels = (unsigned char*)framebuffer;
	*pitch = SCREENWIDTH * GetPixelSize();
}


/* IB_FinishUpdate */
void IB_FinishUpdate (void)
{
	libretro.video(framebuffer, SCREENWIDTH, SCREENHEIGHT, SCREENWIDTH * GetPixelSize());

	IB_Yield();
}


static unsigned int TruncateColourChannel(const unsigned int channel, const unsigned int bits_per_destination_colour_channel)
{
	const unsigned int bits_per_source_colour_channel = 8;
	const unsigned int colour_shift = bits_per_source_colour_channel - bits_per_destination_colour_channel;
	const unsigned int colour_mask = (1 << bits_per_destination_colour_channel) - 1;

	return (channel >> colour_shift) & colour_mask;
}


void IB_GetColor(unsigned char* const bytes, const unsigned char red, const unsigned char green, const unsigned char blue)
{
	switch (libretro.framebuffer_format)
	{
		default:
			assert(false);
			/* Fallthrough */
		case RETRO_PIXEL_FORMAT_0RGB1555:
		{
			const Pixel16 pixel = TruncateColourChannel(red,   5) << (5 * 2)
			                    | TruncateColourChannel(green, 5) << (5 * 1)
			                    | TruncateColourChannel(blue,  5) << (5 * 0);

			/* Do this trick to write the pixel in native-endian byte ordering, as required by the libretro API. */
			*(Pixel16*)bytes = pixel;
			break;
		}

		case RETRO_PIXEL_FORMAT_RGB565:
		{
			const Pixel16 pixel = TruncateColourChannel(red,   5) << (0 + 5 + 6)
			                    | TruncateColourChannel(green, 6) << (0 + 5)
			                    | TruncateColourChannel(blue,  5) << (0);

			/* Do this trick to write the pixel in native-endian byte ordering, as required by the libretro API. */
			*(Pixel16*)bytes = pixel;
			break;
		}

		case RETRO_PIXEL_FORMAT_XRGB8888:
		{
			const Pixel32 pixel = TruncateColourChannel(red,   8) << (8 * 2)
			                    | TruncateColourChannel(green, 8) << (8 * 1)
			                    | TruncateColourChannel(blue,  8) << (8 * 0);

			/* Do this trick to write the pixel in native-endian byte ordering, as required by the libretro API. */
			*(Pixel32*)bytes = pixel;
			break;
		}
	}
}

void IB_InitGraphics(const char *title, size_t screen_width, size_t screen_height, size_t *bytes_per_pixel, const IB_OutputSizeChangedCallback output_size_changed_callback)
{
	(void)title;

	/* libretro will handle scaling the framebuffer, so we are ignoring these. */
	(void)screen_width;
	(void)screen_height;

	*bytes_per_pixel = GetPixelSize();

	/* TODO: Handle failed allocation. */
	framebuffer = malloc(SCREENWIDTH * SCREENHEIGHT * *bytes_per_pixel);

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
