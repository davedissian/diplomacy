cmake_minimum_required(VERSION 3.2)
project(Diplomacy)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

# Mirror the directory structure in virtual directory based projects
function(mirror_physical_directories)
    foreach(FILE ${ARGN})
        get_filename_component(PARENT_DIR "${FILE}" PATH)
        string(REPLACE "/" "\\" GROUP "${PARENT_DIR}")
        source_group("${GROUP}" FILES "${FILE}")
    endforeach()
endfunction()

set(SOURCE_FILES
    src/gameplay/Orders.cpp
    src/gameplay/Orders.h
    src/gameplay/Tank.cpp
    src/gameplay/Tank.h
    src/gameplay/Unit.cpp
    src/gameplay/Unit.h
    src/gui/imconfig.h
    src/gui/imgui.cpp
    src/gui/imgui.h
    src/gui/imgui-SFML.cpp
    src/gui/imgui-SFML.h
    src/gui/imgui_demo.cpp
    src/gui/imgui_draw.cpp
    src/gui/imgui_internal.h
    src/gui/stb_rect_pack.h
    src/gui/stb_textedit.h
    src/gui/stb_truetype.h
    src/noise/Noise.cpp
    src/noise/Noise.h
    src/voronoi/voronoi.h
    src/voronoi/voronoi.cpp
    src/Common.h
    src/Main.cpp
    src/Map.cpp
    src/Map.h
    src/State.cpp
    src/State.h
    src/World.cpp
    src/World.h)
add_executable(Diplomacy ${SOURCE_FILES})
mirror_physical_directories(${SOURCE_FILES})

target_include_directories(Diplomacy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(Diplomacy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/gui)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML REQUIRED system window graphics)
target_include_directories(Diplomacy PRIVATE ${SFML_INCLUDE_DIR})
target_link_libraries(Diplomacy ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})

find_package(OpenGL REQUIRED)
target_link_libraries(Diplomacy OpenGL::GL)
