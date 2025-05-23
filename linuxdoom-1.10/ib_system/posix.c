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

#include <sys/time.h>
#include <unistd.h>

#include "../doomdef.h"

#include "../ib_system.h"


/* IB_GetTime */
/* returns time in 1/70th second tics */
int  IB_GetTime (void)
{
	struct timeval      tp;
	struct timezone     tzp;
	int                 newtics;
	static int          basetime=0;

	gettimeofday(&tp, &tzp);
	if (!basetime)
		basetime = tp.tv_sec;
	newtics = (tp.tv_sec-basetime)*TICRATE + tp.tv_usec*TICRATE/1000000;
	return newtics;
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
#ifdef SGI
	sginap(1);
#else
#ifdef SUN
	sleep(0);
#else
	usleep (count * (1000000/70) );
#endif
#endif
}


void IB_Sleep(void)
{
	usleep(1);
}


size_t IB_GetConfigPath(char* const buffer, const size_t size)
{
	static const struct
	{
		const char* environment_variable_name;
		const char* path_suffix;
	} directories = {
		{"XDG_CONFIG_HOME", "/"},
		{"HOME", "/.config/"},
	};

	size_t i;

	for (i = 0; i < D_COUNT_OF(directories); ++i)
	{
		const char* const configdir = getenv(directories[i].environment_variable_name);

		if (configdir != NULL)
		{
			const size_t offset = M_StringCopy(buffer, size, configdir);
			M_StringCopyOffset(buffer, size, offset, directories[i].path_suffix);
			return offset;
		}
	}

	if (size != 0)
		buffer[0] = '\0';

	return 0;
}
