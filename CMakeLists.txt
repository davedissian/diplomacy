cmake_minimum_required(VERSION 3.2)
project(RealTimeStrategy)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES
    src/Main.cpp)
add_executable(RealTimeStrategy ${SOURCE_FILES})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(SFML REQUIRED system window graphics)
include_directories(${SFML_INCLUDE_DIR})
target_link_libraries(RealTimeStrategy ${SFML_LIBRARIES})
