cmake_minimum_required (VERSION 3.0)
project (SimpleChatroom)

set(CMAKE_CXX_STANDARD 11)
set (SimpleChatroom_VERSION_MAJOR 1)
set (SimpleChatroom_VERSION_MINOR 0)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_executable(client client_main.cpp ${SOURCES})
add_executable(server server_main.cpp ${SOURCES})
include_directories("${PROJECT_SOURCE_DIR}/include")

target_link_libraries(client pthread curses)
target_link_libraries(server pthread curses)