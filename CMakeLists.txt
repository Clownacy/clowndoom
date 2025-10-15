cmake_minimum_required(VERSION 3.19)

project(clowndoom LANGUAGES C)

set(BACKEND "SDL2" CACHE STRING "Which backend to use: Xlib, SDL1, SDL2, libretro, or Null")
option(WILDMIDI "Render MIDI using the WildMIDI library" ON)
option(FRENCH "Change the language of various strings to French" OFF)

if(BACKEND STREQUAL "libretro")
	add_library(clowndoom)
else()
	add_executable(clowndoom WIN32)
endif()

target_sources(clowndoom PRIVATE
	"source/clownlibs/clowncommon/clowncommon.h"
	"source/clownlibs/dictionary.c"
	"source/clownlibs/dictionary.h"
	"source/am_map.c"
	"source/am_map.h"
	"source/d_englsh.h"
	"source/d_event.h"
	"source/d_french.h"
	"source/d_items.c"
	"source/d_items.h"
	"source/d_main.c"
	"source/d_main.h"
	"source/d_net.c"
	"source/d_net.h"
	"source/doomdata.h"
	"source/doomdef.c"
	"source/doomdef.h"
	"source/doomstat.c"
	"source/doomstat.h"
	"source/doomtype.h"
	"source/d_player.h"
	"source/dstrings.h"
	"source/d_textur.h"
	"source/d_think.h"
	"source/d_ticcmd.h"
	"source/f_finale.c"
	"source/f_finale.h"
	"source/f_wipe.c"
	"source/f_wipe.h"
	"source/g_game.c"
	"source/g_game.h"
	"source/hu_lib.c"
	"source/hu_lib.h"
	"source/hu_stuff.c"
	"source/hu_stuff.h"
	"source/ib_sound.h"
	"source/ib_system.h"
	"source/ib_video.h"
	"source/i_net.c"
	"source/i_net.h"
	"source/info.c"
	"source/info.h"
	"source/i_sound.c"
	"source/i_sound.h"
	"source/i_system.c"
	"source/i_system.h"
	"source/i_video.c"
	"source/i_video.h"
	"source/m_argv.c"
	"source/m_argv.h"
	"source/m_bbox.c"
	"source/m_bbox.h"
	"source/m_cheat.c"
	"source/m_cheat.h"
	"source/m_fixed.c"
	"source/m_fixed.h"
	"source/m_menu.c"
	"source/m_menu.h"
	"source/m_misc.c"
	"source/m_misc.h"
	"source/m_random.c"
	"source/m_random.h"
	"source/m_swap.h"
	"source/p_ceilng.c"
	"source/p_doors.c"
	"source/p_enemy.c"
	"source/p_floor.c"
	"source/p_inter.c"
	"source/p_inter.h"
	"source/p_lights.c"
	"source/p_local.h"
	"source/p_map.c"
	"source/p_maputl.c"
	"source/p_mobj.c"
	"source/p_mobj.h"
	"source/p_plats.c"
	"source/p_pspr.c"
	"source/p_pspr.h"
	"source/p_saveg.c"
	"source/p_saveg.h"
	"source/p_setup.c"
	"source/p_setup.h"
	"source/p_sight.c"
	"source/p_spec.c"
	"source/p_spec.h"
	"source/p_switch.c"
	"source/p_telept.c"
	"source/p_tick.c"
	"source/p_tick.h"
	"source/p_user.c"
	"source/r_bsp.c"
	"source/r_bsp.h"
	"source/r_data.c"
	"source/r_data.h"
	"source/r_defs.h"
	"source/r_draw.c"
	"source/r_draw.h"
	"source/r_local.h"
	"source/r_main.c"
	"source/r_main.h"
	"source/r_plane.c"
	"source/r_plane.h"
	"source/r_segs.c"
	"source/r_segs.h"
	"source/r_sky.c"
	"source/r_sky.h"
	"source/r_state.h"
	"source/r_things.c"
	"source/r_things.h"
	"source/sounds.c"
	"source/sounds.h"
	"source/s_sound.c"
	"source/s_sound.h"
	"source/st_lib.c"
	"source/st_lib.h"
	"source/st_stuff.c"
	"source/st_stuff.h"
	"source/tables.c"
	"source/tables.h"
	"source/v_video.c"
	"source/v_video.h"
	"source/wi_stuff.c"
	"source/wi_stuff.h"
	"source/w_wad.c"
	"source/w_wad.h"
	"source/z_zone.c"
	"source/z_zone.h"
)

set_target_properties(clowndoom PROPERTIES C_STANDARD 90)

if(CMAKE_C_BYTE_ORDER STREQUAL "BIG_ENDIAN")
	target_compile_definitions(clowndoom PRIVATE PLATFORM_IS_BIG_ENDIAN)
	target_sources(clowndoom PRIVATE "source/m_swap.c")
endif()

if(WIN32)
	target_link_libraries(clowndoom PRIVATE ws2_32)
endif()

if(FRENCH)
	target_compile_definitions(clowndoom PRIVATE FRENCH)
endif()

include(CheckLibraryExists)
check_library_exists(m pow "" LIBM)
if(LIBM)
	target_link_libraries(clowndoom PRIVATE m)
endif()

# Handle backends
if(BACKEND STREQUAL "Xlib")
	target_sources(clowndoom PRIVATE
		"source/ib_sound/miniaudio.c"
		"source/ib_sound/miniaudio.h"
		"source/ib_system/posix.c"
		"source/ib_video/xlib.c"
	)

	# These are for the X11 stuff.
	target_link_libraries(clowndoom PRIVATE Xext X11)

	# These are for miniaudio.
	target_link_libraries(clowndoom PRIVATE ${CMAKE_DL_LIBS} pthread)
elseif(BACKEND STREQUAL "SDL1")
	target_sources(clowndoom PRIVATE
		"source/ib_sound/sdl.c"
		"source/ib_system/sdl.c"
		"source/ib_video/sdl.c"
	)

	find_package(SDL REQUIRED)

	# TODO - Add support for in-tree builds (I don't think it's
	# actually possible with SDL1 since it lacks a CMake build system)
	#if(NOT SDL-FOUND)
	#  add_subdirectory("external/sdl1" EXCLUDE_FROM_ALL)
	#endif()

	target_link_libraries(clowndoom PRIVATE SDL::SDL)
elseif(BACKEND STREQUAL "SDL2")
	target_sources(clowndoom PRIVATE
		"source/ib_sound/sdl.c"
		"source/ib_system/sdl.c"
		"source/ib_video/sdl.c"
	)

	find_package(SDL2)

	if(NOT SDL2_FOUND)
		add_subdirectory("external/SDL" EXCLUDE_FROM_ALL)
		if(SDL_STATIC)
			target_link_libraries(clowndoom PRIVATE SDL2::SDL2main SDL2::SDL2-static)
		else()
			target_link_libraries(clowndoom PRIVATE SDL2::SDL2main SDL2::SDL2)
		endif()
	else()
		target_link_libraries(clowndoom PRIVATE SDL2::SDL2main SDL2::SDL2)
	endif()
elseif(BACKEND STREQUAL "libretro")
	target_sources(clowndoom PRIVATE
		"source/ib_sound/libretro.c"
		"source/ib_system/libretro.c"
		"source/ib_video/libretro.c"
		"external/libretro-common/compat/compat_strl.c"
		"external/libretro-common/compat/fopen_utf8.c"
		"external/libretro-common/encodings/encoding_utf.c"
		"external/libretro-common/file/file_path.c"
		"external/libretro-common/file/file_path_io.c"
		"external/libretro-common/libco/libco.c"
		"external/libretro-common/streams/file_stream.c"
		"external/libretro-common/string/stdstring.c"
		"external/libretro-common/time/rtime.c"
		"external/libretro-common/vfs/vfs_implementation.c"
	)

	############################################
	# Standard libretro core boilerplate code. #
	############################################

	# Avoid some relocation-related linker errors when building a shared library that depends on static libraries.
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)

	# Include libretro-common, so that 'libretro.h' can be found.
	target_include_directories(clowndoom PRIVATE "external/libretro-common/include")

	# Provide an 'stdint.h' for older versions of MSVC.
	if(MSVC AND MSVC_VERSION LESS 1600)
		target_include_directories(clowndoom PRIVATE "external/libretro-common/include/compat/msvc")
	endif()

	# Give the code a way of determining that it is targeting libretro.
	target_compile_definitions(clowndoom PRIVATE __LIBRETRO__)

	# Allow only the libretro API functions to be exported.
	set_target_properties(clowndoom PROPERTIES C_VISIBILITY_PRESET hidden CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)

	# Adjust the library's filename. This is relied upon by libretro's CI infrastructure.

	# Remove the 'lib' prefix.
	set_target_properties(clowndoom PROPERTIES PREFIX "")

	# Append the given suffix.
	set_target_properties(clowndoom PROPERTIES OUTPUT_NAME "clowndoom${LIBRETRO_SUFFIX}")

	# Emscripten builds are expected to use the '.bc' file extension instead of '.a'.
	if(LIBRETRO_STATIC AND EMSCRIPTEN)
		set_target_properties(clowndoom PROPERTIES SUFFIX ".bc")
	endif()

	# Obtain the Git revision so that it can be displayed in the core's version string.
	find_package(Git)
	if(GIT_FOUND)
		execute_process(
			COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
			OUTPUT_VARIABLE GIT_VERSION
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		if(GIT_VERSION)
			target_compile_definitions(clowndoom PRIVATE GIT_VERSION=" ${GIT_VERSION}")
		endif()
	endif()
elseif(BACKEND STREQUAL "Null")
	target_sources(clowndoom PRIVATE
		"source/ib_sound/null.c"
		"source/ib_system/null.c"
		"source/ib_video/null.c"
	)
else()
	message(FATAL_ERROR "Invalid BACKEND")
endif()

# Handling linking/building WildMIDI
if(WILDMIDI)
	target_compile_definitions(clowndoom PRIVATE WILDMIDI)

	find_package(WildMidi)

	if(WildMidi_FOUND)
		target_link_libraries(clowndoom PRIVATE WildMidi::libwildmidi)
	else()
		# Set and hide these internal options
		set(WANT_ALSA OFF CACHE INTERNAL "" FORCE)
		set(WANT_DEVTEST OFF CACHE INTERNAL "" FORCE)
		set(WANT_OPENAL OFF CACHE INTERNAL "" FORCE)
		set(WANT_OSS OFF CACHE INTERNAL "" FORCE)
		set(WANT_OSX_DEPLOYMENT OFF CACHE INTERNAL "" FORCE)
		set(WANT_STATIC ON CACHE INTERNAL "" FORCE)

		add_subdirectory("external/wildmidi" EXCLUDE_FROM_ALL)
		target_link_libraries(clowndoom PRIVATE libwildmidi-static)
	endif()
endif()
