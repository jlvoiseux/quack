cmake_minimum_required(VERSION 3.16)
project(quack VERSION 0.1.0 LANGUAGES CXX)

find_package(Python REQUIRED)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG main
)
set(SDL_TEST OFF)
set(SDL_SHARED ON)
set(SDL_STATIC OFF)

FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG master
)

FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
    FetchContent_Populate(stb)
endif()

FetchContent_Declare(
        cgltf
        GIT_REPOSITORY https://github.com/jkuhlmann/cgltf.git
        GIT_TAG master
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    FetchContent_Declare(
            tracy
            GIT_REPOSITORY https://github.com/wolfpld/tracy.git
            GIT_TAG v0.11.1
    )

    set(TRACY_ENABLE ON)
    set(TRACY_ON_DEMAND ON)

    FetchContent_MakeAvailable(tracy)
endif()

FetchContent_MakeAvailable(SDL3 cgltf tracy)

add_executable(quack
        src/main.cpp
        src/math/qkVec3.cpp
        src/qkRenderer.cpp
        src/qkModel.cpp
        include/quack/qkModel.h
        src/qkModel.cpp
        include/quack/qkCamera.h
        src/qkCamera.cpp
        include/quack/qkDebug.h
        include/quack/qkTexture.h
        src/qkTexture.cpp
)

target_include_directories(quack
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${cgltf_SOURCE_DIR}
        ${stb_SOURCE_DIR}
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(quack
            PRIVATE
            SDL3::SDL3
            Tracy::TracyClient
    )
else()
    target_link_libraries(quack
            PRIVATE
            SDL3::SDL3
    )
endif()

if(MSVC)
    target_compile_options(quack PRIVATE /W4)
else()
    target_compile_options(quack PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(quack PRIVATE TRACY_ENABLE)
endif()

set_target_properties(quack
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

if(WIN32)
    add_custom_command(TARGET quack POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            ${CMAKE_SOURCE_DIR}/bin
    )
endif()

add_custom_command(TARGET quack POST_BUILD
        COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/copy_assets.py
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_SOURCE_DIR}/bin/assets
        COMMENT "Copying assets to bin directory"
)