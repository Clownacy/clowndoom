#include "../ib_file.h"

#include <stdio.h>

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

	return (I_File*)fopen(path, rw_mode);
}

void I_FileClose(I_File* const file)
{
	fclose((FILE*)file);
}

size_t I_FileSize(I_File* const file)
{
	fpos_t previous_position;
	size_t size;

	fgetpos((FILE*)file, &previous_position);
	fseek((FILE*)file, 0, SEEK_END);
	size = ftell((FILE*)file);
	fsetpos((FILE*)file, &previous_position);	

	return size;
}

size_t I_FileRead(I_File* const file, void* const buffer, const size_t size)
{
	return fread(buffer, 1, size, (FILE*)file);
}

size_t I_FileWrite(I_File* const file, const void* const buffer, const size_t size)
{
	return fwrite(buffer, 1, size, (FILE*)file);
}

cc_bool I_FilePut(I_File* const file, const char character)
{
	return fputc(character, (FILE*)file) != EOF;
}

cc_bool I_FileSeek(I_File* const file, const size_t offset, const I_FilePosition position)
{
	int rw_position;

	switch (position)
	{
		case I_FILE_POSITION_START:
			rw_position = SEEK_SET;
			break;

		case I_FILE_POSITION_CURRENT:
			rw_position = SEEK_CUR;
			break;

		case I_FILE_POSITION_END:
			rw_position = SEEK_END;
			break;
	}

	return fseek((FILE*)file, offset, rw_position) != -1;
}
