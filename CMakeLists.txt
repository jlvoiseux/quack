cmake_minimum_required(VERSION 3.16)
project(quack VERSION 0.1.0 LANGUAGES C)

find_package(Python REQUIRED)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

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
    FetchContent_MakeAvailable(stb)
endif()

FetchContent_Declare(
        cgltf
        GIT_REPOSITORY https://github.com/jkuhlmann/cgltf.git
        GIT_TAG master
)

FetchContent_MakeAvailable(SDL3 cgltf)

add_executable(quack
        src/main.c
        src/math/qkVec3.c
        src/buffer/qkBuffer.c
        src/buffer/qkSpanBuffer.c
        src/buffer/qkVertexBuffer.c
        src/qkRenderer.c
        src/qkModel.c
        src/qkCamera.c
        src/qkTexture.c
        src/qkPerformance.c
)

target_include_directories(quack
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${cgltf_SOURCE_DIR}
        ${stb_SOURCE_DIR}
)

target_link_libraries(quack PRIVATE SDL3::SDL3)

if(MSVC)
    target_compile_options(quack PRIVATE /W4)
else()
    target_compile_options(quack PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_compile_definitions(quack PRIVATE SIMD_ENABLE)

set_target_properties(quack
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/bin"
)

if(WIN32)
    add_custom_command(TARGET quack POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            ${CMAKE_SOURCE_DIR}/bin
    )

    if(MSVC)
        # Add Visual C++ runtime DLLs deployment
        include(InstallRequiredSystemLibraries)
        add_custom_command(TARGET quack POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} ${CMAKE_SOURCE_DIR}/bin
        )
    endif()

endif()

add_custom_command(TARGET quack POST_BUILD
        COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/copy_assets.py
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_SOURCE_DIR}/bin/assets
        COMMENT "Copying assets to bin directory"
)