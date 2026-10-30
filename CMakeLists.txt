cmake_minimum_required(VERSION 3.20)
project(Kyonshiizu2Recomp C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(NESRECOMP_ROOT "" CACHE PATH "NESRecomp engine path; set with -DNESRECOMP_ROOT=...")
set(NESRECOMP_ROM "${CMAKE_CURRENT_SOURCE_DIR}/roms/game.nes" CACHE FILEPATH "Target NES ROM")
set(NESRECOMP_HOST_COMPILER "${NESRECOMP_ROOT}/build/recompiler/NESRecomp.exe" CACHE FILEPATH "Prebuilt NESRecomp executable")

if(NOT NESRECOMP_ROOT)
    message(FATAL_ERROR "Set NESRECOMP_ROOT with -DNESRECOMP_ROOT=...")
endif()
if(NOT EXISTS "${NESRECOMP_ROOT}/runner/cyc/project.cmake")
    message(FATAL_ERROR "NESRECOMP_ROOT does not contain runner/cyc/project.cmake")
endif()
if(NOT EXISTS "${NESRECOMP_HOST_COMPILER}")
    message(FATAL_ERROR "Build NESRecomp first or set NESRECOMP_HOST_COMPILER to NESRecomp.exe")
endif()
if(NOT EXISTS "${NESRECOMP_ROM}")
    message(FATAL_ERROR "Supply the supported ROM locally at roms/game.nes or set NESRECOMP_ROM")
endif()
file(SHA256 "${NESRECOMP_ROM}" NESRECOMP_INPUT_SHA256)
if(NOT NESRECOMP_INPUT_SHA256 STREQUAL "4fbf02411b66f8e4cdff16219612fc8aa804ec12f2e2ce19235afbccdaeaf4a2")
    message(FATAL_ERROR "Unsupported ROM: SHA-256 does not match the reference image")
endif()

include("${NESRECOMP_ROOT}/runner/cyc/project.cmake")

nesrecomp_add_cycle_game(Kyonshiizu2Recomp
    ROM "${NESRECOMP_ROM}"
    GAME_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/game.toml"
    RECOMPILER "${NESRECOMP_HOST_COMPILER}"
    HOST_EXTRAS "${CMAKE_CURRENT_SOURCE_DIR}/menu_translation_test.c" "${CMAKE_CURRENT_SOURCE_DIR}/text_trace_probe.c" "${CMAKE_CURRENT_SOURCE_DIR}/text_lookup_probe.c" "${CMAKE_CURRENT_SOURCE_DIR}/translation_lookup.c" "${CMAKE_CURRENT_SOURCE_DIR}/english_display_test.c"
    PLAYERS 2
)

# Hide command prompt console window on launch
set_target_properties(Kyonshiizu2Recomp PROPERTIES WIN32_EXECUTABLE TRUE)
target_link_options(Kyonshiizu2Recomp PRIVATE -mwindows)
