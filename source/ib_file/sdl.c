#include "../ib_file.h"

#ifdef USE_SDL3
	#include <SDL3/SDL.h>
#else
	#include "SDL.h"
#endif

I_File* I_FileOpen(const char* const path, const I_FileMode mode)
{
	const char *rw_mode = "";

	switch (mode)
	{
		case I_FILE_MODE_READ:
			rw_mode = "rb";
			break;

		case I_FILE_MODE_WRITE:
			rw_mode = "wb";
			break;
	}

#if SDL_MAJOR_VERSION >= 3
	return (I_File*)SDL_IOFromFile(path, rw_mode);
#else
	return (I_File*)SDL_RWFromFile(path, rw_mode);
#endif
}

void I_FileClose(I_File* const file)
{
#if SDL_MAJOR_VERSION >= 3
	SDL_CloseIO((SDL_IOStream*)file);
#else
	SDL_RWclose((SDL_RWops*)file);
#endif
}

size_t I_FileSize(I_File* const file)
{
#if SDL_MAJOR_VERSION >= 3
	return SDL_GetIOSize((SDL_IOStream*)file);
#else
	return SDL_RWsize((SDL_RWops*)file);
#endif
}

size_t I_FileRead(I_File* const file, void* const buffer, const size_t size)
{
#if SDL_MAJOR_VERSION >= 3
	return SDL_ReadIO((SDL_IOStream*)file, buffer, size);
#else
	return SDL_RWread((SDL_RWops*)file, buffer, 1, size);
#endif
}

size_t I_FileWrite(I_File* const file, const void* const buffer, const size_t size)
{
#if SDL_MAJOR_VERSION >= 3
	return SDL_WriteIO((SDL_IOStream*)file, buffer, size);
#else
	return SDL_RWwrite((SDL_RWops*)file, buffer, 1, size);
#endif
}

cc_bool I_FilePut(I_File* const file, const char character)
{
	return I_FileWrite(file, &character, 1) != 0;
}

cc_bool I_FileSeek(I_File* const file, const size_t offset, const I_FilePosition position)
{
#if SDL_MAJOR_VERSION >= 3
	SDL_IOWhence rw_position;
#else
	int rw_position;
#endif

	switch (position)
	{
		case I_FILE_POSITION_START:
#if SDL_MAJOR_VERSION >= 3
			rw_position = SDL_IO_SEEK_SET;
#else
			rw_position = RW_SEEK_SET;
#endif
			break;

		case I_FILE_POSITION_CURRENT:
#if SDL_MAJOR_VERSION >= 3
			rw_position = SDL_IO_SEEK_CUR;
#else
			rw_position = RW_SEEK_CUR;
#endif
			break;

		case I_FILE_POSITION_END:
#if SDL_MAJOR_VERSION >= 3
			rw_position = SDL_IO_SEEK_END;
#else
			rw_position = RW_SEEK_END;
#endif
			break;
	}

#if SDL_MAJOR_VERSION >= 3
	return SDL_SeekIO((SDL_IOStream*)file, offset, rw_position) != -1;
#else
	return SDL_RWseek((SDL_RWops*)file, offset, rw_position) != -1;
#endif
}
