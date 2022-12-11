cmake_minimum_required(VERSION 3.3.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(DevTools VERSION 1.0.0)

file(GLOB_RECURSE SOURCES
    src/*.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

# This must be precompiled or cocos2d's glew.h takes over
# target_precompile_headers(${PROJECT_NAME} PUBLIC GLEW/glew.h)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

set(GEODE_LINK_NIGHTLY ON)
add_subdirectory($ENV{GEODE_SDK} $ENV{GEODE_SDK}/build)

include(cmake/CPM.cmake)

CPMAddPackage("gh:ocornut/imgui#c191faf")

target_include_directories(${PROJECT_NAME} PRIVATE ${imgui_SOURCE_DIR})

target_sources(${PROJECT_NAME} PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} opengl32 ${CMAKE_CURRENT_SOURCE_DIR}/GLEW/glew32.lib)
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
elseif(ANDROID)
    target_link_libraries(${PROJECT_NAME} GLESv2)
endif()


target_link_libraries(${PROJECT_NAME} geode-sdk)
create_geode_file(${PROJECT_NAME})
