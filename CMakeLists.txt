cmake_minimum_required(VERSION 3.5)

project(GlCraft)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TARGET ${CMAKE_PROJECT_NAME})
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

# Dependency paths
set(LIBNOISE_DIR ${PROJECT_SOURCE_DIR}/depends/noise)
set(GLFW_DIR ${PROJECT_SOURCE_DIR}/depends/glfw)
set(GLEW_DIR ${PROJECT_SOURCE_DIR}/depends/glew)
set(GLM_DIR ${PROJECT_SOURCE_DIR}/depends/glm)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Source files
set(SOURCES
  src/Main.cpp
  src/Utils.cpp
  src/Renderer.cpp
  src/IndexBuffer.cpp
  src/VertexBuffer.cpp
  src/VertexArray.cpp
  src/Camera.cpp
  src/CameraController.cpp
  src/Texture.cpp
  src/TextureCubeMap.cpp
  src/Block.cpp
  src/Chunk.cpp
  src/Biome.cpp
  src/Window.cpp
  src/WindowsInput.cpp
  src/Material.cpp
  src/Ray.cpp
  src/World.cpp
  src/Player.cpp
  src/AssetManager.cpp
  src/Mesh.cpp

  depends/libnoise/noiseutils.cpp
  depends/imgui/imgui_impl_glfw.cpp
  depends/imgui/imgui_impl_opengl3.cpp
  depends/imgui/imgui.cpp
  depends/imgui/imgui_demo.cpp
  depends/imgui/imgui_draw.cpp
  depends/imgui/imgui_widgets.cpp
)

add_executable(${TARGET} ${SOURCES})

# Include directories
target_include_directories(${TARGET} PRIVATE
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/depends/imgui
  ${PROJECT_SOURCE_DIR}/depends/libnoise
  ${PROJECT_SOURCE_DIR}/depends/libnoise
  ${GLFW_DIR}/include
  ${GLEW_DIR}/include
  ${GLM_DIR}
  ${OPENGL_INCLUDE_DIR}
)

# Define macros for static linking
target_compile_definitions(${TARGET} PRIVATE GLEW_STATIC GLFW_STATIC)

# Link libraries
target_link_libraries(${TARGET}
  ${OPENGL_LIBRARIES}
  ${GLEW_DIR}/lib/Release/x64/glew32s.lib
  ${GLFW_DIR}/lib/glfw3.lib
  ${LIBNOISE_DIR}/lib/noise-static.lib
)
