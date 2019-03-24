cmake_minimum_required(VERSION 3.0)

function(embed_resources out_var)
    set(result)
    foreach(in_f ${ARGN})
        file(RELATIVE_PATH src_f ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${in_f})
        set(out_f "${PROJECT_BINARY_DIR}/${in_f}.c")
        set(out_f_dir)
        get_filename_component(out_f_dir ${out_f} DIRECTORY)

        add_custom_command(OUTPUT ${out_f}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${out_f_dir}
                COMMAND embed-resource ${out_f} ${in_f}
                DEPENDS ${in_f}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                COMMENT "Building binary file for embedding ${out_f} from ${in_f}"
                VERBATIM)
        list(APPEND result "${out_f}")
    endforeach()
    set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

add_executable(embed-resource embedresource.cpp)





project(crucible)

#find_package(Vulkan)
#message(STATUS "Vulkan FOUND = ${Vulkan_FOUND}")
#message(STATUS "Vulkan Include = ${Vulkan_INCLUDE_DIR}")
#message(STATUS "Vulkan Lib = ${Vulkan_LIBRARY}")

option(CRUCIBLE_BUILD_EXAMPLES "Build the Crucible example programs" ON)

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(lib/glfw)

option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
option(ASSIMP_BUILD_SAMPLES OFF)
option(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(lib/assimp)

option(BUILD_BULLET2_DEMOS OFF)
option(BUILD_CPU_DEMOS OFF)
option(BUILD_EXTRAS OFF)
option(BUILD_OPENGL3_DEMOS OFF)
option(BUILD_UNIT_TESTS OFF)
add_subdirectory(lib/bullet)

add_subdirectory(lib/freetype)

file(GLOB PROJECT_SHADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/shaders/*)

embed_resources(MyResources ${PROJECT_SHADERS})

include_directories(
                    include/
                    lib/GLFW/include/
                    lib/bullet/src/
                    lib/assimp/include/
                    lib/stb/
                    lib/imgui/
                    lib/freetype/include/
#                    ${Vulkan_INCLUDE_DIR}
                    )

file(GLOB PROJECT_SOURCES src/*.cpp
                          src/*.c
                          lib/imgui/imgui.cpp
                          lib/imgui/imgui_demo.cpp
                          lib/imgui/imgui_draw.cpp
                          lib/imgui/imgui_widgets.cpp
                          lib/imgui/examples/imgui_impl_glfw.cpp
                          lib/imgui/examples/imgui_impl_opengl3.cpp
                          foo.c
                          )

file(GLOB PROJECT_HEADERS include/crucible/*.h
						  include/crucible/*.hpp
						  )

add_definitions(-DGLFW_INCLUDE_NONE
        -DASSIMP_BUILD_BOOST_WORKAROUND
        -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\"
        -DGLFW_STATIC
        -DIMGUI_IMPL_OPENGL_LOADER_GLAD)
add_library(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS} ${MyResources})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES} opengl32 assimp BulletDynamics BulletCollision LinearMath freetype)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES} GL assimp BulletDynamics BulletCollision LinearMath freetype)
endif(WIN32)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()


    if(WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++14 -static -Wa,-mbig-obj")
    elseif(UNIX AND NOT APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++14 -Wa")
    endif(WIN32)
endif()

source_group("Sources" FILES ${PROJECT_SOURCES})
source_group("Headers" FILES ${PROJECT_HEADERS})


if (CRUCIBLE_BUILD_EXAMPLES)
    add_executable(demo examples/demo.cpp)
    add_dependencies(demo crucible)
    target_link_libraries(demo crucible)
    set_target_properties(demo PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

    add_executable(NBodyDemo examples/NBodyDemo.cpp)
    target_link_libraries(NBodyDemo crucible)
    set_target_properties(NBodyDemo PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

    add_executable(VulkanDemo examples/VulkanDemo.cpp)
    target_link_libraries(VulkanDemo crucible)
    set_target_properties(VulkanDemo PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

    add_executable(terrainDemo examples/TerrainDemo.cpp examples/SimplexNoise.cpp examples/SimplexNoise.hpp)
    target_link_libraries(terrainDemo crucible)
    set_target_properties(terrainDemo PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()


file(GLOB EDITOR_SOURCES
        editor/*.c
        editor/*.cpp
        )

file(GLOB EDITOR_HEADERS
        editor/*.h
        editor/*.hpp
        )

add_executable(editor ${EDITOR_SOURCES} ${EDITOR_HEADERS})
target_link_libraries(editor crucible)
set_target_properties(editor PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")