cmake_minimum_required(VERSION 3.20)

project(MeshViewer VERSION 1.0.0 LANGUAGES CXX)

include(CTest)
option(GEOMETRIC_MODELING_BUILD_VIEWER "Build the OpenGL viewer" ON)

set(MESH_CORE_SOURCES
    myFace.cpp
    myHalfedge.cpp
    myMesh.cpp
    myPoint3D.cpp
    myVector3D.cpp
    myVertex.cpp
)

add_library(mesh_core STATIC ${MESH_CORE_SOURCES})
target_include_directories(mesh_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(mesh_core PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(mesh_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(mesh_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(GEOMETRIC_MODELING_BUILD_VIEWER)
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(glm CONFIG REQUIRED)
    find_package(GLUT REQUIRED)

    add_executable(MeshViewer main.cpp)
    target_compile_features(MeshViewer PRIVATE cxx_std_17)
    target_link_libraries(
        MeshViewer
        PRIVATE
            mesh_core
            OpenGL::GL
            OpenGL::GLU
            GLEW::GLEW
            glm::glm
            GLUT::GLUT
    )
endif()

if(BUILD_TESTING)
    add_executable(MeshViewerTests test_mesh.cpp)
    target_link_libraries(MeshViewerTests PRIVATE mesh_core)
    target_compile_definitions(
        MeshViewerTests
        PRIVATE DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    )
    add_test(NAME mesh_tests COMMAND MeshViewerTests)
endif()
