#include "../ib_file.h"

#include "streams/file_stream.h"

I_File* I_FileOpen(const char* const path, const I_FileMode mode)
{
	unsigned int vfs_mode = 0;

	switch (mode)
	{
		case I_FILE_MODE_READ:
			vfs_mode = RETRO_VFS_FILE_ACCESS_READ;
			break;

		case I_FILE_MODE_WRITE:
			vfs_mode = RETRO_VFS_FILE_ACCESS_WRITE;
			break;
	}

	return (I_File*)filestream_open(path, vfs_mode, 0);
}

void I_FileClose(I_File* const file)
{
	filestream_close((RFILE*)file);
}

size_t I_FileSize(I_File* const file)
{
	return filestream_get_size((RFILE*)file);
}

size_t I_FileRead(I_File* const file, void* const buffer, const size_t size)
{
	return filestream_read((RFILE*)file, buffer, size);
}

size_t I_FileWrite(I_File* const file, const void* const buffer, const size_t size)
{
	return filestream_write((RFILE*)file, buffer, size);
}

size_t I_FilePut(I_File* const file, const char character)
{
	return filestream_putc((RFILE*)file, character) == character ? 1 : 0;
}

size_t I_FileSeek(I_File* const file, const size_t offset, const I_FilePosition position)
{
	int vfs_position;

	switch (position)
	{
		case I_FILE_POSITION_START:
			vfs_position = RETRO_VFS_SEEK_POSITION_START;
			break;

		case I_FILE_POSITION_CURRENT:
			vfs_position = RETRO_VFS_SEEK_POSITION_CURRENT;
			break;

		case I_FILE_POSITION_END:
			vfs_position = RETRO_VFS_SEEK_POSITION_END;
			break;
	}

	return filestream_seek((RFILE*)file, offset, vfs_position);
}
