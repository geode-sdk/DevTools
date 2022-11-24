cmake_minimum_required(VERSION 3.3.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(DevTools VERSION 1.0.0)

file(GLOB_RECURSE SOURCES
	src/*.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

# This must be precompiled or cocos2d's glew.h takes over
target_precompile_headers(${PROJECT_NAME} PUBLIC GLEW/glew.h)

add_library(imgui STATIC
    imgui/imgui.cpp
	imgui/imgui_widgets.cpp
	imgui/imgui_tables.cpp
    imgui/imgui_draw.cpp
    imgui/backends/imgui_impl_win32.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)
set_property(TARGET imgui PROPERTY CXX_STANDARD 17)
target_include_directories(imgui PUBLIC
    "imgui"
)
target_link_libraries(
    imgui PRIVATE
    opengl32
)

target_link_libraries(${PROJECT_NAME} 
	imgui
	${CMAKE_SOURCE_DIR}/GLEW/glew32.lib
)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

set(GEODE_LINK_NIGHTLY On)
add_subdirectory($ENV{GEODE_SDK} $ENV{GEODE_SDK}/build)

target_link_libraries(${PROJECT_NAME} geode-sdk)
create_geode_file(${PROJECT_NAME})
