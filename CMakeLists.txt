cmake_minimum_required(VERSION 3.20)
project(gtasystemcore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enforce Release build configuration by default (Debug is forbidden)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type" FORCE)
endif()
set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "Configuration types" FORCE)

set(PLUGIN_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plugin-sdk")

# SDK-shared .cpp — исключаем Audio.cpp
file(GLOB PLUGIN_SDK_SHARED_CPP "${PLUGIN_SDK_DIR}/shared/*.cpp")
list(FILTER PLUGIN_SDK_SHARED_CPP EXCLUDE REGEX ".*Audio\\.cpp$")

# SDK-shared/game .cpp — CRGBA, CVector и другие базовые типы
file(GLOB PLUGIN_SDK_SHARED_GAME_CPP "${PLUGIN_SDK_DIR}/shared/game/*.cpp")

# SDK-game_sa .cpp — методы CVehicle, CPools, CTimer и др.
file(GLOB PLUGIN_SDK_GAMESA_CPP "${PLUGIN_SDK_DIR}/plugin_sa/game_sa/*.cpp")

add_library(gtasystemcore SHARED
    src/main.cpp
    src/globals.cpp
    src/globals.h
    src/economy.cpp
    src/economy.h
    src/economy_telemetry.cpp
    src/economy_telemetry.h
    src/economy_retail.cpp
    src/economy_retail.h
    src/municipal.cpp
    src/municipal.h
    src/rioters.cpp
    src/rioters.h
    src/gang_wars.cpp
    src/gang_wars.h
    src/crash_handler.cpp
    src/crash_handler.h
    src/streaming_fix.cpp
    src/streaming_fix.h
    src/web_assets.h
    src/spsc_queue.h
    "${PLUGIN_SDK_DIR}/shared/extensions/ScriptCommands.cpp"
    "${PLUGIN_SDK_DIR}/shared/extensions/Paths.cpp"
    "${PLUGIN_SDK_DIR}/shared/extensions/Screen.cpp"
    ${PLUGIN_SDK_SHARED_CPP}
    ${PLUGIN_SDK_SHARED_GAME_CPP}
    ${PLUGIN_SDK_GAMESA_CPP}
)

# Автоматически подключаем все директории с заголовочными файлами SDK + папку src
file(GLOB_RECURSE PLUGIN_SDK_HEADERS "${PLUGIN_SDK_DIR}/plugin_sa/*.h" "${PLUGIN_SDK_DIR}/shared/*.h")
set(PLUGIN_SDK_INC_DIRS "${PLUGIN_SDK_DIR}/plugin_sa" "${PLUGIN_SDK_DIR}/plugin_sa/game_sa" "${PLUGIN_SDK_DIR}/shared" "${PLUGIN_SDK_DIR}/shared/game" "${CMAKE_CURRENT_SOURCE_DIR}/src")
foreach(_header ${PLUGIN_SDK_HEADERS})
    get_filename_component(_dir ${_header} DIRECTORY)
    list(APPEND PLUGIN_SDK_INC_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES PLUGIN_SDK_INC_DIRS)

target_include_directories(gtasystemcore PRIVATE
    ${PLUGIN_SDK_INC_DIRS}
    "${PLUGIN_SDK_DIR}/shared/dxsdk"
)

set_target_properties(gtasystemcore PROPERTIES
    PREFIX ""
    SUFFIX ".asi"
    OUTPUT_NAME "gtasystemcore"
)

target_compile_options(gtasystemcore PRIVATE /std:c++latest /permissive- /Z7)
target_link_options(gtasystemcore PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
target_compile_definitions(gtasystemcore PRIVATE 
    _CRT_SECURE_NO_WARNINGS
    _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS
    GTASA
    RW
)

# Директории с .lib-файлами SDK
target_link_directories(gtasystemcore PRIVATE
    "${PLUGIN_SDK_DIR}/shared/dxsdk"
    "${PLUGIN_SDK_DIR}/shared/rwd3d9"
)

# Необходимые библиотеки
target_link_libraries(gtasystemcore PRIVATE
    d3d9.lib
    dinput8.lib
    dxguid.lib
    ws2_32.lib
    dbghelp.lib
)

add_custom_command(TARGET gtasystemcore POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:gtasystemcore>"
        "D:/Grand Theft Auto San Andreas/gtasystemcore.asi"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_PDB_FILE:gtasystemcore>"
        "D:/Grand Theft Auto San Andreas/gtasystemcore.pdb"
)