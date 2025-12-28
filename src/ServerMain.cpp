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
#include "Main.h"
#include "Player.h"
#include "Server.h"
#include "World.h"
#include <atomic>
#include <boost/asio.hpp>
// Globals
glm::ivec3 _wps = {0, 0, 0};
std::unique_ptr<World> world = nullptr;
std::unique_ptr<Window> _window = nullptr;
glm::vec3 chunkpos;
GLuint activePlayer = 0, players_cnt = 2;
GLint vModel_uniform = -1;
GLint vView_uniform = -1;
GLint vProjection_uniform = -1;
GLint side_uniform = -1;
GLint chunkpos_uniform = -1;
GLint vColor_uniform = -1;
GLint vVertex_attrib = -1;
GLint vNormal_attrib = -1;
GLint cameraPos_uniform = -1;
GLint lightpos_uniform = -1;
GLint atlas_uniform = -1;
GLint ui_uniform = -1;
GLint skyColor_uniform = -1;
GLint quadpos_uniform = -1;
GLint uProjLoc_uniform = -1;
GLuint wireframemode, shaderProgram, shaderProgram2, shaderProgramUI,
    shaderProgramPS;
glm::mat4 modelT, viewT, projectionT;
std::vector<std::shared_ptr<Mesh>> meshes;
std::array<std::unique_ptr<Player>, PLAYER_COUNT> players;
std::unique_ptr<AssetManager> asset_manager = std::make_unique<AssetManager>();

// void createAxesLine(unsigned int &, unsigned int &);
ImVec4 clearColor = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

int main(int, char **) {
  // Setup world
  world = std::make_unique<World>(42, _wps);

  // Initalize all potential player
  for (int i = 0; i < PLAYER_COUNT; i++) {
    players[i] = std::make_unique<Player>(i);
  }

  boost::asio::io_context ioc;
  std::make_shared<Server>(ioc, 8080)->on_recv([](const std::string &msg) {
    // handle player
    PlayerState pt;
    std::memcpy(&pt, msg.data(), sizeof(PlayerState));

    // set the id of the activePlayer to mimic their movement here
    // activePlayer = pt.id;
    return players.at(pt.id)->handle_client_input(msg);
  });

  std::thread networking_thread = std::thread([&ioc]() { ioc.run(); });

  glm::mat4 uiProj;
  float last = glfwGetTime();

  while (1) {
    float current = glfwGetTime();
    float dt = current - last;
    last = current;

    auto &player = players.at(activePlayer);
    auto playerpos = player->m_cameracontroller->GetCamera()->GetPosition();
    auto playerdir = player->m_cameracontroller->GetCamera()->GetOrientation();
    auto fov = player->m_cameracontroller->GetCamera()->GetHorizontalFOV();

    // World Calculations
    world->SetupWorld(playerpos);
    // Render first pass
    world->RenderWorld(true);
    // Do Binding for first pass
    // world->DoBindTask(true);

    // Render second pass
    world->RenderWorld(false);
    // Do Binding for second pass
    // world->DoBindTask(false);

    world->Update_queue(playerpos, playerdir, fov);
  }

  ioc.stop();
  networking_thread.join();
  return 0;
}
