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

#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <X11/extensions/XShm.h>

#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <errno.h>
#include <signal.h>

#include "../doomstat.h"
#include "../i_system.h"
#include "../i_video.h"
#include "../v_video.h"
#include "../m_argv.h"
#include "../d_main.h"

#include "../doomdef.h"

static Display* X_display=0;
static Window           X_mainWindow;
static Visual*          X_visual;
static GC               X_gc;
static XEvent           X_event;
static int              X_screen;
static XVisualInfo      X_visualinfo;
static XImage*          image;
static int              X_width;
static int              X_height;

/* MIT SHared Memory extension. */
static d_bool          doShm;

static XShmSegmentInfo  X_shminfo;
static int              X_shmeventtype;

/* Fake mouse handling. */
/* This cannot work properly w/o DGA. */
/* Needs an invisible mouse cursor at least. */
static d_bool          grabMouse;
static Cursor           nullCursor;


/*  Translates the key currently in X_event */

static int xlatekey(void)
{

	int rc;

	switch(rc = XKeycodeToKeysym(X_display, X_event.xkey.keycode, 0))
	{
	  case XK_Left:     rc = KEY_LEFTARROW;     break;
	  case XK_Right:    rc = KEY_RIGHTARROW;    break;
	  case XK_Down:     rc = KEY_DOWNARROW;     break;
	  case XK_Up:       rc = KEY_UPARROW;       break;
	  case XK_Escape:   rc = KEY_ESCAPE;        break;
	  case XK_Return:   rc = KEY_ENTER;         break;
	  case XK_Tab:      rc = KEY_TAB;           break;
	  case XK_F1:       rc = KEY_F1;            break;
	  case XK_F2:       rc = KEY_F2;            break;
	  case XK_F3:       rc = KEY_F3;            break;
	  case XK_F4:       rc = KEY_F4;            break;
	  case XK_F5:       rc = KEY_F5;            break;
	  case XK_F6:       rc = KEY_F6;            break;
	  case XK_F7:       rc = KEY_F7;            break;
	  case XK_F8:       rc = KEY_F8;            break;
	  case XK_F9:       rc = KEY_F9;            break;
	  case XK_F10:      rc = KEY_F10;           break;
	  case XK_F11:      rc = KEY_F11;           break;
	  case XK_F12:      rc = KEY_F12;           break;

	  case XK_BackSpace:
	  case XK_Delete:   rc = KEY_BACKSPACE;     break;

	  case XK_Pause:    rc = KEY_PAUSE;         break;

	  case XK_KP_Equal:
	  case XK_equal:    rc = KEY_EQUALS;        break;

	  case XK_KP_Subtract:
	  case XK_minus:    rc = KEY_MINUS;         break;

	  case XK_Shift_L:
	  case XK_Shift_R:
		rc = KEY_RSHIFT;
		break;

	  case XK_Control_L:
	  case XK_Control_R:
		rc = KEY_RCTRL;
		break;

	  case XK_Alt_L:
	  case XK_Meta_L:
	  case XK_Alt_R:
	  case XK_Meta_R:
		rc = KEY_RALT;
		break;

	  default:
		if (rc >= XK_space && rc <= XK_asciitilde)
			rc = rc - XK_space + ' ';
		if (rc >= 'A' && rc <= 'Z')
			rc = rc - 'A' + 'a';
		break;
	}

	return rc;

}


static int      lastmousex = 0;
static int      lastmousey = 0;
static d_bool  shmFinished;

static void IB_GetEvent(void)
{

	event_t event;
	static int button_state;

	/* put event-grabbing stuff in here */
	XNextEvent(X_display, &X_event);
	switch (X_event.type)
	{
	  case KeyPress:
		event.type = ev_keydown;
		event.data1 = xlatekey();
		D_PostEvent(&event);
		/* I_Info("k"); */
		break;
	  case KeyRelease:
		event.type = ev_keyup;
		event.data1 = xlatekey();
		D_PostEvent(&event);
		/* I_Info("ku"); */
		break;
	  case ButtonPress:
		switch (X_event.xbutton.button)
		{
			case Button1:
				button_state |= 1;
				break;
			case Button2:
				button_state |= 2;
				break;
			case Button3:
				button_state |= 4;
				break;
		}
		event.type = ev_mouse;
		event.data1 = button_state;
		event.data2 = event.data3 = 0;
		D_PostEvent(&event);
		/* I_Info("b"); */
		break;
	  case ButtonRelease:
		switch (X_event.xbutton.button)
		{
			case Button1:
				button_state &= ~1;
				break;
			case Button2:
				button_state &= ~2;
				break;
			case Button3:
				button_state &= ~4;
				break;
		}
		event.type = ev_mouse;
		event.data1 = button_state;
		event.data2 = event.data3 = 0;
		D_PostEvent(&event);
		/* I_Info("bu"); */
		break;
	  case MotionNotify:
		event.type = ev_mouse;
		event.data1 = button_state;
		event.data2 = (X_event.xmotion.x - lastmousex) << 2;
		event.data3 = (lastmousey - X_event.xmotion.y) << 2;

		if (event.data2 || event.data3)
		{
			lastmousex = X_event.xmotion.x;
			lastmousey = X_event.xmotion.y;
			if (X_event.xmotion.x != X_width/2 ||
				X_event.xmotion.y != X_height/2)
			{
				D_PostEvent(&event);
				/* I_Info("m"); */
			}
		}
		break;

	  case Expose:
	  case ConfigureNotify:
		break;

	  default:
		if (doShm && X_event.type == X_shmeventtype) shmFinished = d_true;
		break;
	}

}

static Cursor
createnullcursor
( Display*      display,
  Window        root )
{
	Pixmap cursormask;
	XGCValues xgc;
	GC gc;
	XColor dummycolour;
	Cursor cursor;

	cursormask = XCreatePixmap(display, root, 1, 1, 1/*depth*/);
	xgc.function = GXclear;
	gc =  XCreateGC(display, cursormask, GCFunction, &xgc);
	XFillRectangle(display, cursormask, gc, 0, 0, 1, 1);
	dummycolour.pixel = 0;
	dummycolour.red = 0;
	dummycolour.flags = 04;
	cursor = XCreatePixmapCursor(display, cursormask, cursormask,
								 &dummycolour,&dummycolour, 0,0);
	XFreePixmap(display,cursormask);
	XFreeGC(display,gc);
	return cursor;
}

/* IB_StartTic */
void IB_StartTic (void)
{

	if (!X_display)
		return;

	while (XPending(X_display))
		IB_GetEvent();

	/* Warp the pointer back to the middle of the window */
	/*  or it will wander off - that is, the game will */
	/*  loose input focus within X11. */
	if (grabMouse)
	{
		XWarpPointer( X_display,
					  None,
					  X_mainWindow,
					  0, 0,
					  0, 0,
					  X_width/2, X_height/2);
	}

}


void IB_GetFramebuffer(unsigned char **pixels, size_t *pitch)
{
	*pixels = (unsigned char*)image->data;
	*pitch = X_width * 4;
}


/* IB_FinishUpdate */
void IB_FinishUpdate (void)
{
	if (doShm)
	{

		if (!XShmPutImage(      X_display,
								X_mainWindow,
								X_gc,
								image,
								0, 0,
								0, 0,
								X_width, X_height,
								True ))
			I_Error("XShmPutImage() failed\n");

		/* wait for it to finish and processes all input events */
		shmFinished = d_false;
		do
		{
			IB_GetEvent();
		} while (!shmFinished);

	}
	else
	{

		/* draw the image */
		XPutImage(      X_display,
						X_mainWindow,
						X_gc,
						image,
						0, 0,
						0, 0,
						X_width, X_height );

		/* sync up with server */
		XSync(X_display, False);

	}

}


void IB_GetColor (unsigned char* bytes, unsigned char red, unsigned char green, unsigned char blue)
{
	bytes[0] = blue;
	bytes[1] = green;
	bytes[2] = red;
}


/* This function is probably redundant, */
/*  if XShmDetach works properly. */
/* ddt never detached the XShm memory, */
/*  thus there might have been stale */
/*  handles accumulating. */
static void grabsharedmemory(size_t size)
{

  int                   key = ('d'<<24) | ('o'<<16) | ('o'<<8) | 'm';
  struct shmid_ds       shminfo;
  int                   minsize = 320*200;
  int                   id;
  int                   rc;
  /* UNUSED int done=0; */
  int                   pollution=5;

  /* try to use what was here before */
  do
  {
	id = shmget((key_t) key, minsize, 0777); /* just get the id */
	if (id != -1)
	{
	  rc=shmctl(id, IPC_STAT, &shminfo); /* get stats on it */
	  if (!rc)
	  {
		if (shminfo.shm_nattch)
		{
		  I_Info("User %d appears to be running "
		         "DOOM.  Is that wise?\n", shminfo.shm_cpid);
		  key++;
		}
		else
		{
		  if (getuid() == shminfo.shm_perm.cuid)
		  {
			rc = shmctl(id, IPC_RMID, 0);
			if (!rc)
			  I_Info("Was able to kill my old shared memory\n");
			else
			  I_Error("Was NOT able to kill my old shared memory");

			id = shmget((key_t)key, size, IPC_CREAT|0777);
			if (id==-1)
			  I_Error("Could not get shared memory");

			rc=shmctl(id, IPC_STAT, &shminfo);

			break;

		  }
		  if (size >= shminfo.shm_segsz)
		  {
			I_Info("will use %d's stale shared memory\n",
			       shminfo.shm_cpid);
			break;
		  }
		  else
		  {
			I_Info("warning: can't use stale "
			       "shared memory belonging to id %d, "
			       "key=0x%x\n",
			       shminfo.shm_cpid, key);
			key++;
		  }
		}
	  }
	  else
	  {
		I_Error("could not get stats on key=%d", key);
	  }
	}
	else
	{
	  id = shmget((key_t)key, size, IPC_CREAT|0777);
	  if (id==-1)
	  {
		I_Info("errno=%d\n", errno);
		I_Error("Could not get any shared memory");
	  }
	  break;
	}
  } while (--pollution);

  if (!pollution)
  {
	I_Error("Sorry, system too polluted with stale "
			"shared memory segments.\n");
	}

  X_shminfo.shmid = id;

  /* attach to the shared memory segment */
  image->data = X_shminfo.shmaddr = shmat(id, 0, 0);

  I_Info("shared memory id=%d, addr=0x%p\n", id,
		  (void*) (image->data));
}

static void I_Quit_Wrapper(int dummy)
{
	(void)dummy;

	I_Quit();
}

void IB_InitGraphics(const char *title, size_t screen_width, size_t screen_height, size_t *bytes_per_pixel, void (*output_size_changed_callback)(size_t width, size_t height))
{

	char*               displayname;
	char*               d;
	int                 n;
	int                 pnum;
	int                 x=0;
	int                 y=0;

	/* warning: char format, different type arg */
	char                xsign=' ';
	char                ysign=' ';

	int                 oktodraw;
	unsigned long       attribmask;
	XSetWindowAttributes attribs;
	XGCValues           xgcvalues;
	int                 valuemask;

	signal(SIGINT, I_Quit_Wrapper);

	X_width = screen_width;
	X_height = screen_height;

	/* check for command-line display name */
	if ( (pnum=M_CheckParm("-disp")) ) /* suggest parentheses around assignment */
		displayname = myargv[pnum+1];
	else
		displayname = 0;

	/* check for command-line geometry */
	if ( (pnum=M_CheckParm("-geom")) ) /* suggest parentheses around assignment */
	{
		/* warning: char format, different type arg 3,5 */
		n = sscanf(myargv[pnum+1], "%c%d%c%d", &xsign, &x, &ysign, &y);

		if (n==2)
			x = y = 0;
		else if (n==6)
		{
			if (xsign == '-')
				x = -x;
			if (ysign == '-')
				y = -y;
		}
		else
			I_Error("bad -geom parameter");
	}

	/* open the display */
	X_display = XOpenDisplay(displayname);
	if (!X_display)
	{
		if (displayname)
			I_Error("Could not open display [%s]", displayname);
		else
			I_Error("Could not open display (DISPLAY=[%s])", getenv("DISPLAY"));
	}

	/* use the default visual */
	X_screen = DefaultScreen(X_display);
	if (!XMatchVisualInfo(X_display, X_screen, 24, TrueColor, &X_visualinfo))
		I_Error("xdoom currently only supports 24-bit TrueColor screens");
	X_visual = X_visualinfo.visual;

	/* check for the MITSHM extension */
	doShm = XShmQueryExtension(X_display);

	/* even if it's available, make sure it's a local connection */
	if (doShm)
	{
		if (!displayname) displayname = (char *) getenv("DISPLAY");
		if (displayname)
		{
			d = displayname;
			while (*d && (*d != ':')) d++;
			if (*d) *d = 0;
			if (!(!strcasecmp(displayname, "unix") || !*displayname)) doShm = d_false;
		}
	}

	if (doShm)
		I_Info("Using MITSHM extension\n");

	/* setup attributes for main window */
	attribmask = CWEventMask | CWBorderPixel;
	attribs.event_mask =
		KeyPressMask
		| KeyReleaseMask
		/* | PointerMotionMask | ButtonPressMask | ButtonReleaseMask */
		| ExposureMask;

	attribs.border_pixel = 0;

	/* create the main window */
	X_mainWindow = XCreateWindow(       X_display,
										RootWindow(X_display, X_screen),
										x, y,
										X_width, X_height,
										0, /* borderwidth */
										24, /* depth */
										InputOutput,
										X_visual,
										attribmask,
										&attribs );

	nullCursor = createnullcursor( X_display, X_mainWindow );

	/* create the GC */
	valuemask = GCGraphicsExposures;
	xgcvalues.graphics_exposures = False;
	X_gc = XCreateGC(   X_display,
						X_mainWindow,
						valuemask,
						&xgcvalues );

	/* map the window */
	XMapWindow(X_display, X_mainWindow);

	/* wait until it is OK to draw */
	oktodraw = 0;
	while (!oktodraw)
	{
		XNextEvent(X_display, &X_event);
		if (X_event.type == Expose
			&& !X_event.xexpose.count)
		{
			oktodraw = 1;
		}
	}

	if (doShm)
	{

		X_shmeventtype = XShmGetEventBase(X_display) + ShmCompletion;

		/* create the image */
		image = XShmCreateImage(        X_display,
										X_visual,
										24,
										ZPixmap,
										0,
										&X_shminfo,
										X_width,
										X_height );

		grabsharedmemory(image->bytes_per_line * image->height);


		/* UNUSED */
		/* create the shared memory segment */
		/* X_shminfo.shmid = shmget (IPC_PRIVATE, */
		/* image->bytes_per_line * image->height, IPC_CREAT | 0777); */
		/* if (X_shminfo.shmid < 0) */
		/* { */
		/* perror(""); */
		/* I_Error("shmget() failed in InitGraphics()"); */
		/* } */
		/* I_Info("shared memory id=%d\n", X_shminfo.shmid); */
		/* attach to the shared memory segment */
		/* image->data = X_shminfo.shmaddr = shmat(X_shminfo.shmid, 0, 0); */


		if (!image->data)
		{
			perror("");
			I_Error("shmat() failed in InitGraphics()");
		}

		/* get the X server to attach to it */
		if (!XShmAttach(X_display, &X_shminfo))
			I_Error("XShmAttach() failed in InitGraphics()");

	}
	else
	{
		image = XCreateImage(   X_display,
								X_visual,
								24,
								ZPixmap,
								0,
								(char*)malloc(X_width * X_height * 4),
								X_width, X_height,
								32,
								X_width*4 );

	}

	XStoreName(X_display, X_mainWindow, title);

	*bytes_per_pixel = 4;

	output_size_changed_callback(screen_width, screen_height);
}


void IB_ShutdownGraphics(void)
{
  /* Detach from X server */
  if (!XShmDetach(X_display, &X_shminfo))
			I_Error("XShmDetach() failed in I_ShutdownGraphics()");

  /* Release shared memory. */
  shmdt(X_shminfo.shmaddr);
  shmctl(X_shminfo.shmid, IPC_RMID, 0);

  /* Paranoia. */
  image->data = NULL;
}


void IB_GrabMouse(d_bool grab)
{
	grabMouse = grab;

	if (grab)
	{
		XGrabPointer(X_display, X_mainWindow, True,
					 ButtonPressMask|ButtonReleaseMask|PointerMotionMask,
					 GrabModeAsync, GrabModeAsync,
					 X_mainWindow, None, CurrentTime);
		XDefineCursor(X_display, X_mainWindow, nullCursor);
	}
	else
	{
		XUngrabPointer(X_display, CurrentTime);
		XUndefineCursor(X_display, X_mainWindow);
	}
}
