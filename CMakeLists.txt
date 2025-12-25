cmake_minimum_required(VERSION 3.5)

project(GlCraft)
set(TARGET ${CMAKE_PROJECT_NAME})
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
set(LIBNOISE_DIR ${PROJECT_SOURCE_DIR}/depends/noise)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(GLEW REQUIRED)

link_directories(${LIBNOISE_DIR})

set(SOURCES
  "src/Main.cpp"
  "src/Utils.cpp"
  "src/Renderer.cpp"
  "src/IndexBuffer.cpp"
  "src/VertexBuffer.cpp"
  "src/VertexArray.cpp"
  "src/Camera.cpp"
  "src/CameraController.cpp"
  "src/Texture.cpp"
  "src/TextureCubeMap.cpp"
  "src/Block.cpp"
  "src/Chunk.cpp"
  "src/Biome.cpp"
  "src/Window.cpp"
  "src/WindowsInput.cpp"
  "src/Material.cpp"
  "src/Ray.cpp"
  "src/World.cpp"
  "src/Player.cpp"
  "src/AssetManager.cpp"
  "src/Mesh.cpp"
  "src/Inventory.cpp"
  "src/Quad.cpp"
  "src/UIComponent.cpp"
  "src/UI.cpp"
  "src/ParticleSystem.cpp"
  "src/Effect.cpp"
  "src/SmokeEffect.cpp"

  "depends/libnoise/noiseutils.cpp"
  "depends/imgui/imgui_impl_glfw.cpp"
  "depends/imgui/imgui_impl_opengl3.cpp"
  "depends/imgui/imgui.cpp"
  "depends/imgui/imgui_demo.cpp"
  "depends/imgui/imgui_draw.cpp"
  "depends/imgui/imgui_widgets.cpp"
)

add_executable(${TARGET} ${SOURCES})

target_include_directories(${TARGET} PRIVATE
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/depends/imgui
  ${PROJECT_SOURCE_DIR}/depends/libnoise
  ${GLFW_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR}
  ${GLM_INCLUDE_DIRS/../include}
)
target_link_libraries(${TARGET} ${OPENGL_LIBRARIES} glfw GLEW::GLEW)
target_link_libraries(${TARGET} ${LIBNOISE_DIR} noise)
target_link_libraries(${TARGET} ${LIBNOISE_DIR} noiseutils)
