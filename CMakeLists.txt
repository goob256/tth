project("tth")
cmake_minimum_required(VERSION 2.6)

# Options

option(DEBUG "Debug build" OFF)
option(USE_STATIC_LIBS "Link with static libraries" ON)
option(STEAMWORKS "Build for Steam" OFF)
option(DEMO "Build the demo version" OFF)
option(WANT_TTF "Build with TTF support" OFF)
option(ASHEDIT_TRAILER "Build AshEdit trailer" OFF)

set(USER_INCLUDE_PATH "" CACHE FILEPATH "Any extra include directories you want")
set(USER_LIBRARY_PATH "" CACHE FILEPATH "Any extra library directories you want")

include_directories("include")
include_directories(${USER_INCLUDE_PATH})
link_directories(${USER_LIBRARY_PATH})

if(USE_STATIC_LIBS)
	set(WEDGE3_LIB wedge3_static)
	set(SHIM3_LIB shim3_static)
	set(TGUI5_LIB tgui5_static)
else()
	set(WEDGE3_LIB wedge3)
	set(SHIM3_LIB shim3)
	set(TGUI5_LIB tgui5)
endif()

# Inputs

set(SOURCES
	src/achievements.cpp
	src/area_game.cpp
	src/autosave.cpp
	src/battle_combo_drawer.cpp
	src/battle_game.cpp
	src/battle_player.cpp
	src/battle_transition_in.cpp
	src/battle_transition_in2.cpp
	src/battle_transition_out.cpp
	src/battle_transition_out2.cpp
	src/battles.cpp
	src/coin.cpp
	src/combo.cpp
	src/dialogue.cpp
	src/enemies.cpp
	src/general.cpp
	src/globals.cpp
	src/gui.cpp
	src/gui_drawing_hook.cpp
	src/hit.cpp
	src/inventory.cpp
	src/menu.cpp
	src/pan_camera.cpp
	src/question.cpp
	src/start_battle.cpp
	src/transition.cpp
	src/tth.cpp
	src/widgets.cpp
)

if(WIN32)
	list(APPEND SOURCES misc/tth.manifest misc/versioninfo.rc icons/windows_icons.rc)
endif()

if(APPLE)
	set(SOURCES ${SOURCES} src/apple.M)
endif()

# Outputs

add_executable("tth" ${SOURCES})

# Platform specifics

if(WIN32)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /IGNORE:4075 /IGNORE:4099")
	SET(FLAGS "${FLAGS} /D_CRT_SECURE_NO_WARNINGS /D_USE_MATH_DEFINES /EHsc /MP /W3 /wd4244 /wd4800 /wd4101 /MT")
	SET(FLAGS "${FLAGS} /DNTDDI_VERSION=0x06010000 /D_WIN32_WINNT=0x601")
	if(DEBUG)
		set(FLAGS "${FLAGS} /DDEBUG=1")
	endif()
	if(USE_STATIC_LIBS)
		set(FLAGS "${FLAGS} /DTGUI5_STATIC /DSHIM3_STATIC /DWEDGE3_STATIC")
	endif()
	if(STEAMWORKS)
		set(FLAGS "${FLAGS} /DSTEAMWORKS")
		set(LIBS ${LIBS} steam_api64)
	endif()
	if(DEMO)
		set(FLAGS "${FLAGS} /DDEMO")
	endif()
	if(WANT_TTF)
		set(FLAGS "${FLAGS} /DUSE_TTF")
	endif()
	set_target_properties("tth" PROPERTIES COMPILE_FLAGS "${FLAGS}")
	set_target_properties("tth" PROPERTIES OUTPUT_NAME "tth")
	#set(LIBS ${LIBS} ${WEDGE3_LIB} ${SHIM3_LIB} ${TGUI5_LIB} SDL2 SDL2main libvorbisfile_static libvorbis_static ogg zlibstatic d3d9 winmm dbghelp shcore)
	if(ASHEDIT_TRAILER)
		set(LIBS ${LIBS} ${WEDGE3_LIB} ${SHIM3_LIB} ${TGUI5_LIB} SDL2 SDL2main ogg FLAC zlibstatic d3d9 winmm dbghelp shcore)
	else()
		set(LIBS ${LIBS} ${WEDGE3_LIB} ${SHIM3_LIB} ${TGUI5_LIB} SDL2 SDL2main ogg FLAC zlibstatic d3d9 winmm dbghelp shcore)
	endif()
elseif(APPLE)
	SET(FLAGS "-Wall -Wno-extern-c-compat -Wno-c++11-narrowing -Wno-overloaded-virtual -DTGUI5_STATIC -DSHIM3_STATIC -DWEDGE3_STATIC")
	set(FLAGS "${FLAGS} -stdlib=libc++ -std=c++11")
	if(DEBUG)
		set(FLAGS "${FLAGS} -DDEBUG=1 -g")
	endif()
	if(STEAMWORKS)
		set(FLAGS "${FLAGS} -DSTEAMWORKS")
		set(LIBS ${LIBS} steam_api)
	endif()
	if(DEMO)
		set(FLAGS "${FLAGS} -DDEMO")
	endif()
	set_target_properties("tth" PROPERTIES BUILD_WITH_INSTALL_RPATH on)
	set_target_properties("tth" PROPERTIES INSTALL_RPATH "@executable_path/")
	set_target_properties("tth" PROPERTIES COMPILE_FLAGS "${FLAGS}")
	set_target_properties("tth" PROPERTIES LINK_FLAGS "-framework Cocoa -framework IOKit -framework OpenGL -framework CoreAudio -framework AudioUnit -framework ForceFeedback -framework Carbon")
	set_target_properties("tth" PROPERTIES OUTPUT_NAME "tth")
	#set(LIBS ${LIBS} ${WEDGE3_LIB} ${SHIM3_LIB} ${TGUI5_LIB} SDL2 vorbisfile vorbis ogg z)
	set(LIBS ${LIBS} ${WEDGE3_LIB} ${SHIM3_LIB} ${TGUI5_LIB} SDL2 z)
else() # Linux
	SET(FLAGS "-Wall -Wno-comment -Wno-strict-aliasing -std=c++0x")
	set(LFLAGS "-Wl,-rpath,\$ORIGIN")
	if(DEBUG)
		set(FLAGS "${FLAGS} -DDEBUG=1 -g -fsanitize=address -fsanitize=leak")
		set(LFLAGS "${LFLAGS} -fsanitize=address -fsanitize=leak")
	else()
		set(FLAGS "${FLAGS}")
	endif()
	if(STEAMWORKS)
		set(FLAGS "${FLAGS} -DSTEAMWORKS")
		set(LIBS ${LIBS} steam_api)
	endif()
	if(DEMO)
		set(FLAGS "${FLAGS} -DDEMO")
	endif()
	set_target_properties("tth" PROPERTIES COMPILE_FLAGS "${FLAGS}")
	set_target_properties("tth" PROPERTIES LINK_FLAGS "${LFLAGS}")
	set_target_properties("tth" PROPERTIES OUTPUT_NAME "tth")
	#set(LIBS ${LIBS} wedge3_static shim3_static tgui5_static SDL2 GL Xcursor X11 z vorbisfile vorbis ogg)
	#set(LIBS ${LIBS} wedge3_static shim3_static tgui5_static SDL2 GL Xcursor X11 FLAC ogg z)
	set(LIBS ${LIBS} wedge3_static shim3_static tgui5_static SDL2 GL Xcursor X11 FLAC ogg z)
endif()

target_link_libraries("tth" ${LIBS})
