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
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

#include <string.h>

#include "SDL.h"

#include "../doomstat.h"
#include "../i_system.h"
#include "../v_video.h"
#include "../m_argv.h"
#include "../d_main.h"

#include "../doomdef.h"

#ifdef __GNUG__
#pragma implementation "../ib_video.h"
#endif
#include "../ib_video.h"


#if SDL_MAJOR_VERSION >= 2
static SDL_Window *window;
#endif
static SDL_Surface *window_surface;
static SDL_Surface *surface;

static SDL_Color colors[256];

// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
static int	multiply=1;


//
//  Translates the key currently in X_event
//

#if SDL_MAJOR_VERSION >= 2
static int xlatekey(SDL_KeyCode keysym)
#else
static int xlatekey(SDLKey keysym)
#endif
{

    int rc;

    switch(rc = keysym)
    {
      case SDLK_LEFT:	rc = KEY_LEFTARROW;	break;
      case SDLK_RIGHT:	rc = KEY_RIGHTARROW;	break;
      case SDLK_DOWN:	rc = KEY_DOWNARROW;	break;
      case SDLK_UP:	rc = KEY_UPARROW;	break;
      case SDLK_ESCAPE:	rc = KEY_ESCAPE;	break;
      case SDLK_RETURN:	rc = KEY_ENTER;		break;
      case SDLK_TAB:	rc = KEY_TAB;		break;
      case SDLK_F1:	rc = KEY_F1;		break;
      case SDLK_F2:	rc = KEY_F2;		break;
      case SDLK_F3:	rc = KEY_F3;		break;
      case SDLK_F4:	rc = KEY_F4;		break;
      case SDLK_F5:	rc = KEY_F5;		break;
      case SDLK_F6:	rc = KEY_F6;		break;
      case SDLK_F7:	rc = KEY_F7;		break;
      case SDLK_F8:	rc = KEY_F8;		break;
      case SDLK_F9:	rc = KEY_F9;		break;
      case SDLK_F10:	rc = KEY_F10;		break;
      case SDLK_F11:	rc = KEY_F11;		break;
      case SDLK_F12:	rc = KEY_F12;		break;
	
      case SDLK_BACKSPACE:
      case SDLK_DELETE:	rc = KEY_BACKSPACE;	break;

      case SDLK_PAUSE:	rc = KEY_PAUSE;		break;

      case SDLK_KP_EQUALS:
      case SDLK_EQUALS:	rc = KEY_EQUALS;	break;

      case SDLK_KP_MINUS:
      case SDLK_MINUS:	rc = KEY_MINUS;		break;

      case SDLK_LSHIFT:
      case SDLK_RSHIFT:
	rc = KEY_RSHIFT;
	break;
	
      case SDLK_LCTRL:
      case SDLK_RCTRL:
	rc = KEY_RCTRL;
	break;
	
      case SDLK_LALT:
      case SDLK_RALT:
#if SDL_MAJOR_VERSION >= 2
      case SDLK_LGUI:
      case SDLK_RGUI:
#endif
	rc = KEY_RALT;
	break;
	
      default:
	if (rc >= SDLK_SPACE && rc <= SDLK_BACKQUOTE)
	    rc = rc - SDLK_SPACE + ' ';
	else if (rc >= 'A' && rc <= 'Z')
	    rc = rc - 'A' + 'a';
	break;
    }

    return rc;

}


void IB_ShutdownGraphics(void)
{
    SDL_FreeSurface(surface);
#if SDL_MAJOR_VERSION >= 2
    SDL_DestroyWindow(window);
#endif

#if SDL_MAJOR_VERSION >= 2
    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
#else
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
#endif
}


//
// IB_StartTic
//
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
		event.data1 = xlatekey(sdl_event.key.keysym.sym);
		D_PostEvent(&event);
		// fprintf(stderr, "k");
		break;
	    case SDL_KEYUP:
		event.type = ev_keyup;
		event.data1 = xlatekey(sdl_event.key.keysym.sym);
		D_PostEvent(&event);
		// fprintf(stderr, "ku");
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
		// fprintf(stderr, "b");
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
		// fprintf(stderr, "bu");
		break;
	    case SDL_MOUSEMOTION:
		event.type = ev_mouse;
		event.data1 = button_state;
		event.data2 = sdl_event.motion.xrel << 3;
		event.data3 = sdl_event.motion.yrel << 3;

		if (event.data2 || event.data3)
		{
		    D_PostEvent(&event);
		    // fprintf(stderr, "m");
		}
		break;
	}
    }
}


//
// IB_FinishUpdate
//
void IB_FinishUpdate (void)
{
    SDL_LockSurface(surface);

    // scales the screen size before blitting it
    if (multiply == 1)
    {
	for (size_t y = 0; y < SCREENHEIGHT; ++y)
	    memcpy((unsigned char*)surface->pixels + y * surface->pitch, &screens[0][y * SCREENWIDTH], SCREENWIDTH);
    }
    else
    {
	const unsigned char *src_pointer = screens[0];

	for (size_t y = 0; y < SCREENHEIGHT; ++y)
	{
	    unsigned char *dst_row = (unsigned char*)surface->pixels + y * multiply * surface->pitch;
	    unsigned char *dst_pointer = dst_row;

	    for (size_t x = 0; x < SCREENWIDTH; ++x)
	    {
		const unsigned char pixel = *src_pointer++;

		for (int i = 0; i < multiply; ++i)
		    *dst_pointer++ = pixel;
	    }

	    for (int i = 1; i < multiply; ++i)
		memcpy(&dst_row[i * SCREENWIDTH * multiply], dst_row, SCREENWIDTH * multiply);
	}
    }

    SDL_UnlockSurface(surface);

    SDL_BlitSurface(surface, NULL, window_surface, NULL);

#if SDL_MAJOR_VERSION >= 2
    SDL_UpdateWindowSurface(window);
#else
    SDL_Flip(window_surface);
#endif
}


//
// IB_SetPalette
//
void IB_SetPalette (const byte* palette)
{
    register int	i;

    for (i=0 ; i<256 ; i++)
    {
	colors[i].r = gammatable[usegamma][*palette++];
	colors[i].g = gammatable[usegamma][*palette++];
	colors[i].b = gammatable[usegamma][*palette++];
    }

#if SDL_MAJOR_VERSION >= 2
    SDL_SetPaletteColors(surface->format->palette, colors, 0, 256);
#else
    SDL_SetPalette(surface, SDL_LOGPAL | SDL_PHYSPAL, colors, 0, 256);
#endif
}


void IB_InitGraphics(void)
{
#if SDL_MAJOR_VERSION >= 2
    SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
#else
    SDL_InitSubSystem(SDL_INIT_VIDEO);
#endif

    if (M_CheckParm("-2"))
	multiply = 2;

    if (M_CheckParm("-3"))
	multiply = 3;

    if (M_CheckParm("-4"))
	multiply = 4;

#if SDL_MAJOR_VERSION >= 2
    window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREENWIDTH * multiply, SCREENHEIGHT * multiply, 0);

    if (window == NULL)
	I_Error("Could not create SDL window");

    window_surface = SDL_GetWindowSurface(window);
#else
    window_surface = SDL_SetVideoMode(SCREENWIDTH * multiply, SCREENHEIGHT * multiply, 0, SDL_SWSURFACE);

    if (window_surface == NULL)
	I_Error("Could not create SDL window surface");
#endif

    surface = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREENWIDTH * multiply, SCREENHEIGHT * multiply, 8, 0, 0, 0, 0);

    if (surface == NULL)
	I_Error("Could not create SDL surface");
}


void IB_GrabMouse(boolean grab)
{
#if SDL_MAJOR_VERSION >= 2
    SDL_SetRelativeMouseMode(grab ? SDL_ENABLE : SDL_DISABLE);
#else
    SDL_WM_GrabInput(grab ? SDL_GRAB_ON : SDL_GRAB_OFF);
    SDL_ShowCursor(!grab);
#endif
}
