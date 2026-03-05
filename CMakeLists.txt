cmake_minimum_required(VERSION 3.22)
project(XPlataformerCpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(xplataformer_core STATIC
    game_core.cpp
)

target_include_directories(xplataformer_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(xplataformer_headless
    main.cpp
)

target_link_libraries(xplataformer_headless PRIVATE xplataformer_core)

find_package(SDL2 REQUIRED)

add_executable(xplataformer_sdl2
    sdl2_full_port.cpp
)

if(TARGET SDL2::SDL2)
    if(TARGET SDL2::SDL2main)
        target_link_libraries(xplataformer_sdl2 PRIVATE xplataformer_core SDL2::SDL2main SDL2::SDL2)
    else()
        target_link_libraries(xplataformer_sdl2 PRIVATE xplataformer_core SDL2::SDL2)
    endif()
elseif(TARGET SDL2::SDL2-static)
    if(TARGET SDL2::SDL2main)
        target_link_libraries(xplataformer_sdl2 PRIVATE xplataformer_core SDL2::SDL2main SDL2::SDL2-static)
    else()
        target_link_libraries(xplataformer_sdl2 PRIVATE xplataformer_core SDL2::SDL2-static)
    endif()
else()
    target_include_directories(xplataformer_sdl2 PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(xplataformer_sdl2 PRIVATE xplataformer_core ${SDL2_LIBRARIES})
endif()
