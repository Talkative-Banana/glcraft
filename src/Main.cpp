#include "Utils.h"

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <../stb/stb_image.h>
#include <../stb/stb_image_write.h>

#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <memory>

#include "AssetManager.h"
#include "Constants.hpp"
#include "Input.h"
#include "Main.h"
#include "Player.h"
#include "Ray.h"
#include "Renderer.h"
#include "Texture.h"
#include "World.h"

// Globals
glm::ivec3 _wps = {0, 0, 0};
std::unique_ptr<World> world = std::make_unique<World>(42, _wps);
std::unique_ptr<Window> _window = nullptr;
glm::vec3 chunkpos;
GLuint activePlayer, players_cnt = 2;
GLint vModel_uniform = -1;
GLint vView_uniform = -1;
GLint vProjection_uniform = -1;
GLint side_uniform = -1;
GLint chunkpos_uniform;
GLint vColor_uniform;
GLint atlas_uniform;
GLint vVertex_attrib = -1;
GLint vNormal_attrib = -1;
GLint cameraPos_uniform = -1;
GLint lightpos_uniform = -1;
GLint atlast_uniform = -1;
GLuint wireframemode, shaderProgram, shaderProgram2;
glm::mat4 modelT, viewT, projectionT;  // The model, view and projection transformations
std::vector<std::shared_ptr<Mesh>> meshes;
std::array<std::unique_ptr<Player>, PLAYER_COUNT> players;

// void createAxesLine(unsigned int &, unsigned int &);
ImVec4 clearColor;

// void draw_axis(unsigned int axis_VAO, unsigned int shaderProgram) {
//   glBindVertexArray(axis_VAO);
//   setupModelTransformationAxis(shaderProgram, 0.0, glm::vec3(0, 0, 1));
//   // glUniform4f(vColor_uniform, 1.0, 0.0, 0.0, 1.0); //Red -> X
//   glDrawArrays(GL_LINES, 0, 2);
//
//   setupModelTransformationAxis(shaderProgram, glm::radians(90.0), glm::vec3(0, 0, 1));
//   // glUniform4f(vColor_uniform, 0.0, 1.0, 0.0, 1.0); //Green -> Y
//   glDrawArrays(GL_LINES, 0, 2);
//
//   setupModelTransformationAxis(shaderProgram, -glm::radians(90.0), glm::vec3(0, 1, 0));
//   // glUniform4f(vColor_uniform, 0.0, 0.0, 1.0, 1.0); //Blue -> Z
//   glDrawArrays(GL_LINES, 0, 2);
//
//   glEnable(GL_DEPTH_TEST);  // Enable depth test again
// }

// void createAxesLine(unsigned int &program, unsigned int &axis_VAO) {
//   glUseProgram(program);
//
//   // Bind shader variables
//   int vVertex_attrib_position = glGetAttribLocation(program, "vVertex");
//   if (vVertex_attrib_position == -1) {
//     fprintf(stderr, "Could not bind location: vVertex\n");
//     exit(0);
//   }
//
//   // Axes data
//   GLfloat axis_vertices[] = {0, 0, 0, 20, 0, 0};  // X-axis
//   glGenVertexArrays(1, &axis_VAO);
//   glBindVertexArray(axis_VAO);
//
//   // Create VBO for the VAO
//   int nVertices = 2;  // 2 vertices
//   GLuint vertex_VBO;
//   glGenBuffers(1, &vertex_VBO);
//   glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
//   glBufferData(GL_ARRAY_BUFFER, nVertices * 3 * sizeof(GLfloat), axis_vertices, GL_STATIC_DRAW);
//   glEnableVertexAttribArray(vVertex_attrib_position);
//   glVertexAttribPointer(vVertex_attrib_position, 3, GL_FLOAT, GL_FALSE, 0, 0);
//
//   glBindBuffer(GL_ARRAY_BUFFER, 0);
//   glBindVertexArray(0);  // Unbind the VAO to disable changes outside this function.
// }

// int FrustumCulling(GLuint vertex) {
//   // return 0 if inside frustum
//
//   auto Center = [&](glm::ivec3 pos, uint centeroff) {
//     glm::ivec3 center = pos;
//
//     if ((centeroff & 1u) == 1u) {
//       center.z += HALF_BLOCK_SIZE;
//     } else {
//       center.z -= HALF_BLOCK_SIZE;
//     }
//
//     if ((centeroff & 2u) == 2u) {
//       center.y += HALF_BLOCK_SIZE;
//     } else {
//       center.y -= HALF_BLOCK_SIZE;
//     }
//
//     if ((centeroff & 4u) == 4u) {
//       center.x += HALF_BLOCK_SIZE;
//     } else {
//       center.x -= HALF_BLOCK_SIZE;
//     }
//     return center;
//   };
//
//   uint positionX = (vertex) & 63u;
//   uint positionY = (vertex >> 6u) & 63u;
//   uint positionZ = (vertex >> 12u) & 63u;
//   uint centeroff = (vertex >> 18u) & 7u;
//
//   glm::ivec3 pos =
//       glm::ivec3(BLOCK_SIZE * positionX, BLOCK_SIZE * positionY, BLOCK_SIZE * positionZ);
//   // Center of block
//   glm::ivec3 centercord = Center(pos, centeroff);
//   for (int i = 0; i < 6; i++) {
//     // for each of frusum face check if inside else return 1
//     // TODO:
//   }
//   return 0;
// }

void bind_uniforms() {
  if (vVertex_attrib == -1) {
    vVertex_attrib = glGetAttribLocation(shaderProgram, "vVertex");
    if (vVertex_attrib == -1) {
      fprintf(stderr, "Could not bind location: vVertex\n");
      exit(0);
    }
  }

  if (vNormal_attrib == -1) {
    vNormal_attrib = glGetAttribLocation(shaderProgram2, "vNormal");
    if (vNormal_attrib == -1) {
      std::cout << "Could not bind location: vNormal\n";
      exit(0);
    }
  }

  // Get handle to eye normal variable in shader
  cameraPos_uniform = glGetUniformLocation(shaderProgram2, "cameraPos");
  if (cameraPos_uniform == -1) {
    fprintf(stderr, "Could not bind location: cameraPos. Specular Lighting Switched Off.\n");
  }

  // Moved outside of loop
  lightpos_uniform = glGetUniformLocation(shaderProgram2, "lightpos");
  if (lightpos_uniform == -1) {
    fprintf(stderr, "Could not bind location: lightpos\n");
    exit(0);
  }

  if (side_uniform == -1) {
    side_uniform = glGetUniformLocation(shaderProgram, "side");
    if (side_uniform == -1) {
      fprintf(stderr, "Could not bind location: side\n");
      exit(0);
    }
  }

  atlas_uniform = glGetUniformLocation(shaderProgram, "atlas");
  if (atlas_uniform == -1) {
    std::cerr << "Could not bind: atlas\n";
    exit(0);
  }
  glUniform1f(side_uniform, BLOCK_SIZE);


  // Moved outside of loop
  chunkpos_uniform = glGetUniformLocation(shaderProgram, "chunkpos");
  if (chunkpos_uniform == -1) {
    fprintf(stderr, "Could not bind location: chunkpos\n");
    exit(0);
  }
}

int main(int, char **) {
  // Setup window
  _window = std::make_unique<Window>(SCREEN_WIDTH, SCREEN_HEIGHT);
  ImGuiIO &io = ImGui::GetIO();  // Create IO
  clearColor = ImVec4(0.471f, 0.786f, .784f, 1.00f);

  shaderProgram = createProgram("./shaders/vshader.vs", "./shaders/fshader.fs");
  shaderProgram2 = createProgram("./shaders/vshader2.vs", "./shaders/fshader2.fs");

  glUseProgram(shaderProgram);

  unsigned int axis_VAO;

  Texture atlas("textures/default_texture.png");

  bind_uniforms();
  // createAxesLine(shaderProgram, axis_VAO);
  // Initalize all the players
  for (int i = 0; i < players_cnt; i++) {
    players[i] = std::make_unique<Player>();
  }

  std::unique_ptr<AssetManager> asset_manager = std::make_unique<AssetManager>();

  uint64_t handle1 = asset_manager->loadMeshObject(
      "assets/bunny.obj",
      shaderProgram2,
      0.025,
      0.0,
      glm::vec3(70.0, 70.0, 70.0),
      glm::vec3(1.0, 0.0, 0.0));
  uint64_t handle2 = asset_manager->loadMeshObject(
      "assets/buddha.obj",
      shaderProgram2,
      0.025,
      180.0,
      glm::vec3(70, 70.0, 100.0),
      glm::normalize(glm::vec3(0.0, 1.0, 1.0)));

  auto mesh1 = asset_manager->get_mesh(handle1);
  auto mesh2 = asset_manager->get_mesh(handle2);
  mesh1->setup();
  mesh2->setup();

  meshes.push_back(mesh1);
  meshes.push_back(mesh2);

  while (!glfwWindowShouldClose(_window->GetWindow())) {
    glfwPollEvents();

    if (Input::WasKeyPressed(GLFW_KEY_P)) {
      // Switch Player
      activePlayer += 1;
      activePlayer %= players_cnt;
    }

    // handle player
    players[activePlayer]->update();

    auto playerpos = players[activePlayer]->m_cameracontroller->GetCamera()->GetPosition();
    auto playerdir = players[activePlayer]->m_cameracontroller->GetCamera()->GetOrientation();
    auto fov = players[activePlayer]->m_cameracontroller->GetCamera()->GetHorizontalFOV();

    // World Calculations
    world->SetupWorld(playerpos);

    // Render World with Inter Chunk walls
    world->RenderWorld(true);

    world->DoBindTask();

    world->Update_queue(playerpos, playerdir, fov);
    // glBindVertexArray(cube_VAO);
    atlas.Bind();
    glUniform1i(atlas_uniform, 0);  // bind sampler to texture unit 0
    world->Draw();
    glDisable(GL_DEPTH_TEST);  // Disable depth test for drawing axes. We want
                               // axes to be drawn on top of all

    // draw_axis(axis_VAO, shaderProgram);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(_window->GetWindow());
  }

  // Cleanup
  cleanup(_window->GetWindow());
  return 0;
}
