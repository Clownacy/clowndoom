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
	"linuxdoom-1.10/clownlibs/clowncommon/clowncommon.h"
	"linuxdoom-1.10/clownlibs/dictionary.c"
	"linuxdoom-1.10/clownlibs/dictionary.h"
	"linuxdoom-1.10/am_map.c"
	"linuxdoom-1.10/am_map.h"
	"linuxdoom-1.10/d_englsh.h"
	"linuxdoom-1.10/d_event.h"
	"linuxdoom-1.10/d_french.h"
	"linuxdoom-1.10/d_items.c"
	"linuxdoom-1.10/d_items.h"
	"linuxdoom-1.10/d_main.c"
	"linuxdoom-1.10/d_main.h"
	"linuxdoom-1.10/d_net.c"
	"linuxdoom-1.10/d_net.h"
	"linuxdoom-1.10/doomdata.h"
	"linuxdoom-1.10/doomdef.c"
	"linuxdoom-1.10/doomdef.h"
	"linuxdoom-1.10/doomstat.c"
	"linuxdoom-1.10/doomstat.h"
	"linuxdoom-1.10/doomtype.h"
	"linuxdoom-1.10/d_player.h"
	"linuxdoom-1.10/dstrings.h"
	"linuxdoom-1.10/d_textur.h"
	"linuxdoom-1.10/d_think.h"
	"linuxdoom-1.10/d_ticcmd.h"
	"linuxdoom-1.10/f_finale.c"
	"linuxdoom-1.10/f_finale.h"
	"linuxdoom-1.10/f_wipe.c"
	"linuxdoom-1.10/f_wipe.h"
	"linuxdoom-1.10/g_game.c"
	"linuxdoom-1.10/g_game.h"
	"linuxdoom-1.10/hu_lib.c"
	"linuxdoom-1.10/hu_lib.h"
	"linuxdoom-1.10/hu_stuff.c"
	"linuxdoom-1.10/hu_stuff.h"
	"linuxdoom-1.10/ib_sound.h"
	"linuxdoom-1.10/ib_system.h"
	"linuxdoom-1.10/ib_video.h"
	"linuxdoom-1.10/i_net.c"
	"linuxdoom-1.10/i_net.h"
	"linuxdoom-1.10/info.c"
	"linuxdoom-1.10/info.h"
	"linuxdoom-1.10/i_sound.c"
	"linuxdoom-1.10/i_sound.h"
	"linuxdoom-1.10/i_system.c"
	"linuxdoom-1.10/i_system.h"
	"linuxdoom-1.10/i_video.c"
	"linuxdoom-1.10/i_video.h"
	"linuxdoom-1.10/m_argv.c"
	"linuxdoom-1.10/m_argv.h"
	"linuxdoom-1.10/m_bbox.c"
	"linuxdoom-1.10/m_bbox.h"
	"linuxdoom-1.10/m_cheat.c"
	"linuxdoom-1.10/m_cheat.h"
	"linuxdoom-1.10/m_fixed.c"
	"linuxdoom-1.10/m_fixed.h"
	"linuxdoom-1.10/m_menu.c"
	"linuxdoom-1.10/m_menu.h"
	"linuxdoom-1.10/m_misc.c"
	"linuxdoom-1.10/m_misc.h"
	"linuxdoom-1.10/m_random.c"
	"linuxdoom-1.10/m_random.h"
	"linuxdoom-1.10/m_swap.h"
	"linuxdoom-1.10/p_ceilng.c"
	"linuxdoom-1.10/p_doors.c"
	"linuxdoom-1.10/p_enemy.c"
	"linuxdoom-1.10/p_floor.c"
	"linuxdoom-1.10/p_inter.c"
	"linuxdoom-1.10/p_inter.h"
	"linuxdoom-1.10/p_lights.c"
	"linuxdoom-1.10/p_local.h"
	"linuxdoom-1.10/p_map.c"
	"linuxdoom-1.10/p_maputl.c"
	"linuxdoom-1.10/p_mobj.c"
	"linuxdoom-1.10/p_mobj.h"
	"linuxdoom-1.10/p_plats.c"
	"linuxdoom-1.10/p_pspr.c"
	"linuxdoom-1.10/p_pspr.h"
	"linuxdoom-1.10/p_saveg.c"
	"linuxdoom-1.10/p_saveg.h"
	"linuxdoom-1.10/p_setup.c"
	"linuxdoom-1.10/p_setup.h"
	"linuxdoom-1.10/p_sight.c"
	"linuxdoom-1.10/p_spec.c"
	"linuxdoom-1.10/p_spec.h"
	"linuxdoom-1.10/p_switch.c"
	"linuxdoom-1.10/p_telept.c"
	"linuxdoom-1.10/p_tick.c"
	"linuxdoom-1.10/p_tick.h"
	"linuxdoom-1.10/p_user.c"
	"linuxdoom-1.10/r_bsp.c"
	"linuxdoom-1.10/r_bsp.h"
	"linuxdoom-1.10/r_data.c"
	"linuxdoom-1.10/r_data.h"
	"linuxdoom-1.10/r_defs.h"
	"linuxdoom-1.10/r_draw.c"
	"linuxdoom-1.10/r_draw.h"
	"linuxdoom-1.10/r_local.h"
	"linuxdoom-1.10/r_main.c"
	"linuxdoom-1.10/r_main.h"
	"linuxdoom-1.10/r_plane.c"
	"linuxdoom-1.10/r_plane.h"
	"linuxdoom-1.10/r_segs.c"
	"linuxdoom-1.10/r_segs.h"
	"linuxdoom-1.10/r_sky.c"
	"linuxdoom-1.10/r_sky.h"
	"linuxdoom-1.10/r_state.h"
	"linuxdoom-1.10/r_things.c"
	"linuxdoom-1.10/r_things.h"
	"linuxdoom-1.10/sounds.c"
	"linuxdoom-1.10/sounds.h"
	"linuxdoom-1.10/s_sound.c"
	"linuxdoom-1.10/s_sound.h"
	"linuxdoom-1.10/st_lib.c"
	"linuxdoom-1.10/st_lib.h"
	"linuxdoom-1.10/st_stuff.c"
	"linuxdoom-1.10/st_stuff.h"
	"linuxdoom-1.10/tables.c"
	"linuxdoom-1.10/tables.h"
	"linuxdoom-1.10/v_video.c"
	"linuxdoom-1.10/v_video.h"
	"linuxdoom-1.10/wi_stuff.c"
	"linuxdoom-1.10/wi_stuff.h"
	"linuxdoom-1.10/w_wad.c"
	"linuxdoom-1.10/w_wad.h"
	"linuxdoom-1.10/z_zone.c"
	"linuxdoom-1.10/z_zone.h"
)

set_target_properties(clowndoom PROPERTIES C_STANDARD 90)

if(CMAKE_C_BYTE_ORDER STREQUAL "BIG_ENDIAN")
	target_compile_definitions(clowndoom PRIVATE PLATFORM_IS_BIG_ENDIAN)
	target_sources(clowndoom PRIVATE "linuxdoom-1.10/m_swap.c")
endif()

if(WIN32)
	target_link_libraries(clowndoom PRIVATE ws2_32)
endif()

if(FRENCH)
	target_compile_definitions(clowndoom PRIVATE FRENCH)
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

# Handle backends
if(BACKEND STREQUAL "Xlib")
	target_sources(clowndoom PRIVATE
		"linuxdoom-1.10/ib_sound/miniaudio.c"
		"linuxdoom-1.10/ib_sound/miniaudio.h"
		"linuxdoom-1.10/ib_system/posix.c"
		"linuxdoom-1.10/ib_video/xlib.c"
	)

	# These are for the X11 stuff.
	target_link_libraries(clowndoom PRIVATE Xext X11)

	# These are for miniaudio.
	target_link_libraries(clowndoom PRIVATE m ${CMAKE_DL_LIBS} pthread)
elseif(BACKEND STREQUAL "SDL1")
	target_sources(clowndoom PRIVATE
		"linuxdoom-1.10/ib_sound/sdl.c"
		"linuxdoom-1.10/ib_system/sdl.c"
		"linuxdoom-1.10/ib_video/sdl.c"
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
		"linuxdoom-1.10/ib_sound/sdl.c"
		"linuxdoom-1.10/ib_system/sdl.c"
		"linuxdoom-1.10/ib_video/sdl.c"
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
		"linuxdoom-1.10/ib_sound/libretro.c"
		"linuxdoom-1.10/ib_system/libretro.c"
		"linuxdoom-1.10/ib_video/libretro.c"
		"external/libretro-common/libco/libco.c"
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
		"linuxdoom-1.10/ib_sound/null.c"
		"linuxdoom-1.10/ib_system/null.c"
		"linuxdoom-1.10/ib_video/null.c"
	)
else()
	message(FATAL_ERROR "Invalid BACKEND")
endif()
