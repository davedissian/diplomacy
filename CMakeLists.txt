cmake_minimum_required(VERSION 3.2)
project(Diplomacy)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES
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
    src/voronoi/voronoi.h
    src/voronoi/voronoi.cpp
    src/Common.h
    src/Main.cpp
    src/State.cpp
    src/State.h
    src/World.cpp
    src/World.h)
add_executable(Diplomacy ${SOURCE_FILES})

target_include_directories(Diplomacy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(Diplomacy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/gui)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(SFML REQUIRED system window graphics)
target_include_directories(Diplomacy PRIVATE ${SFML_INCLUDE_DIR})
target_link_libraries(Diplomacy ${SFML_LIBRARIES})

find_package(OpenGL REQUIRED)
target_link_libraries(Diplomacy OpenGL::GL)
