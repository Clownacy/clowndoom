#include "../ib_file.h"

I_File* I_FileOpen(const char* const path, const I_FileMode mode)
{
	(void)path;
	(void)mode;
	return NULL;
}

void I_FileClose(I_File* const file)
{
	(void)file;
}

size_t I_FileSize(I_File* const file)
{
	(void)file;
	return 0;
}

size_t I_FileRead(I_File* const file, void* const buffer, const size_t size)
{
	(void)file;
	(void)buffer;
	(void)size;
	return 0;
}

size_t I_FileWrite(I_File* const file, const void* const buffer, const size_t size)
{
	(void)file;
	(void)buffer;
	(void)size;
	return 0;
}

cc_bool I_FilePut(I_File* const file, const char character)
{
	(void)file;
	(void)character;
	return 0;
}

cc_bool I_FileSeek(I_File* const file, const size_t offset, const I_FilePosition position)
{
	(void)file;
	(void)offset;
	(void)position;
	return 0;
}
