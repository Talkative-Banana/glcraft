#include <array>
#include <memory>

#include "AssetManager.h"
#include "CameraController.h"
#include "Chunk.h"
#include "Client.h"
#include "Inventory.h"
#include "Mesh.h"
#include "Ray.h"
#include "Utils.h"
#include "World.h"
#include <boost/asio.hpp>
#pragma once

extern std::unique_ptr<Window> _window;
extern std::unique_ptr<World> world;
extern std::unique_ptr<AssetManager> asset_manager;
extern GLuint activePlayer, players_cnt, shaderProgram, shaderProgram2;
extern GLint vModel_uniform, vView_uniform, vProjection_uniform, side_uniform,
    chunkpos_uniform, vColor_uniform, atlas_uniform, lightpos_uniform,
    cameraPos_uniform;
extern glm::mat4 modelT, viewT,
    projectionT; // The model, view and projection transformations
extern ImVec4 clearColor;
extern std::vector<std::shared_ptr<Mesh>> meshes;
extern void add_player(uint32_t);

class Player {
public:
  std::unique_ptr<CameraController> m_cameracontroller;
  Player(const uint64_t);
  Player(const uint64_t, const uint64_t);
  Player(const uint64_t, const glm::vec3 &, const glm::vec3 &, const uint64_t);
  Player(const uint64_t, const uint64_t, const std::string &,
         const std::string &, boost::asio::io_context &,
         std::function<void(const std::string &)>);

  void handle_input(float);
  std::shared_ptr<std::string> handle_client_input(const std::string &);
  void update(float);
  void handle_stats();
  void handleNetworkRequest(PlayerState &st);
  void handle_transformations();
  void setupModelTransformationCube(unsigned int &);
  void setupModelTransformationAxis(unsigned int &, float, glm::vec3);
  void setupViewTransformation(unsigned int &,
                               std::unique_ptr<CameraController> &);
  void setupProjectionTransformation(unsigned int &,
                                     std::unique_ptr<CameraController> &);
  Client *const get_client();
  const std::string get_state();
  bool Valid(PlayerState &);
  BLOCK_TYPE InsideBlock();

private:
  glm::vec3 m_position = glm::vec3(70.0, 100.0, 85.0);
  glm::vec3 m_forward = glm::vec3(0.0, 0.0, 1.0);
  glm::vec3 m_velocity = glm::vec3(0.0, 0.0, 0.0);
  glm::vec3 m_up = glm::vec3(0.0, 1.0, 0.0);
  float m_speed = WALKING_SPEED;
  float m_sensitivity = 0.002f;
  glm::vec2 MousePos = {0.0f, 0.0f};
  bool enable_gravity = 1;
  char textKeyStatus[IMGUI_TEXT_CAPACITY] = {0};
  char textKeyDescription[IMGUI_TEXT_CAPACITY] = {0};
  int display_w, display_h;
  GLFWwindow *window;
  GLuint bltype = 0, mdtype = 0, Nokeypressed = 0, m_id = 0;
  uint64_t m_meshhandle;
  Inventory m_inventory;
  BLOCK_TYPE inside_block;
  std::unique_ptr<Client> m_client = nullptr;
};
