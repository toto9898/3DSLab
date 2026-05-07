cmake_minimum_required(VERSION 3.28)
project(3DSLab)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json for better IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ──────────────────────────────────────────────────────────
# Dependencies via FetchContent
# ──────────────────────────────────────────────────────────
include(FetchContent)

# ── bgfx (via bgfx.cmake wrapper) ─────────────────────────
set(BGFX_BUILD_TOOLS      ON  CACHE BOOL "" FORCE)
set(BGFX_BUILD_EXAMPLES   OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_TESTS      OFF CACHE BOOL "" FORCE)
set(BGFX_INSTALL           OFF CACHE BOOL "" FORCE)
set(BGFX_CUSTOM_TARGETS    OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    bgfx_cmake
    GIT_REPOSITORY https://github.com/bkaradzic/bgfx.cmake.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
    PATCH_COMMAND  ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/patch_bgfx.cmake
)
FetchContent_MakeAvailable(bgfx_cmake)

# ── GLFW ───────────────────────────────────────────────────
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(glfw)

# ── Dear ImGui ─────────────────────────────────────────────
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.8
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

# Build ImGui as a static library with the GLFW backend
set(IMGUI_SRC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
)
add_library(imgui_lib STATIC ${IMGUI_SRC})
target_include_directories(imgui_lib PUBLIC 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui_lib PUBLIC glfw)

# ── Eigen ──────────────────────────────────────────────────
FetchContent_Declare(
    eigen
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG        5.0.1
    GIT_SHALLOW    TRUE
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(eigen)

# ──────────────────────────────────────────────────────────
# Shader compilation (embedded as C headers)
# ──────────────────────────────────────────────────────────
set(SHADER_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/shaders)

# Include bgfx tool utilities for shader compilation
include(${bgfx_cmake_SOURCE_DIR}/cmake/bgfxToolUtils.cmake)

# Mesh shaders
bgfx_compile_shaders(
    TYPE VERTEX
    SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vs_mesh.sc
    VARYING_DEF ${CMAKE_CURRENT_SOURCE_DIR}/shaders/varying.def.sc
    OUTPUT_DIR ${SHADER_OUTPUT_DIR}
    OUT_FILES_VAR VS_MESH_OUTPUTS
    AS_HEADERS
    INCLUDE_DIRS ${BGFX_DIR}/src
)

bgfx_compile_shaders(
    TYPE FRAGMENT
    SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/fs_mesh.sc
    VARYING_DEF ${CMAKE_CURRENT_SOURCE_DIR}/shaders/varying.def.sc
    OUTPUT_DIR ${SHADER_OUTPUT_DIR}
    OUT_FILES_VAR FS_MESH_OUTPUTS
    AS_HEADERS
    INCLUDE_DIRS ${BGFX_DIR}/src
)

# ImGui shaders
bgfx_compile_shaders(
    TYPE VERTEX
    SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vs_imgui.sc
    VARYING_DEF ${CMAKE_CURRENT_SOURCE_DIR}/shaders/varying_imgui.def.sc
    OUTPUT_DIR ${SHADER_OUTPUT_DIR}
    OUT_FILES_VAR VS_IMGUI_OUTPUTS
    AS_HEADERS
    INCLUDE_DIRS ${BGFX_DIR}/src
)

bgfx_compile_shaders(
    TYPE FRAGMENT
    SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/fs_imgui.sc
    VARYING_DEF ${CMAKE_CURRENT_SOURCE_DIR}/shaders/varying_imgui.def.sc
    OUTPUT_DIR ${SHADER_OUTPUT_DIR}
    OUT_FILES_VAR FS_IMGUI_OUTPUTS
    AS_HEADERS
    INCLUDE_DIRS ${BGFX_DIR}/src
)

# Line shaders (unlit, position + color only)
bgfx_compile_shaders(
    TYPE VERTEX
    SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vs_line.sc
    VARYING_DEF ${CMAKE_CURRENT_SOURCE_DIR}/shaders/varying_line.def.sc
    OUTPUT_DIR ${SHADER_OUTPUT_DIR}
    OUT_FILES_VAR VS_LINE_OUTPUTS
    AS_HEADERS
    INCLUDE_DIRS ${BGFX_DIR}/src
)

bgfx_compile_shaders(
    TYPE FRAGMENT
    SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/fs_line.sc
    VARYING_DEF ${CMAKE_CURRENT_SOURCE_DIR}/shaders/varying_line.def.sc
    OUTPUT_DIR ${SHADER_OUTPUT_DIR}
    OUT_FILES_VAR FS_LINE_OUTPUTS
    AS_HEADERS
    INCLUDE_DIRS ${BGFX_DIR}/src
)

set(ALL_SHADER_OUTPUTS
    ${VS_MESH_OUTPUTS} ${FS_MESH_OUTPUTS}
    ${VS_IMGUI_OUTPUTS} ${FS_IMGUI_OUTPUTS}
    ${VS_LINE_OUTPUTS} ${FS_LINE_OUTPUTS}
)

# ──────────────────────────────────────────────────────────
# Project source files
# ──────────────────────────────────────────────────────────
set(SRC_FILES
    src/main.cpp
    src/Rendering/Application.cpp
    src/Rendering/Renderer.cpp
    src/Rendering/Window.cpp
    src/Rendering/imgui_impl_bgfx.cpp
    src/Rendering/MeshSelector.cpp
    src/Rendering/CameraController.cpp
    src/Rendering/MeshUploader.cpp
    src/Rendering/TextureLoader.cpp
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
    src/Parser/Chunks/AnimNodeChunks.cpp
    src/Parser/Chunks/LightChunks.cpp
    src/Parser/Chunks/CameraChunks.cpp
    src/UI/SceneTreePanel.cpp
)

add_executable(${PROJECT_NAME} ${SRC_FILES} ${ALL_SHADER_OUTPUTS})

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
    ${SHADER_OUTPUT_DIR}
    ${bgfx_cmake_SOURCE_DIR}/bimg/3rdparty
)

# Prevent Windows.h from defining min/max macros; disable WGSL shader embedding
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX _USE_MATH_DEFINES BGFX_PLATFORM_SUPPORTS_WGSL=0)

# Link dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE
    bgfx
    bx
    bimg
    glfw
    imgui_lib
    Eigen3::Eigen
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_options(${PROJECT_NAME} PRIVATE "LINKER:/ENTRY:mainCRTStartup")
endif()

# ──────────────────────────────────────────────────────────
# Install rules
# ──────────────────────────────────────────────────────────
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)

# ──────────────────────────────────────────────────────────
# Doxygen docs target  (cmake --build build --target docs)
# ──────────────────────────────────────────────────────────
find_package(Doxygen OPTIONAL_COMPONENTS dot)
if(DOXYGEN_FOUND)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_SOURCE_DIR}/docs/Doxyfile"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
    message(STATUS "Doxygen found — build target 'docs' is available")
else()
    message(STATUS "Doxygen not found — 'docs' target will not be available")
endif()
