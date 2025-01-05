cmake_minimum_required(VERSION 3.16)
project(quack VERSION 0.1.0 LANGUAGES CXX)

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

FetchContent_MakeAvailable(SDL3)

add_library(quack_lib
        src/math/vec3.cpp
        src/renderer.cpp
)

target_include_directories(quack_lib
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(quack_lib
        PUBLIC
        SDL3::SDL3
)

if(MSVC)
    target_compile_options(quack_lib PRIVATE /W4)
else()
    target_compile_options(quack_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(quack src/main.cpp)

target_link_libraries(quack
        PRIVATE
        quack_lib
)

set_target_properties(quack
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

install(TARGETS quack
        RUNTIME DESTINATION bin
)

if(WIN32)
    add_custom_command(TARGET quack POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:quack>
    )
endif()