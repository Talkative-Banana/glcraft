#include <array>
#include <memory>

#include "CameraController.h"
#include "Chunk.h"
#include "Mesh.h"
#include "Ray.h"
#include "Utils.h"
#include "World.h"
#pragma once

extern std::unique_ptr<Window> _window;
extern std::unique_ptr<World> world;
extern GLuint activePlayer, players_cnt, shaderProgram;
extern GLint vModel_uniform, vView_uniform, vProjection_uniform, side_uniform, chunkpos_uniform,
    vColor_uniform, atlas_uniform, lightpos_uniform, cameraPos_uniform;
extern glm::mat4 modelT, viewT, projectionT;  // The model, view and projection transformations
extern ImVec4 clearColor;
extern std::vector<std::shared_ptr<Mesh>> meshes;

class Player {
 private:
  glm::vec3 m_position = glm::vec3(70.0, 100.0, 85.0);
  glm::vec3 m_forward = glm::vec3(0.0, 0.0, 1.0);
  glm::vec3 m_velocity = glm::vec3(0.0, 0.0, 0.0);
  glm::vec3 m_up = glm::vec3(0.0, 1.0, 0.0);
  float m_speed = 4.48f;
  float m_sensitivity = 0.002f;
  glm::vec2 MousePos = {0.0f, 0.0f};
  bool enable_gravity = 1;
  char textKeyStatus[IMGUI_TEXT_CAPACITY];
  char textKeyDescription[IMGUI_TEXT_CAPACITY];
  int display_w, display_h;
  GLFWwindow* window;
  GLuint bltype = 0, mdtype = 0, Nokeypressed = 0;

 public:
  std::unique_ptr<CameraController> m_cameracontroller;

 public:
  Player(const glm::vec3& pos, const glm::vec3& dir);
  Player();

  void handle_input();
  void update();
  void handle_stats();
  void handle_transformations();
  void setupModelTransformationCube(unsigned int& program);
  void setupModelTransformationAxis(unsigned int& program, float rot_angle, glm::vec3 rot_axis);
  void setupViewTransformation(unsigned int& program, std::unique_ptr<CameraController>& occ);
  void setupProjectionTransformation(unsigned int& program, std::unique_ptr<CameraController>& occ);
};
