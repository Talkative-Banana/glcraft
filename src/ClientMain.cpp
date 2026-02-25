#include "Utils.h"

#define GLM_FORCE_RADIANS
#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
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
#include "GameSound.hpp"
#include "Input.h"
#include "Main.h"
#include "ParticleSystem.h"
#include "Player.h"
#include "RainEffect.h"
#include "Ray.h"
#include "Renderer.h"
#include "SmokeEffect.h"
#include "SnowEffect.h"
#include "SoundSystem.hpp"
#include "Texture.h"
#include "UI.h"
#include "WaterEffect.h"
#include "World.h"
#include <boost/asio.hpp>
#include <random>
#include <unordered_set>

// Globals
glm::ivec3 _wps = {0, 0, 0};
std::unique_ptr<UI> ui = nullptr;
std::unique_ptr<ParticleSystem> water_ps, smoke_ps, snow_ps, rain_ps;
std::unique_ptr<SoundSystem> ss = nullptr;
std::unique_ptr<World> world = nullptr;
std::unique_ptr<Window> _window = nullptr;
glm::vec3 chunkpos;
GLuint activePlayer, players_cnt = 2;
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
glm::mat4 modelT, viewT,
    projectionT; // The model, view and projection transformations
std::vector<std::shared_ptr<Mesh>> meshes;
std::array<std::unique_ptr<Player>, PLAYER_COUNT> players;
std::unique_ptr<AssetManager> asset_manager;
extern std::mutex m;
extern std::queue<glm::ivec3> refreshq;
std::vector<WorldState> client_operations;

// void createAxesLine(unsigned int &, unsigned int &);
ImVec4 clearColor = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

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
  if (cameraPos_uniform == -1) {
    cameraPos_uniform = glGetUniformLocation(shaderProgram2, "cameraPos");
    if (cameraPos_uniform == -1) {
      fprintf(stderr, "Could not bind location: cameraPos. Specular Lighting "
                      "Switched Off.\n");
      exit(0);
    }
  }

  // Moved outside of loop
  if (lightpos_uniform == -1) {
    lightpos_uniform = glGetUniformLocation(shaderProgram2, "lightpos");
    if (lightpos_uniform == -1) {
      fprintf(stderr, "Could not bind location: lightpos\n");
      exit(0);
    }
  }

  if (side_uniform == -1) {
    side_uniform = glGetUniformLocation(shaderProgram, "side");
    if (side_uniform == -1) {
      fprintf(stderr, "Could not bind location: side\n");
      exit(0);
    }
  }

  if (atlas_uniform == -1) {
    atlas_uniform = glGetUniformLocation(shaderProgram, "atlas");
    if (atlas_uniform == -1) {
      std::cerr << "Could not bind: atlas\n";
      exit(0);
    }
  }

  if (skyColor_uniform == -1) {
    skyColor_uniform = glGetUniformLocation(shaderProgram, "skyColor");
    if (skyColor_uniform == -1) {
      std::cerr << "Could not bind: skyColor\n";
      exit(0);
    }
  }

  glUniform1f(side_uniform, BLOCK_SIZE);

  if (chunkpos_uniform == -1) {
    chunkpos_uniform = glGetUniformLocation(shaderProgram, "chunkpos");
    if (chunkpos_uniform == -1) {
      fprintf(stderr, "Could not bind location: chunkpos\n");
      exit(0);
    }
  }

  if (quadpos_uniform == -1) {
    quadpos_uniform = glGetAttribLocation(shaderProgramUI, "quadpos");
    if (quadpos_uniform == -1) {
      fprintf(stderr, "Could not bind location: quadpos");
      exit(0);
    }
  }

  if (ui_uniform == -1) {
    ui_uniform = glGetUniformLocation(shaderProgramUI, "UICOMP");
    if (ui_uniform == -1) {
      std::cerr << "Could not bind: UI\n";
      exit(0);
    }
  }

  if (uProjLoc_uniform == -1) {
    uProjLoc_uniform = glGetUniformLocation(shaderProgramUI, "uProj");
    if (uProjLoc_uniform == -1) {
      std::cerr << "Could not bind uniform uProj in UI\n";
      exit(0);
    } else {
      uProjLoc_uniform = glGetUniformLocation(shaderProgramPS, "uProj");
      if (uProjLoc_uniform == -1) {
        std::cerr << "Could not bind uniform uProj in PS\n";
        exit(0);
      }
    }
  }
}

uint32_t allocatePlayerId() {
  static std::mt19937 rng{std::random_device{}()};
  static std::uniform_int_distribution<int> dist(1, PLAYER_COUNT);
  return dist(rng);
}

void updatePlayer(const std::string &msg) {
  // Received update from server
  State st;
  std::memcpy(&st, msg.data(), sizeof(State));

  if (std::holds_alternative<PlayerState>(st._data)) {
    PlayerState pst = std::get<PlayerState>(st._data);
    if ((st.id == activePlayer) && (!st.enforce))
      return; // do not update my state unless explicitly asked by server
    players[st.id]->handleNetworkRequest(pst);
  } else if (std::holds_alternative<WorldState>(st._data)) {
    WorldState wst = std::get<WorldState>(st._data);
    if ((st.id == activePlayer) && (st.enforce)) {
      return; // do not update my world state alreay did
    }
    auto chunk = world->get_chunk_by_center(wst.blockpos);
    if (chunk && chunk->chunkva) {
      // check if block within render distance
      world->handleNetworkRequest(wst);
    } else {
      // if not will apply change when block within render distance
      std::cout << "Skipping update chunk not loaded yet\n";
      client_operations.push_back(wst);
    }
  } else {
    std::cerr << "Invalid State Message\n";
  }
}

int main(int, char **) {
  // Setup window
  _window = std::make_unique<Window>(SCREEN_WIDTH, SCREEN_HEIGHT);
  ImGuiIO &io = ImGui::GetIO(); // Create IO

  // create UI Render
  ui = std::make_unique<UI>();

  water_ps = std::make_unique<ParticleSystem>("./textures/water.png");
  smoke_ps = std::make_unique<ParticleSystem>("./textures/smoke.png");
  snow_ps = std::make_unique<ParticleSystem>("./textures/snow.png");
  rain_ps = std::make_unique<ParticleSystem>("./textures/rain.png");

  ss = std::make_unique<SoundSystem>();
  world = std::make_unique<World>(42, _wps);

  asset_manager = std::make_unique<AssetManager>();

  shaderProgram =
      createProgram("./shaders/vshaderWorld.vs", "./shaders/fshaderWorld.fs");
  shaderProgram2 =
      createProgram("./shaders/vshaderAsset.vs", "./shaders/fshaderAsset.fs");
  shaderProgramUI =
      createProgram("./shaders/vshaderUI.vs", "./shaders/fshaderUI.fs");
  shaderProgramPS =
      createProgram("./shaders/vshaderPS.vs", "./shaders/fshaderPS.fs");

  glUseProgram(shaderProgram);

  unsigned int axis_VAO;

  Texture atlas("textures/default_texture.png");

  UIComponent uicomp1 =
      UIComponent("textures/gauge.png", glm::vec2(256.0, 256.0), 512);
  ui->add_component(uicomp1);
  ui->Render();

  std::unique_ptr<SmokeEffect> smoke_effect =
      std::make_unique<SmokeEffect>(128);
  std::unique_ptr<SnowEffect> snow_effect = std::make_unique<SnowEffect>(256);
  std::unique_ptr<RainEffect> rain_effect = std::make_unique<RainEffect>(512);
  std::unique_ptr<WaterEffect> water_effect = std::make_unique<WaterEffect>(1);

  smoke_effect->setup();
  snow_effect->setup();
  rain_effect->setup();
  water_effect->setup();

  auto sm_id = smoke_ps->add_effect(std::move(smoke_effect));
  auto sn_id = snow_ps->add_effect(std::move(snow_effect));
  auto we_id = water_ps->add_effect(std::move(water_effect));
  auto rn_id = rain_ps->add_effect(std::move(rain_effect));

  water_ps->Render();
  rain_ps->Render();
  snow_ps->Render();

  bind_uniforms();

  boost::asio::io_context ioc;
  // createAxesLine(shaderProgram, axis_VAO);
  uint32_t randomPlayerID = allocatePlayerId();
  activePlayer = randomPlayerID;

  for (int i = 0; i < PLAYER_COUNT; i++) {
    if (i == randomPlayerID) {
      auto ptr =
          std::make_unique<Player>(randomPlayerID, shaderProgram2, "127.0.0.1",
                                   "8080", ioc, updatePlayer);
      players[randomPlayerID] = std::move(ptr);
    } else {
      players[i] = std::make_unique<Player>(i, shaderProgram2);
    }
  }
  std::thread networking_thread = std::thread([&ioc]() { ioc.run(); });

  // uint64_t handle1 = asset_manager->loadMeshObject(
  //     "assets/bunny.obj",
  //     shaderProgram2,
  //     0.025,
  //     0.0,
  //     glm::vec3(70.0, 70.0, 70.0),
  //     glm::vec3(1.0, 0.0, 0.0));
  // uint64_t handle2 = asset_manager->loadMeshObject(
  //     "assets/buddha.obj",
  //     shaderProgram2,
  //     0.025,
  //     180.0,
  //     glm::vec3(70, 70.0, 100.0),
  //     glm::normalize(glm::vec3(0.0, 1.0, 1.0)));
  //
  // auto mesh1 = asset_manager->get_mesh(handle1);
  // auto mesh2 = asset_manager->get_mesh(handle2);
  // mesh1->setup();
  // mesh2->setup();
  //
  // meshes.push_back(mesh1);
  // meshes.push_back(mesh2);

  // Audio Setup [Maintain Lifetime]
  GameSound thundersound("assets/audio/calm-thunderstorm-mono.wav");
  GameSound blizzardsound("assets/audio/blizzard.ogg");

  ss->AddSound("thunderstorm", thundersound);
  ss->AddSound("blizzard", blizzardsound);

  glm::mat4 uiProj;
  float last = glfwGetTime();
  while (!glfwWindowShouldClose(_window->GetWindow())) {
    float current = glfwGetTime();
    float dt = current - last;
    last = current;

    glfwPollEvents();

    // handle player
    auto &player = players[activePlayer];
    player->update(dt);

    auto playerpos = player->m_cameracontroller->GetCamera()->GetPosition();
    auto playerdir = players[activePlayer]
                         ->m_cameracontroller->GetCamera()
                         ->GetOrientation();

    sf::Listener::setPosition({playerpos.x, playerpos.y, playerpos.z});
    sf::Listener::setDirection({playerdir.x, playerdir.y, playerdir.z});
    auto playervp =
        player->m_cameracontroller->GetCamera()->GetProjectionViewMatrix();

    {
      std::lock_guard<std::mutex> l{m};
      if (!refreshq.empty()) {
        auto raycord = refreshq.front();
        refreshq.pop();
        world->RefreshChunks(raycord);
      }
    }

    for (size_t i = 0; i < client_operations.size();) {
      auto &wst = client_operations[i];
      auto chunk = world->get_chunk_by_center(wst.blockpos);

      if (chunk && chunk->chunkva) {
        world->handleNetworkRequest(wst);

        std::swap(client_operations[i], client_operations.back());
        client_operations.pop_back();
      } else {
        ++i;
      }
    }
    // World Calculations
    world->EnqueueVisibleBiomes(playerpos);

    // Setup biomes [first pass]
    world->SetupBiomesPass1();

    // Do Binding for first pass
    world->DoBindTask(true);

    // Setup biomes [second pass]
    world->SetupBiomesPass2();

    // Do Binding for second pass
    world->DoBindTask(false);

    world->Update_queue(playerpos, playervp);
    // glBindVertexArray(cube_VAO);
    atlas.Bind();
    glUniform1i(atlas_uniform, 0); // bind sampler to texture unit 0
    // skyColor.Bind();
    glUniform3f(skyColor_uniform, clearColor.x, clearColor.y,
                clearColor.z); // bind sampler to texture unit 0

    int fbw, fbh;
    glfwGetFramebufferSize(_window->GetWindow(), &fbw, &fbh);
    uiProj = glm::ortho(0.0f, (float)fbw, 0.0f, (float)fbh);

    auto *effect = water_ps->get_effect(we_id);

    if (players[activePlayer]->InsideBlock() == BLOCK_TYPE::WATER_BLOCK) {
      effect->set_initial_size(std::max(fbw, fbh));
      effect->effect_visible = true;
    } else {
      effect->effect_visible = false;
    }

    auto *rain_effect = rain_ps->get_effect(rn_id);
    rain_effect->set_position(fbw, fbh);
    rain_effect->set_vel_y_factor(-fbh / 2.0);
    rain_effect->fraction = (static_cast<float>(fbw) / 3840.0f);

    auto *snow_effect = snow_ps->get_effect(sn_id);
    snow_effect->set_position(fbw, fbh);
    snow_effect->set_vel_y_factor(-fbh / 2.0);
    snow_effect->set_vel_x_factor(-fbh / 2.0);
    snow_effect->fraction = (static_cast<float>(fbw) / 3840.0f);

    auto &player_dir = players[activePlayer]
                           ->m_cameracontroller->GetCamera()
                           ->GetOrientation();
    if (players[activePlayer]->InsideBlock() == BLOCK_TYPE::WATER_BLOCK) {
      rain_effect->effect_visible = false;
      snow_effect->effect_visible = false;
    } else {
      rain_effect->effect_visible = true;
      snow_effect->effect_visible = true;
    }

    // OPAQUE PASS
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    world->Draw(OBJ_TYPE::OPAQUE_);

    // TRANSPARENT PASS
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    world->Draw(OBJ_TYPE::TRANSPARENT_);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // PARTICLE SYSTEM
    glUseProgram(shaderProgramPS);

    glUniformMatrix4fv(uProjLoc_uniform, 1, GL_FALSE, glm::value_ptr(uiProj));
    water_ps->Draw(dt);

    if (world->getWeather() == WEATHER::HAILSTORM) {
      rain_ps->Draw(dt);
      if (thundersound.getSoundStatus() == SOUNDSTATUS::PAUSED)
        thundersound.get_sound().play();
      if (blizzardsound.getSoundStatus() == SOUNDSTATUS::PLAYING)
        blizzardsound.get_sound().pause();
    } else if (world->getWeather() == WEATHER::SNOWSTORM) {
      snow_ps->Draw(dt);
      if (blizzardsound.getSoundStatus() == SOUNDSTATUS::PAUSED)
        blizzardsound.get_sound().play();
      if (thundersound.getSoundStatus() == SOUNDSTATUS::PLAYING)
        thundersound.get_sound().pause();
    } else {
      if (blizzardsound.getSoundStatus() == SOUNDSTATUS::PLAYING)
        blizzardsound.get_sound().pause();
      if (thundersound.getSoundStatus() == SOUNDSTATUS::PLAYING)
        thundersound.get_sound().pause();
    }

    // UI PASS (Keep it at last)
    glUseProgram(shaderProgramUI);

    glUniformMatrix4fv(uProjLoc_uniform, 1, GL_FALSE, glm::value_ptr(uiProj));
    // ui->Draw();

    // Restore
    glDepthMask(GL_TRUE);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
    // draw_axis(axis_VAO, shaderProgram);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(_window->GetWindow());
  }

  ioc.stop();
  networking_thread.join();

  // Cleanup
  cleanup(_window->GetWindow());
  return 0;
}
