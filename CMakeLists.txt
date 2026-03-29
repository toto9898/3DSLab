cmake_minimum_required(VERSION 3.16)
project(3DSLab)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json for better IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Libigl
include(libigl)

# Enable the target igl::glfw
igl_include(glfw)
# Other modules you could enable
#igl_include(embree)
igl_include(imgui)
igl_include(opengl)
#igl_include(stb)
#igl_include(predicates)
#igl_include(xml)
#igl_include(copyleft cgal)
#igl_include(copyleft comiso)
#igl_include(copyleft core)
#igl_include(copyleft cork)
#igl_include(copyleft tetgen)
#igl_include(restricted matlab)
#igl_include(restricted mosek)
#igl_include(restricted triangle)

# Add your project files explicitly with relative paths
set(SRC_FILES
    src/main.cpp
    src/Rendering/Application.cpp
    src/Rendering/MeshSelector.cpp
    src/Rendering/CameraController.cpp
    src/Rendering/MeshUploader.cpp
    src/Parser/Importer.cpp
    src/Scene/Scene.cpp
    src/Scene/Mesh.cpp
    src/Scene/ObjectNode.cpp
    src/Scene/Animation/AnimationHeaders.cpp
    src/Parser/Chunks/Chunk.cpp
    src/Parser/Chunks/ChunkRegistration.cpp
    src/Parser/Chunks/ColorChunks.cpp
    src/Parser/Chunks/FaceArrayChunk.cpp
    src/Parser/Chunks/M3dVersionChunk.cpp
    src/Parser/Chunks/MasterScaleChunk.cpp
    src/Parser/Chunks/MaterialChunks.cpp
    src/Parser/Chunks/PercentageChunk.cpp
    src/Parser/Chunks/MeshMatrixChunk.cpp
    src/Parser/Chunks/MeshVersionChunk.cpp
    src/Parser/Chunks/MshMatGroupChunk.cpp
    src/Parser/Chunks/NamedObjectChunk.cpp
    src/Parser/Chunks/NodeHdrChunk.cpp
    src/Parser/Chunks/NTriObjectChunk.cpp
    src/Parser/Chunks/PointArrayChunk.cpp
    src/Parser/Chunks/TexVertsChunk.cpp
    src/Parser/Chunks/SmoothGroupChunk.cpp
    src/Parser/Chunks/KeyframeChunks.cpp
    src/Parser/Chunks/ObjectNodeChunks.cpp
    src/UI/SceneTreePanel.cpp
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

# Fix: GLFW's cmake emits /clang:-Wall (a clang-cl flag) but the glfw target is
# compiled by clang.exe on this toolchain, which rejects it as an unknown file.
# Replace it with the plain -Wall that clang.exe understands.
if(TARGET glfw)
    get_target_property(_glfw_opts glfw COMPILE_OPTIONS)
    if(_glfw_opts)
        list(REMOVE_ITEM _glfw_opts "/clang:-Wall")
        list(APPEND _glfw_opts "-Wall")
        set_property(TARGET glfw PROPERTY COMPILE_OPTIONS "${_glfw_opts}")
    endif()
endif()

# Add include directories for headers
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Parser
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Parser/Chunks
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Scene
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Scene/Animation
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI
)

# Prevent Windows.h from defining min/max macros (conflicts with std::min/std::max in libigl)
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)

# Link igl (and the glfw module) to your project
target_link_libraries(${PROJECT_NAME} PUBLIC 
  igl::glfw
  igl::opengl
  igl::imgui
  ## Other modules you could link to
  # igl::embree
  # igl::opengl
  # igl::stb
  # igl::predicates
  # igl::xml
  # igl_copyleft::cgal
  # igl_copyleft::comiso
  # igl_copyleft::core
  # igl_copyleft::cork
  # igl_copyleft::tetgen
  # igl_restricted::matlab
  # igl_restricted::mosek
  # igl_restricted::triangle
  )
