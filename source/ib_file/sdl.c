#include "../ib_file.h"

#include "SDL.h"

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

size_t I_FilePut(I_File* const file, const char character)
{
	return I_FileWrite(file, &character, 1);
}

size_t I_FileSeek(I_File* const file, const size_t offset, const I_FilePosition position)
{
	int rw_position;

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

	return SDL_RWseek((SDL_RWops*)file, offset, rw_position);
}
