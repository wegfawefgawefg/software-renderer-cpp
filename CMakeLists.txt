cmake_minimum_required(VERSION 3.20)
project(software_renderer_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# SDL2 (+ image/ttf). Keep giflib out for now; we'll add it back when we need GIFs.
find_package(SDL2 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

add_library(sr
    src/sr/platform/sdl.cpp
    src/sr/gfx/framebuffer.cpp
    src/sr/gfx/texture.cpp
    src/sr/assets/obj_loader.cpp
    src/sr/assets/texture_loader.cpp
    src/sr/assets/asset_store.cpp
    src/sr/assets/mtl_loader.cpp
    src/sr/assets/obj_model_loader.cpp
    src/sr/assets/gltf_model_loader.cpp
    src/sr/render/renderer.cpp
    src/sr/scene/fly_camera.cpp
    src/sr/scene/player_controller.cpp
    src/sr/physics/triangle_collider.cpp
)
target_include_directories(sr PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party/cgltf
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
)
target_link_libraries(sr PUBLIC
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
)

if(UNIX AND NOT APPLE)
    target_link_libraries(sr PUBLIC m)
endif()

target_compile_options(sr PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wformat=2
    -Wno-unused-parameter
)

add_executable(renderer
    src/app/main.cpp
)
target_link_libraries(renderer PRIVATE sr)
