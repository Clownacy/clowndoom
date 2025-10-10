#include "libretro-helper.h"

#include <stdio.h>

static struct retro_vfs_file_handle* RETRO_CALLCONV File_OpenDefault(const char* const path, const unsigned int mode, const unsigned int hints)
{
	const char* mode_standard;

	(void)hints;

	switch (mode)
	{
		case RETRO_VFS_FILE_ACCESS_READ:
		case RETRO_VFS_FILE_ACCESS_READ | RETRO_VFS_FILE_ACCESS_UPDATE_EXISTING:
			mode_standard = "rb";
			break;

		case RETRO_VFS_FILE_ACCESS_WRITE:
			mode_standard = "wb";
			break;

		case RETRO_VFS_FILE_ACCESS_WRITE | RETRO_VFS_FILE_ACCESS_UPDATE_EXISTING:
		case RETRO_VFS_FILE_ACCESS_READ | RETRO_VFS_FILE_ACCESS_WRITE | RETRO_VFS_FILE_ACCESS_UPDATE_EXISTING:
			mode_standard = "r+b";
			break;

		case RETRO_VFS_FILE_ACCESS_READ | RETRO_VFS_FILE_ACCESS_WRITE:
			mode_standard = "w+b";
			break;

		default:
			return NULL;
	}

	return (struct retro_vfs_file_handle*)fopen(path, mode_standard);
}

static int RETRO_CALLCONV File_CloseDefault(struct retro_vfs_file_handle* const stream)
{
	if (stream == NULL)
		return -1;

	return fclose((FILE*)stream) == 0 ? 0 : -1;
}


static int64_t RETRO_CALLCONV File_GetSizeDefault(struct retro_vfs_file_handle* const stream)
{
	FILE* const file = (FILE*)stream;
	fpos_t position;
	int64_t result = -1;

	if (fgetpos(file, &position) == 0)
	{
		if (fseek(file, 0, SEEK_END) == 0)
			result = ftell(file);

		if (fsetpos(file, &position) != 0)
			result = -1;
	}

	return result;
}

static int64_t RETRO_CALLCONV File_TellDefault(struct retro_vfs_file_handle* const stream)
{
	return ftell((FILE*)stream);
}

static int64_t RETRO_CALLCONV File_SeekDefault(struct retro_vfs_file_handle* const stream, const int64_t offset, const int seek_position)
{
	int whence;

	if (offset < LONG_MIN || offset > LONG_MAX)
		return -1;

	switch (seek_position)
	{
		case RETRO_VFS_SEEK_POSITION_START:
			whence = SEEK_SET;
			break;

		case RETRO_VFS_SEEK_POSITION_CURRENT:
			whence = SEEK_CUR;
			break;

		case RETRO_VFS_SEEK_POSITION_END:
			whence = SEEK_END;
			break;

		default:
			return -1;
	}

	if (fseek((FILE*)stream, offset, whence) != 0)
		return -1;

	return File_TellDefault(stream);
}

static int64_t RETRO_CALLCONV File_ReadDefault(struct retro_vfs_file_handle* const stream, void* const s, const uint64_t len)
{
	if (len > (size_t)-1)
		return -1;

	return fread(s, 1, len, (FILE*)stream);
}

static int64_t RETRO_CALLCONV File_WriteDefault(struct retro_vfs_file_handle* const stream, const void* const s, const uint64_t len)
{
	if (len > (size_t)-1)
		return -1;

	return fwrite(s, 1, len, (FILE*)stream);
}

static int RETRO_CALLCONV File_RemoveDefault(const char* const path)
{
	return remove(path);
}

static const struct retro_vfs_interface default_iface = {
	NULL,
	File_OpenDefault,
	File_CloseDefault,
	File_GetSizeDefault,
	File_TellDefault,
	File_SeekDefault,
	File_ReadDefault,
	File_WriteDefault,
	NULL,
	File_RemoveDefault,
};

bool libretro_helper_get_vfs_interface(const retro_environment_t environment, struct retro_vfs_interface_info* const info)
{
	if (info->required_interface_version != 1)
		return false;

	if (!environment(RETRO_ENVIRONMENT_GET_VFS_INTERFACE, info))
		info->iface = &default_iface;

	return true;
}
