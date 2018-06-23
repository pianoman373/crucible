cmake_minimum_required(VERSION 3.0)
project(crucible)

option(CRUCIBLE_BUILD_EXAMPLES "Build the Crucible example programs" ON)

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(lib/glfw)

option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
option(ASSIMP_BUILD_SAMPLES OFF)
option(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(lib/assimp)

include_directories(
                    include/
                    )

file(GLOB PROJECT_SOURCES src/*.cpp
                          src/*.c
                          )

file(GLOB PROJECT_HEADERS include/crucible/*.h
						  include/crucible/*.hpp
						  )

add_definitions(-DGLFW_INCLUDE_NONE
                -DASSIMP_BUILD_BOOST_WORKAROUND
                -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\"
				-DGLFW_STATIC
				-DSFML_STATIC)
add_library(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES} opengl32 assimp)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES} GL assimp)
endif(WIN32)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++14")
endif()

source_group("Sources" FILES ${PROJECT_SOURCES})
source_group("Headers" FILES ${PROJECT_HEADERS})


if (CRUCIBLE_BUILD_EXAMPLES)
    add_executable(demo examples/demo.cpp)
    target_link_libraries(demo crucible)
    set_target_properties(demo PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

    add_executable(2dDemo examples/2dDemo.cpp)
    target_link_libraries(2dDemo crucible)
    set_target_properties(2dDemo PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

    add_executable(NBodyDemo examples/NBodyDemo.cpp)
    target_link_libraries(NBodyDemo crucible)
    set_target_properties(NBodyDemo PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

    add_executable(terrainDemo examples/terrainDemo.cpp)
    target_link_libraries(terrainDemo crucible)
    set_target_properties(terrainDemo PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()

add_executable(editor editor/main.cpp editor/WorkspaceObject.cpp editor/GuiMain.cpp editor/GuiMain.hpp)
target_link_libraries(editor crucible)
set_target_properties(editor PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")