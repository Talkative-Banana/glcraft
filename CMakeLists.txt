set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
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
  # "src/ClientMain.cpp"
  # "src/ServerMain.cpp"
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
  "src/SnowEffect.cpp"
  "src/Client.cpp"
  "src/Server.cpp"

  "depends/libnoise/noiseutils.cpp"
  "depends/imgui/imgui_impl_glfw.cpp"
  "depends/imgui/imgui_impl_opengl3.cpp"
  "depends/imgui/imgui.cpp"
  "depends/imgui/imgui_demo.cpp"
  "depends/imgui/imgui_draw.cpp"
  "depends/imgui/imgui_widgets.cpp"
)


set(CLIENT_SOURCES
  # "src/Main.cpp"
  "src/ClientMain.cpp"
  # "src/ServerMain.cpp"
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
  "src/SnowEffect.cpp"
  "src/Client.cpp"
  "src/Server.cpp"

  "depends/libnoise/noiseutils.cpp"
  "depends/imgui/imgui_impl_glfw.cpp"
  "depends/imgui/imgui_impl_opengl3.cpp"
  "depends/imgui/imgui.cpp"
  "depends/imgui/imgui_demo.cpp"
  "depends/imgui/imgui_draw.cpp"
  "depends/imgui/imgui_widgets.cpp"
)

set(SERVER_SOURCES
  # "src/Main.cpp"
  # "src/ClientMain.cpp"
  "src/ServerMain.cpp"
  "src/Utils.cpp"
  "src/Renderer.cpp"
  "src/IndexBuffer.cpp"
  "src/VertexBuffer.cpp"
  "src/VertexArray.cpp"
  "src/Camera.cpp"
  "src/CameraController.cpp"
  "src/Block.cpp"
  "src/Chunk.cpp"
  "src/Biome.cpp"
  "src/Window.cpp"
  "src/WindowsInput.cpp"
  "src/Ray.cpp"
  "src/World.cpp"
  "src/Player.cpp"
  "src/AssetManager.cpp"
  "src/Mesh.cpp"
  "src/Inventory.cpp"
  "src/Client.cpp"
  "src/Server.cpp"

  "depends/libnoise/noiseutils.cpp"
  "depends/imgui/imgui_impl_glfw.cpp"
  "depends/imgui/imgui_impl_opengl3.cpp"
  "depends/imgui/imgui.cpp"
  "depends/imgui/imgui_demo.cpp"
  "depends/imgui/imgui_draw.cpp"
  "depends/imgui/imgui_widgets.cpp"
)

option(BUILD_CLIENT "Build client" ON)
option(BUILD_SERVER "Build server" ON)
option(BUILD_LOCAL  "Build local single binary" OFF)

if (BUILD_CLIENT)
  add_executable(GlCraftClient ${CLIENT_SOURCES})

  target_include_directories(GlCraftClient PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/depends
    ${PROJECT_SOURCE_DIR}/depends/imgui
    ${PROJECT_SOURCE_DIR}/depends/libnoise
    ${GLFW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}}
  )
  target_link_libraries(GlCraftClient ${OPENGL_LIBRARIES} glfw GLEW::GLEW)
  target_link_libraries(GlCraftClient ${LIBNOISE_DIR} noise)
  target_link_libraries(GlCraftClient ${LIBNOISE_DIR} noiseutils)

endif()

if (BUILD_SERVER)
  add_executable(GlCraftServer ${SERVER_SOURCES})

  target_include_directories(GlCraftServer PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/depends
    ${PROJECT_SOURCE_DIR}/depends/imgui
    ${PROJECT_SOURCE_DIR}/depends/libnoise
    ${GLFW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}}
  )
  target_link_libraries(GlCraftServer ${OPENGL_LIBRARIES} glfw GLEW::GLEW)
  target_link_libraries(GlCraftServer ${LIBNOISE_DIR} noise)
  target_link_libraries(GlCraftServer ${LIBNOISE_DIR} noiseutils)

endif()

if (BUILD_LOCAL)
  add_executable(GlCraftLocal ${SOURCES})

  target_include_directories(GlCraftLocal PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/depends
    ${PROJECT_SOURCE_DIR}/depends/imgui
    ${PROJECT_SOURCE_DIR}/depends/libnoise
    ${GLFW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}}
  )
  target_link_libraries(GlCraftLocal ${OPENGL_LIBRARIES} glfw GLEW::GLEW)
  target_link_libraries(GlCraftLocal ${LIBNOISE_DIR} noise)
  target_link_libraries(GlCraftLocal ${LIBNOISE_DIR} noiseutils)
endif()
