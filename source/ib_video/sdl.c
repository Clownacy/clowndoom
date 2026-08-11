/******************************************************************************

   Copyright (C) 1993-1996 by id Software, Inc.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 3
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   DESCRIPTION:
        DOOM graphics stuff for X11, UNIX.

******************************************************************************/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_SDL3
	#include <SDL3/SDL.h>
#else
	#include "SDL.h"
#endif

#include "../doomstat.h"
#include "../i_system.h"
#include "../v_video.h"
#include "../m_argv.h"
#include "../m_misc.h"
#include "../d_main.h"

#include "../doomdef.h"

#include "../ib_video.h"

#if SDL_MAJOR_VERSION >= 3
	#undef SDL_QUIT
	#define SDL_QUIT SDL_EVENT_QUIT
	#undef SDL_KEYDOWN
	#define SDL_KEYDOWN SDL_EVENT_KEY_DOWN
	#undef SDL_KEYUP
	#define SDL_KEYUP SDL_EVENT_KEY_UP
	#undef SDL_MOUSEBUTTONDOWN
	#define SDL_MOUSEBUTTONDOWN SDL_EVENT_MOUSE_BUTTON_DOWN
	#undef SDL_MOUSEBUTTONUP
	#define SDL_MOUSEBUTTONUP SDL_EVENT_MOUSE_BUTTON_UP
	#undef SDL_MOUSEMOTION
	#define SDL_MOUSEMOTION SDL_EVENT_MOUSE_MOTION
	#undef SDL_CONTROLLERBUTTONDOWN
	#define SDL_CONTROLLERBUTTONDOWN SDL_EVENT_GAMEPAD_BUTTON_DOWN
	#undef SDL_CONTROLLERBUTTONUP
	#define SDL_CONTROLLERBUTTONUP SDL_EVENT_GAMEPAD_BUTTON_UP
	#undef SDL_CONTROLLERAXISMOTION
	#define SDL_CONTROLLERAXISMOTION SDL_EVENT_GAMEPAD_AXIS_MOTION
	#undef SDL_WINDOWEVENT_SIZE_CHANGED
	#define SDL_WINDOWEVENT_SIZE_CHANGED SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED

	#undef SDL_FillRect
	#define SDL_FillRect SDL_FillSurfaceRect
	#undef SDL_FreeSurface
	#define SDL_FreeSurface SDL_DestroySurface

	#undef SDL_CONTROLLER_BUTTON_DPAD_UP
	#define SDL_CONTROLLER_BUTTON_DPAD_UP SDL_GAMEPAD_BUTTON_DPAD_UP
	#undef SDL_CONTROLLER_BUTTON_DPAD_DOWN
	#define SDL_CONTROLLER_BUTTON_DPAD_DOWN SDL_GAMEPAD_BUTTON_DPAD_DOWN
	#undef SDL_CONTROLLER_BUTTON_DPAD_LEFT
	#define SDL_CONTROLLER_BUTTON_DPAD_LEFT SDL_GAMEPAD_BUTTON_DPAD_LEFT
	#undef SDL_CONTROLLER_BUTTON_DPAD_RIGHT
	#define SDL_CONTROLLER_BUTTON_DPAD_RIGHT SDL_GAMEPAD_BUTTON_DPAD_RIGHT
	#undef SDL_CONTROLLER_BUTTON_Y
	#define SDL_CONTROLLER_BUTTON_Y SDL_GAMEPAD_BUTTON_NORTH
	#undef SDL_CONTROLLER_BUTTON_A
	#define SDL_CONTROLLER_BUTTON_A SDL_GAMEPAD_BUTTON_SOUTH
	#undef SDL_CONTROLLER_BUTTON_X
	#define SDL_CONTROLLER_BUTTON_X SDL_GAMEPAD_BUTTON_WEST
	#undef SDL_CONTROLLER_BUTTON_B
	#define SDL_CONTROLLER_BUTTON_B SDL_GAMEPAD_BUTTON_EAST
	#undef SDL_CONTROLLER_BUTTON_START
	#define SDL_CONTROLLER_BUTTON_START SDL_GAMEPAD_BUTTON_START
	#undef SDL_CONTROLLER_BUTTON_BACK
	#define SDL_CONTROLLER_BUTTON_BACK SDL_GAMEPAD_BUTTON_BACK
	#undef SDL_CONTROLLER_BUTTON_LEFTSHOULDER
	#define SDL_CONTROLLER_BUTTON_LEFTSHOULDER SDL_GAMEPAD_BUTTON_LEFT_SHOULDER
	#undef SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
	#define SDL_CONTROLLER_BUTTON_RIGHTSHOULDER SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER
	#undef SDL_CONTROLLER_BUTTON_LEFTSTICK
	#define SDL_CONTROLLER_BUTTON_LEFTSTICK SDL_GAMEPAD_BUTTON_LEFT_STICK
	#undef SDL_CONTROLLER_BUTTON_RIGHTSTICK
	#define SDL_CONTROLLER_BUTTON_RIGHTSTICK SDL_GAMEPAD_BUTTON_RIGHT_STICK

	#undef SDL_CONTROLLER_AXIS_LEFTX
	#define SDL_CONTROLLER_AXIS_LEFTX SDL_GAMEPAD_AXIS_LEFTX
	#undef SDL_CONTROLLER_AXIS_LEFTY
	#define SDL_CONTROLLER_AXIS_LEFTY SDL_GAMEPAD_AXIS_LEFTY
	#undef SDL_CONTROLLER_AXIS_RIGHTX
	#define SDL_CONTROLLER_AXIS_RIGHTX SDL_GAMEPAD_AXIS_RIGHTX
	#undef SDL_CONTROLLER_AXIS_RIGHTY
	#define SDL_CONTROLLER_AXIS_RIGHTY SDL_GAMEPAD_AXIS_RIGHTY
	#undef SDL_CONTROLLER_AXIS_TRIGGERLEFT
	#define SDL_CONTROLLER_AXIS_TRIGGERLEFT SDL_GAMEPAD_AXIS_LEFT_TRIGGER
	#undef SDL_CONTROLLER_AXIS_TRIGGERRIGHT
	#define SDL_CONTROLLER_AXIS_TRIGGERRIGHT SDL_GAMEPAD_AXIS_RIGHT_TRIGGER

	#undef SDL_WINDOW_ALLOW_HIGHDPI
	#define SDL_WINDOW_ALLOW_HIGHDPI SDL_WINDOW_HIGH_PIXEL_DENSITY

	#undef SDL_TRUE
	#define SDL_TRUE true
	#undef SDL_FALSE
	#define SDL_FALSE false
#endif

#if SDL_MAJOR_VERSION >= 2
static SDL_Window *window;
#endif
static SDL_Surface *surface;

static IB_OutputSizeChangedCallback output_size_changed_callback;

static int SDLKeyToNative(const SDL_Event* const event)
{
#if SDL_MAJOR_VERSION >= 3
	const SDL_Keycode keycode = event->key.key;
	const SDL_Scancode scancode = event->key.scancode;
#elif SDL_MAJOR_VERSION >= 2
	const SDL_Keycode keycode = event->key.keysym.sym;
	const SDL_Scancode scancode = event->key.keysym.scancode;
#else
	const SDLKey keycode = event->key.keysym.sym;
	const Uint8 scancode = event->key.keysym.scancode;
#endif

	switch (keycode)
	{
		case SDLK_LEFT:   return KEY_LEFTARROW;
		case SDLK_RIGHT:  return KEY_RIGHTARROW;
		case SDLK_DOWN:   return KEY_DOWNARROW;
		case SDLK_UP:     return KEY_UPARROW;
		case SDLK_ESCAPE: return KEY_ESCAPE;
		case SDLK_RETURN: return KEY_ENTER;
		case SDLK_TAB:    return KEY_TAB;
		case SDLK_F1:     return KEY_F1;
		case SDLK_F2:     return KEY_F2;
		case SDLK_F3:     return KEY_F3;
		case SDLK_F4:     return KEY_F4;
		case SDLK_F5:     return KEY_F5;
		case SDLK_F6:     return KEY_F6;
		case SDLK_F7:     return KEY_F7;
		case SDLK_F8:     return KEY_F8;
		case SDLK_F9:     return KEY_F9;
		case SDLK_F10:    return KEY_F10;
		case SDLK_F11:    return KEY_F11;
		case SDLK_F12:    return KEY_F12;

		case SDLK_BACKSPACE:
		case SDLK_DELETE: return KEY_BACKSPACE;

		case SDLK_PAUSE:  return KEY_PAUSE;

		case SDLK_LSHIFT:
		case SDLK_RSHIFT: return KEY_RSHIFT;

		case SDLK_LCTRL:
		case SDLK_RCTRL:  return KEY_RCTRL;

		case SDLK_LALT:
		case SDLK_RALT:
#if SDL_MAJOR_VERSION >= 2
		case SDLK_LGUI:
		case SDLK_RGUI:
#else
		case SDLK_LMETA:
		case SDLK_RMETA:
#endif
			return KEY_RALT;

		default:
			break;
	}

	switch(scancode)
	{
	#if SDL_MAJOR_VERSION >= 2
		#define SCANCODE_WRAPPER(sdl2, linux, macos) SDL_SCANCODE_##sdl2
	#elif defined(_WIN32)
		#define SCANCODE_WRAPPER(sdl2, linux, macos) linux - 8
	#elif defined(__MACOSX__)
		#define SCANCODE_WRAPPER(sdl2, linux, macos) macos
	#else
		#define SCANCODE_WRAPPER(sdl2, linux, macos) linux
	#endif
		/* These magic numbers are taken from 'sdl12-compat':
		   https://github.com/libsdl-org/sdl12-compat/blob/b000fdc51a7543f4067fa45dbe402ace6c738ba6/src/SDL12_compat.c#L4291 */
		case SCANCODE_WRAPPER(EQUALS      , 0x15, 0x18): return KEY_EQUALS;
		case SCANCODE_WRAPPER(MINUS       , 0x14, 0x1B): return KEY_MINUS;
		case SCANCODE_WRAPPER(SPACE       , 0x41, 0x31): return ' ';
		case SCANCODE_WRAPPER(LEFTBRACKET , 0x22, 0x21): return '[';
		case SCANCODE_WRAPPER(RIGHTBRACKET, 0x23, 0x1E): return ']';
		case SCANCODE_WRAPPER(BACKSLASH   , 0x33, 0x2A): return '\\';
		case SCANCODE_WRAPPER(SEMICOLON   , 0x2F, 0x29): return ';';
		case SCANCODE_WRAPPER(APOSTROPHE  , 0x30, 0x27): return '\'';
		case SCANCODE_WRAPPER(GRAVE       , 0x31, 0x32): return '`';
		case SCANCODE_WRAPPER(COMMA       , 0x3B, 0x2B): return ',';
		case SCANCODE_WRAPPER(PERIOD      , 0x3C, 0x2F): return '.';
		case SCANCODE_WRAPPER(SLASH       , 0x3D, 0x2C): return '/';
		case SCANCODE_WRAPPER(0, 0x13, 0x1D): return '0';
		case SCANCODE_WRAPPER(1, 0x0A, 0x12): return '1';
		case SCANCODE_WRAPPER(2, 0x0B, 0x13): return '2';
		case SCANCODE_WRAPPER(3, 0x0C, 0x14): return '3';
		case SCANCODE_WRAPPER(4, 0x0D, 0x15): return '4';
		case SCANCODE_WRAPPER(5, 0x0E, 0x16): return '5';
		case SCANCODE_WRAPPER(6, 0x0F, 0x17): return '6';
		case SCANCODE_WRAPPER(7, 0x10, 0x18): return '7';
		case SCANCODE_WRAPPER(8, 0x11, 0x19): return '8';
		case SCANCODE_WRAPPER(9, 0x12, 0x1A): return '9';
		case SCANCODE_WRAPPER(A, 0x26, 0x00): return 'a';
		case SCANCODE_WRAPPER(B, 0x38, 0x0B): return 'b';
		case SCANCODE_WRAPPER(C, 0x36, 0x08): return 'c';
		case SCANCODE_WRAPPER(D, 0x28, 0x02): return 'd';
		case SCANCODE_WRAPPER(E, 0x1A, 0x0E): return 'e';
		case SCANCODE_WRAPPER(F, 0x29, 0x03): return 'f';
		case SCANCODE_WRAPPER(G, 0x2A, 0x05): return 'g';
		case SCANCODE_WRAPPER(H, 0x2B, 0x04): return 'h';
		case SCANCODE_WRAPPER(I, 0x1F, 0x22): return 'i';
		case SCANCODE_WRAPPER(J, 0x2C, 0x26): return 'j';
		case SCANCODE_WRAPPER(K, 0x2D, 0x28): return 'k';
		case SCANCODE_WRAPPER(L, 0x2E, 0x25): return 'l';
		case SCANCODE_WRAPPER(M, 0x3A, 0x2E): return 'm';
		case SCANCODE_WRAPPER(N, 0x39, 0x2D): return 'n';
		case SCANCODE_WRAPPER(O, 0x20, 0x1F): return 'o';
		case SCANCODE_WRAPPER(P, 0x21, 0x23): return 'p';
		case SCANCODE_WRAPPER(Q, 0x18, 0x0C): return 'q';
		case SCANCODE_WRAPPER(R, 0x1B, 0x0F): return 'r';
		case SCANCODE_WRAPPER(S, 0x27, 0x01): return 's';
		case SCANCODE_WRAPPER(T, 0x1C, 0x11): return 't';
		case SCANCODE_WRAPPER(U, 0x1E, 0x20): return 'u';
		case SCANCODE_WRAPPER(V, 0x37, 0x09): return 'v';
		case SCANCODE_WRAPPER(W, 0x19, 0x0D): return 'w';
		case SCANCODE_WRAPPER(X, 0x35, 0x07): return 'x';
		case SCANCODE_WRAPPER(Y, 0x1D, 0x10): return 'y';
		case SCANCODE_WRAPPER(Z, 0x34, 0x06): return 'z';

	#undef SCANCODE_WRAPPER

		default:
			break;
	}

	return -1;
}

#if SDL_MAJOR_VERSION >= 2
static int joystick_button_state, joystick_x_left, joystick_y_left, joystick_x_right, joystick_x_dpad, joystick_y_dpad;

static void SetJoystickButton(const unsigned int button_index, const cc_bool pressed)
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
	event.data2 = SDL_clamp(joystick_x_left + joystick_x_dpad, -0x7FFF, 0x7FFF);
	event.data3 = SDL_clamp(joystick_y_left + joystick_y_dpad, -0x7FFF, 0x7FFF);
	event.data4 = joystick_x_right;
	D_PostJoystickEvent(&event);
}
#endif

/* IB_StartTic */
void IB_StartTic (void)
{
	event_t event;
	static int button_state;
	SDL_Event sdl_event;

	while (SDL_PollEvent(&sdl_event))
	{
		switch (sdl_event.type)
		{
			case SDL_QUIT:
				I_Quit(0);
				break;

			case SDL_KEYDOWN:
			#if SDL_MAJOR_VERSION >= 2
				/* Ignore repeat key-presses. */
				if (sdl_event.key.repeat)
					break;
			#endif

				event.type = ev_keydown;
				event.data1 = SDLKeyToNative(&sdl_event);
				if (event.data1 != -1)
					D_PostEvent(&event);
				/* I_Info("k"); */
				break;
			case SDL_KEYUP:
				event.type = ev_keyup;
				event.data1 = SDLKeyToNative(&sdl_event);
				if (event.data1 != -1)
					D_PostEvent(&event);
				/* I_Info("ku"); */
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			{
				int button_index;
				const cc_bool button_down = sdl_event.type == SDL_MOUSEBUTTONDOWN;

				button_index = -1;

				switch (sdl_event.button.button)
				{
					default:
						break;
					case SDL_BUTTON_LEFT:
						button_index = 0;
						break;
					case SDL_BUTTON_RIGHT:
						button_index = 1;
						break;
					case SDL_BUTTON_MIDDLE:
						button_index = 2;
						break;
				}

				if (button_index != -1)
				{
					const unsigned int mask = 1U << button_index;

					if (button_down)
						button_state |= mask;
					else
						button_state &= ~mask;
				}

				event.type = ev_mouse;
				event.data1 = button_state;
				event.data2 = event.data3 = 0;
				D_PostEvent(&event);
				/* I_Info(button_down ? "b" : "bu"); */
				break;
			}
			case SDL_MOUSEMOTION:
				event.type = ev_mouse;
				event.data1 = button_state;
				event.data2 = sdl_event.motion.xrel * (1 << 5);
				event.data3 = -sdl_event.motion.yrel * (1 << 5);

				if (event.data2 || event.data3)
				{
					D_PostEvent(&event);
					/* I_Info("m"); */
				}
				break;
			#if SDL_MAJOR_VERSION >= 3
			case SDL_EVENT_GAMEPAD_ADDED:
				SDL_OpenGamepad(sdl_event.gdevice.which);
				break;
			case SDL_EVENT_GAMEPAD_REMOVED:
				SDL_CloseGamepad(SDL_GetGamepadFromID(sdl_event.gdevice.which));
				break;
			#elif SDL_MAJOR_VERSION >= 2
			case SDL_CONTROLLERDEVICEADDED:
				SDL_GameControllerOpen(sdl_event.cdevice.which);
				break;
			case SDL_CONTROLLERDEVICEREMOVED:
				SDL_GameControllerClose(SDL_GameControllerFromInstanceID(sdl_event.cdevice.which));
				break;
			#endif
			#if SDL_MAJOR_VERSION >= 2
			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
			{
				int button_index, x_delta, y_delta;

				const cc_bool button_down = sdl_event.type == SDL_CONTROLLERBUTTONDOWN;

				button_index = -1;
				x_delta = y_delta = 0;

			#if SDL_MAJOR_VERSION >= 3
				switch (sdl_event.gbutton.button)
			#else
				switch (sdl_event.cbutton.button)
			#endif
				{
					default:
						break;
					case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
						button_index = 0;
						break;
					case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
						button_index = 1;
						break;
					case SDL_CONTROLLER_BUTTON_X:
						button_index = 2;
						break;
					case SDL_CONTROLLER_BUTTON_A:
						button_index = 3;
						break;
					case SDL_CONTROLLER_BUTTON_Y:
						button_index = 4;
						break;
					case SDL_CONTROLLER_BUTTON_B:
						button_index = 5;
						break;
					case SDL_CONTROLLER_BUTTON_START:
						button_index = 6;
						break;
					case SDL_CONTROLLER_BUTTON_BACK:
						button_index = 7;
						break;
					case SDL_CONTROLLER_BUTTON_LEFTSTICK:
						button_index = 8;
						break;
					case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
						button_index = 9;
						break;
					case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
						x_delta = -1;
						break;
					case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
						x_delta = 1;
						break;
					case SDL_CONTROLLER_BUTTON_DPAD_UP:
						y_delta = -1;
						break;
					case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
						y_delta = 1;
						break;
				}

				if (!button_down)
				{
					x_delta = -x_delta;
					y_delta = -y_delta;
				}

				joystick_x_dpad += x_delta;
				joystick_y_dpad += y_delta;

				if (button_index != -1)
					SetJoystickButton(button_index, button_down);

				SubmitJoystickEvent();
				/* I_Info(button_down ? "j" : "ju"); */
				break;
			}
			case SDL_CONTROLLERAXISMOTION:
			{
			#if SDL_MAJOR_VERSION >= 3
				SDL_GamepadAxisEvent* const axis = &sdl_event.gaxis;
			#else
				SDL_ControllerAxisEvent* const axis = &sdl_event.caxis;
			#endif
				unsigned long value = axis->value;

				switch (axis->axis)
				{
					case SDL_CONTROLLER_AXIS_LEFTX:
					case SDL_CONTROLLER_AXIS_LEFTY:
						/* Deadzone. */
						if (SDL_abs(value) < 0x7FFF / 4)
							value = 0;

						if (axis->axis == SDL_CONTROLLER_AXIS_LEFTX)
							joystick_x_left = value;
						else
							joystick_y_left = value;

						break;

					case SDL_CONTROLLER_AXIS_RIGHTX:
						value *= value;
						value /= 0x8000;

						/* Deadzone. */
						if (value < 0x7FFF / 0x10)
							value = 0;

						if (axis->value < 0)
							value = -value;

						joystick_x_right = value;

						break;

					case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
					case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
						/* Deadzone. */
						SetJoystickButton(axis->axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT ? 10 : 11, value >= 0x7FFF / 4);
						break;
				}

				SubmitJoystickEvent();
				break;
			}
			#if SDL_MAJOR_VERSION == 2
			case SDL_WINDOWEVENT:
				switch (sdl_event.window.event)
				{
			#endif
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						surface = SDL_GetWindowSurface(window);
						/* Clear surface of any garbage pixels. */
						SDL_FillRect(surface, NULL, 0);
						output_size_changed_callback(surface->w, surface->h, aspect_ratio_correction);
						break;
			#if SDL_MAJOR_VERSION == 2
				}
				break;
			#endif
			#else
			case SDL_VIDEORESIZE:
				surface = SDL_SetVideoMode(sdl_event.resize.w, sdl_event.resize.h, 32, SDL_SWSURFACE | SDL_ANYFORMAT | SDL_RESIZABLE);
				output_size_changed_callback(sdl_event.resize.w, sdl_event.resize.h, aspect_ratio_correction);

				if (surface == NULL)
					I_Error("Could not create SDL window surface");

				break;
			#endif
		}
	}
}


void IB_GetFramebuffer(unsigned char **pixels, size_t *pitch)
{
	SDL_LockSurface(surface);

	*pixels = (unsigned char*)surface->pixels;
	*pitch = surface->pitch;
}


/* IB_FinishUpdate */
void IB_FinishUpdate (void)
{
	SDL_UnlockSurface(surface);

#if SDL_MAJOR_VERSION >= 2
	SDL_UpdateWindowSurface(window);
#else
	SDL_Flip(surface);
#endif
}


void IB_GetColor(unsigned char *bytes, unsigned char red, unsigned char green, unsigned char blue)
{
	unsigned int i;

#if SDL_MAJOR_VERSION >= 3
	const SDL_PixelFormatDetails* format = SDL_GetPixelFormatDetails(surface->format);
	const Uint8 bytes_per_pixel = format->bytes_per_pixel;
	const Uint32 color = SDL_MapRGB(format, NULL, red, green, blue);
#else
	const Uint8 bytes_per_pixel = surface->format.BytesPerPixel;
	const Uint32 color = SDL_MapRGB(surface->format, red, green, blue);
#endif
	for (i = 0; i < bytes_per_pixel; ++i)
		bytes[i] = (color >> (i * 8)) & 0xFF;
}


void IB_InitGraphics(const char *title, size_t screen_width, size_t screen_height, size_t *bytes_per_pixel, IB_OutputSizeChangedCallback output_size_changed_callback_p)
{
	output_size_changed_callback = output_size_changed_callback_p;
#if SDL_MAJOR_VERSION >= 3
	SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD);
#elif SDL_MAJOR_VERSION >= 2
#ifdef _WIN32
	/* Enable high-DPI support on Windows because SDL2 is bad at being a platform abstraction library. */
	SDL_SetHint(SDL_HINT_WINDOWS_DPI_SCALING, "1");
#endif

	SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER);
#else
	SDL_InitSubSystem(SDL_INIT_VIDEO);

	/* Disable repeated key inputs, since there's no way to filter them out in SDL1. */
	SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
#endif

#if SDL_MAJOR_VERSION >= 2
	window = SDL_CreateWindow(title,
#if SDL_MAJOR_VERSION <= 2
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
#endif
		screen_width, screen_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | (M_CheckParm("-full") ? SDL_WINDOW_FULLSCREEN : 0));

	if (window == NULL)
		I_Error("Could not create SDL window");

	surface = SDL_GetWindowSurface(window);
#else
	surface = SDL_SetVideoMode(screen_width, screen_height, 32, SDL_SWSURFACE | SDL_ANYFORMAT | SDL_RESIZABLE);

	if (surface == NULL)
		I_Error("Could not create SDL window surface");

	SDL_WM_SetCaption(title, title);
#endif

#if SDL_MAJOR_VERSION >= 3
	*bytes_per_pixel = SDL_GetPixelFormatDetails(surface->format)->bytes_per_pixel;
#else
	*bytes_per_pixel = surface->format->BytesPerPixel;
#endif

	output_size_changed_callback(surface->w, surface->h, aspect_ratio_correction);
}


void IB_ShutdownGraphics(void)
{
#if SDL_MAJOR_VERSION >= 2
	SDL_FreeSurface(surface);
	SDL_DestroyWindow(window);
#endif

#if SDL_MAJOR_VERSION >= 2
	SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
#else
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
#endif
}


void IB_GrabMouse(cc_bool grab)
{
#if SDL_MAJOR_VERSION >= 3
	SDL_SetWindowRelativeMouseMode(window, grab);
#elif SDL_MAJOR_VERSION >= 2
	SDL_SetRelativeMouseMode(grab ? SDL_TRUE : SDL_FALSE);
#else
	SDL_WM_GrabInput(grab ? SDL_GRAB_ON : SDL_GRAB_OFF);
	SDL_ShowCursor(!grab);
#endif
}

void IB_ToggleFullscreen(void)
{
#if SDL_MAJOR_VERSION >= 2
	static cc_bool fullscreen;

	fullscreen = !fullscreen;
#if SDL_MAJOR_VERSION >= 3
	SDL_SetWindowFullscreen(window, fullscreen);
#else
	SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
#endif
#else
	SDL_WM_ToggleFullScreen(surface);
#endif
}
