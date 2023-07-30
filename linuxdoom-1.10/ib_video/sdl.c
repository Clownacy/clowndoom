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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"

#include "../doomstat.h"
#include "../i_system.h"
#include "../v_video.h"
#include "../m_argv.h"
#include "../d_main.h"

#include "../doomdef.h"

#include "../ib_video.h"


#if SDL_MAJOR_VERSION >= 2
static SDL_Window *window;
#endif
static SDL_Surface *surface;

static void (*output_size_changed_callback)(size_t width, size_t height);

#if SDL_MAJOR_VERSION >= 2
static int SDLKeyToNative(const SDL_Keycode keycode, const SDL_Scancode scancode)
#else
static int SDLKeyToNative(const SDLKey keycode, const Uint8 scancode)
#endif
{
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

	#undef CONVERT_SCANCODE
	#undef SCANCODE_WRAPPER

		default:
			break;
	}

	return -1;
}


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
				I_Quit();
				break;

			case SDL_KEYDOWN:
				event.type = ev_keydown;
				event.data1 = SDLKeyToNative(sdl_event.key.keysym.sym, sdl_event.key.keysym.scancode);
				if (event.data1 != -1)
					D_PostEvent(&event);
				/* fprintf(stderr, "k"); */
				break;
			case SDL_KEYUP:
				event.type = ev_keyup;
				event.data1 = SDLKeyToNative(sdl_event.key.keysym.sym, sdl_event.key.keysym.scancode);
				if (event.data1 != -1)
					D_PostEvent(&event);
				/* fprintf(stderr, "ku"); */
				break;
			case SDL_MOUSEBUTTONDOWN:
				switch (sdl_event.button.button)
				{
					case SDL_BUTTON_LEFT:
						button_state |= 1;
						break;
					case SDL_BUTTON_MIDDLE:
						button_state |= 2;
						break;
					case SDL_BUTTON_RIGHT:
						button_state |= 4;
						break;
				}
				event.type = ev_mouse;
				event.data1 = button_state;
				event.data2 = event.data3 = 0;
				D_PostEvent(&event);
				/* fprintf(stderr, "b"); */
				break;
			case SDL_MOUSEBUTTONUP:
				switch (sdl_event.button.button)
				{
					case SDL_BUTTON_LEFT:
						button_state &= ~1;
						break;
					case SDL_BUTTON_MIDDLE:
						button_state &= ~2;
						break;
					case SDL_BUTTON_RIGHT:
						button_state &= ~4;
						break;
				}
				event.type = ev_mouse;
				event.data1 = button_state;
				event.data2 = event.data3 = 0;
				D_PostEvent(&event);
				/* fprintf(stderr, "bu"); */
				break;
			case SDL_MOUSEMOTION:
				event.type = ev_mouse;
				event.data1 = button_state;
				event.data2 = sdl_event.motion.xrel * (1 << 5);
				event.data3 = -sdl_event.motion.yrel * (1 << 5);

				if (event.data2 || event.data3)
				{
					D_PostEvent(&event);
					/* fprintf(stderr, "m"); */
				}
				break;
			#if SDL_MAJOR_VERSION >= 2
			case SDL_WINDOWEVENT:
				switch (sdl_event.window.event)
				{
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						surface = SDL_GetWindowSurface(window);
						output_size_changed_callback(sdl_event.window.data1, sdl_event.window.data2);
						break;
				}
				break;
			#else
			case SDL_VIDEORESIZE:
				surface = SDL_SetVideoMode(sdl_event.resize.w, sdl_event.resize.h, 32, SDL_SWSURFACE | SDL_ANYFORMAT | SDL_RESIZABLE);
				output_size_changed_callback(sdl_event.resize.w, sdl_event.resize.h);

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

	const Uint32 color = SDL_MapRGB(surface->format, red, green, blue);

	for (i = 0; i < surface->format->BytesPerPixel; ++i)
		bytes[i] = (color >> (i * 8)) & 0xFF;
}


void IB_InitGraphics(const char *title, size_t screen_width, size_t screen_height, size_t *bytes_per_pixel, void (*output_size_changed_callback_p)(size_t width, size_t height))
{
	output_size_changed_callback = output_size_changed_callback_p;
#if SDL_MAJOR_VERSION >= 2
	SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
#else
	SDL_InitSubSystem(SDL_INIT_VIDEO);
#endif

#if SDL_MAJOR_VERSION >= 2
	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

	if (window == NULL)
		I_Error("Could not create SDL window");

	surface = SDL_GetWindowSurface(window);
#else
	surface = SDL_SetVideoMode(screen_width, screen_height, 32, SDL_SWSURFACE | SDL_ANYFORMAT | SDL_RESIZABLE);

	if (surface == NULL)
		I_Error("Could not create SDL window surface");

	SDL_WM_SetCaption(title, title);
#endif

	*bytes_per_pixel = surface->format->BytesPerPixel;
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


void IB_GrabMouse(d_bool grab)
{
#if SDL_MAJOR_VERSION >= 2
	SDL_SetRelativeMouseMode(grab ? SDL_TRUE : SDL_FALSE);
#else
	SDL_WM_GrabInput(grab ? SDL_GRAB_ON : SDL_GRAB_OFF);
	SDL_ShowCursor(!grab);
#endif
}
