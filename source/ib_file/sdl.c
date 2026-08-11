#include "../ib_file.h"

#ifdef USE_SDL3
	#include <SDL3/SDL.h>
#else
	#include "SDL.h"
#endif

#if SDL_MAJOR_VERSION >= 3
	#undef SDL_RWops
	#define SDL_RWops SDL_IOStream
	#undef SDL_RWFromFile
	#define SDL_RWFromFile SDL_IOFromFile
	#undef SDL_RWclose
	#define SDL_RWclose SDL_CloseIO
	#undef SDL_RWsize
	#define SDL_RWsize SDL_GetIOSize
	#undef SDL_RWread
	#define SDL_RWread(a, b, c, d) (SDL_ReadIO(a, b, d) / c)
	#undef SDL_RWwrite
	#define SDL_RWwrite(a, b, c, d) (SDL_WriteIO(a, b, d) / c)
	#undef SDL_RWseek
	#define SDL_RWseek SDL_SeekIO
	#undef RW_SEEK_SET
	#define RW_SEEK_SET SDL_IO_SEEK_SET
	#undef RW_SEEK_CUR
	#define RW_SEEK_CUR SDL_IO_SEEK_CUR
	#undef RW_SEEK_END
	#define RW_SEEK_END SDL_IO_SEEK_END
#else
	#undef SDL_IOWhence
	#define SDL_IOWhence int
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

	return (I_File*)SDL_RWFromFile(path, rw_mode);
}

void I_FileClose(I_File* const file)
{
	SDL_RWclose((SDL_RWops*)file);
}

size_t I_FileSize(I_File* const file)
{
	return SDL_RWsize((SDL_RWops*)file);
}

size_t I_FileRead(I_File* const file, void* const buffer, const size_t size)
{
	return SDL_RWread((SDL_RWops*)file, buffer, 1, size);
}

size_t I_FileWrite(I_File* const file, const void* const buffer, const size_t size)
{
	return SDL_RWwrite((SDL_RWops*)file, buffer, 1, size);
}

cc_bool I_FilePut(I_File* const file, const char character)
{
	return I_FileWrite(file, &character, 1) != 0;
}

cc_bool I_FileSeek(I_File* const file, const size_t offset, const I_FilePosition position)
{
	SDL_IOWhence rw_position;

	switch (position)
	{
		case I_FILE_POSITION_START:
			rw_position = RW_SEEK_SET;
			break;

		case I_FILE_POSITION_CURRENT:
			rw_position = RW_SEEK_CUR;
			break;

		case I_FILE_POSITION_END:
			rw_position = RW_SEEK_END;
			break;
	}

	return SDL_RWseek((SDL_RWops*)file, offset, rw_position) != -1;
}
