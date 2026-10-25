cmake_minimum_required(VERSION 3.24)
project(GlCraft LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(BUILD_CLIENT "Build the network client" ON)
option(BUILD_SERVER "Build the network server" ON)
option(BUILD_LOCAL "Build the local single-player executable" OFF)

if(NOT BUILD_CLIENT AND NOT BUILD_SERVER AND NOT BUILD_LOCAL)
  message(FATAL_ERROR "Enable at least one of BUILD_CLIENT, BUILD_SERVER, or BUILD_LOCAL")
endif()

if(WIN32)
  add_compile_definitions(_WIN32_WINNT=0x0601)
endif()
add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

# Fetch pinned dependency sources so fresh builds use known versions.
include(FetchContent)
find_package(OpenGL REQUIRED)

set(BUILD_SHARED_LIBS OFF)

# GLFW: keep the build focused on the library. On Linux use X11; on macOS the
# upstream CMake project selects Cocoa automatically.
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
if(UNIX AND NOT APPLE)
  set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
endif()
FetchContent_Declare(glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG a74efa0d5628b74adc0426af4c5710e287fa7c2c # 3.4
  GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(glfw)

# GLM is header-only.
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG a532f5b1cf27d6a3c099437e6959cf7e398a0a67 # 1.0.2
  GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(glm)

# GLEW's upstream Git repository omits generated sources; use the maintained
# CMake packaging of the upstream-generated source tree.
set(ONLY_LIBS ON CACHE BOOL "" FORCE)
set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(glew-cmake_BUILD_STATIC ON CACHE BOOL "" FORCE)
FetchContent_Declare(glew
  GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
  GIT_TAG 5851ebadbec7a7c4beaa497c61d59656f429e965
  GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(glew)

# SFML 3 audio only; it builds its own audio dependencies when needed.
set(SFML_BUILD_WINDOW OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_GRAPHICS OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_AUDIO ON CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(SFML_USE_SYSTEM_DEPS OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
set(SFML_INSTALL_PKGCONFIG_FILES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(SFML
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG 0fa201c969e48ecc253581c5841ce73f44d42f49 # 3.0.2
  GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(SFML)

# Fetch the complete libnoise source at a fixed commit. The old checked-in
# snapshot was missing source files, and its binaries were platform-specific.
set(BUILD_LIBNOISE_DOCUMENTATION OFF CACHE BOOL "" FORCE)
set(BUILD_LIBNOISE_UTILS OFF CACHE BOOL "" FORCE)
set(BUILD_LIBNOISE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(BUILD_SHARED_LIBS OFF)
FetchContent_Declare(libnoise
  GIT_REPOSITORY https://github.com/qknight/libnoise.git
  GIT_TAG 9ce0737b55812f7de907e86dc633724524e3a8e8
)
FetchContent_MakeAvailable(libnoise)

# Use only the ImGui core and the GLFW/OpenGL3 backends used by this app.
add_library(imgui STATIC
  depends/imgui/imgui.cpp
  depends/imgui/imgui_draw.cpp
  depends/imgui/imgui_widgets.cpp
  depends/imgui/imgui_impl_glfw.cpp
  depends/imgui/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC "${PROJECT_SOURCE_DIR}/depends/imgui")
# The ImGui backends include GLFW and use GLEW's OpenGL declarations. Keep
# these dependencies on the backend library so its own sources compile on all
# platforms, not only when consumed by an application target.
target_link_libraries(imgui PRIVATE glfw libglew_static)
target_compile_definitions(imgui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLEW)

set(GLCRAFT_SOURCES
  src/Main.cpp
  src/Utils.cpp
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
  src/Game.cpp
  src/WindowsInput.cpp
  src/Material.cpp
  src/Ray.cpp
  src/World.cpp
  src/Player.cpp
  src/AssetManager.cpp
  src/Mesh.cpp
  src/Inventory.cpp
  src/Quad.cpp
  src/UIComponent.cpp
  src/UI.cpp
  src/ParticleSystem.cpp
  src/Effect.cpp
  src/SmokeEffect.cpp
  src/RainEffect.cpp
  src/SnowEffect.cpp
  src/WaterEffect.cpp
  src/Client.cpp
  src/Server.cpp
)

function(add_glcraft_variant target mode)
  add_executable(${target} ${GLCRAFT_SOURCES})
  target_include_directories(${target} PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/depends"
    "${libnoise_SOURCE_DIR}/src"
  )
  target_compile_definitions(${target} PRIVATE ${mode})
  target_link_libraries(${target} PRIVATE
    OpenGL::GL
    glfw
    libglew_static
    glm::glm
    SFML::Audio
    imgui
    noise-static
  )
endfunction()

if(BUILD_CLIENT)
  add_glcraft_variant(GlCraftClient BUILD_CLIENT)
endif()

if(BUILD_SERVER)
  add_glcraft_variant(GlCraftServer BUILD_SERVER)
endif()

if(BUILD_LOCAL)
  add_glcraft_variant(GlCraftLocal BUILD_LOCAL)
endif()
