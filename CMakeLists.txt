cmake_minimum_required(VERSION 3.16)
project(example)

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
#igl_include(imgui)
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
    src/MeshSelector.cpp
    src/3DS/Importer.cpp
    src/3DS/Scene.cpp
    src/3DS/SceneObjects/Mesh.cpp
    src/3DS/Chunks/Chunk.cpp
    src/3DS/Chunks/ChunkRegistration.cpp
    src/3DS/Chunks/Color24Chunk.cpp
    src/3DS/Chunks/ColorFChunk.cpp
    src/3DS/Chunks/FaceArrayChunk.cpp
    src/3DS/Chunks/M3dVersionChunk.cpp
    src/3DS/Chunks/MasterScaleChunk.cpp
    src/3DS/Chunks/MaterialChunks.cpp
    src/3DS/Chunks/PercentageChunk.cpp
    src/3DS/Chunks/MeshMatrixChunk.cpp
    src/3DS/Chunks/MeshVersionChunk.cpp
    src/3DS/Chunks/MshMatGroupChunk.cpp
    src/3DS/Chunks/NamedObjectChunk.cpp
    src/3DS/Chunks/NodeHdrChunk.cpp
    src/3DS/Chunks/NTriObjectChunk.cpp
    src/3DS/Chunks/PointArrayChunk.cpp
    src/3DS/Chunks/TexVertsChunk.cpp
    src/3DS/Chunks/SmoothGroupChunk.cpp
    src/3DS/Chunks/KeyframeChunks.cpp
    src/3DS/Chunks/ObjectNodeChunks.cpp
    src/3DS/SceneObjects/ObjectNode.cpp
    src/3DS/Animation/AnimationHeaders.cpp
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

# Add include directories for headers
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/3DS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/3DS/Chunks
    ${CMAKE_CURRENT_SOURCE_DIR}/src/3DS/SceneObjects
    ${CMAKE_CURRENT_SOURCE_DIR}/src/3DS/Animation
)

# Link igl (and the glfw module) to your project
target_link_libraries(${PROJECT_NAME} PUBLIC 
  igl::glfw
  igl::opengl
  ## Other modules you could link to
  # igl::embree
  # igl::imgui
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
