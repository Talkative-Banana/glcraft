#define GLM_FORCE_RADIANS
#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "Game.hpp"
#include "Window.h"
// Globals
glm::ivec3 _wps = {0, 0, 0};
std::unique_ptr<World> world = nullptr;
std::unique_ptr<Window> _window = nullptr;
glm::dvec3 chunkpos;
GLuint activePlayer, players_cnt = 2;
GLuint wireframemode;
// The model, view and projection transformations
glm::dmat4 modelT, viewT, viewRotateT, projectionT;
std::vector<std::shared_ptr<Mesh>> meshes;
std::array<std::unique_ptr<Player>, PLAYER_COUNT> players;
std::unique_ptr<AssetManager> asset_manager;
extern std::mutex m;
extern std::queue<glm::ivec3> refreshq;
std::vector<WorldState> client_operations;
std::vector<std::string> world_operations;
// void createAxesLine(unsigned int &, unsigned int &);
ImVec4 clearColor = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

// dummy implementation
void add_player(uint32_t id) {}

GLint vModel_uniform = -1;
GLint vView_uniform = -1;
GLint vProjection_uniform = -1;
GLint side_uniform = -1;
GLint chunkpos_uniform = -1;
GLint vColor_uniform = -1;
GLint vVertex_attrib = -1;
GLint vNormal_attrib = -1;
GLint atlas_uniform = -1;
GLint ui_uniform = -1;
GLint skyColor_uniform = -1;
GLint quadpos_uniform = -1;
GLint uProjLoc_uniformUI = -1;
GLint uProjLoc_uniformPS = -1;

void Game::bindUniforms() {
  if (vVertex_attrib == -1) {
    vVertex_attrib = glGetAttribLocation(m_shaderProgram, "vVertex");
    if (vVertex_attrib == -1) {
      fprintf(stderr, "Could not bind location: vVertex\n");
      exit(0);
    }
  }

  if (vNormal_attrib == -1) {
    vNormal_attrib = glGetAttribLocation(m_shaderProgram2, "vNormal");
    if (vNormal_attrib == -1) {
      std::cout << "Could not bind location: vNormal\n";
      exit(0);
    }
  }

  if (side_uniform == -1) {
    side_uniform = glGetUniformLocation(m_shaderProgram, "side");
    if (side_uniform == -1) {
      fprintf(stderr, "Could not bind location: side\n");
      exit(0);
    }
  }

  if (atlas_uniform == -1) {
    atlas_uniform = glGetUniformLocation(m_shaderProgram, "atlas");
    if (atlas_uniform == -1) {
      std::cerr << "Could not bind: atlas\n";
      exit(0);
    }
  }

  if (skyColor_uniform == -1) {
    skyColor_uniform = glGetUniformLocation(m_shaderProgram, "skyColor");
    if (skyColor_uniform == -1) {
      std::cerr << "Could not bind: skyColor\n";
      exit(0);
    }
  }

  glUseProgram(m_shaderProgram);
  glUniform1f(side_uniform, BLOCK_SIZE);

  if (chunkpos_uniform == -1) {
    chunkpos_uniform = glGetUniformLocation(m_shaderProgram, "chunkpos");
    if (chunkpos_uniform == -1) {
      fprintf(stderr, "Could not bind location: chunkpos\n");
      exit(0);
    }
  }

  if (quadpos_uniform == -1) {
    quadpos_uniform = glGetAttribLocation(m_shaderProgramUI, "quadpos");
    if (quadpos_uniform == -1) {
      fprintf(stderr, "Could not bind location: quadpos");
      exit(0);
    }
  }

  if (ui_uniform == -1) {
    ui_uniform = glGetUniformLocation(m_shaderProgramUI, "UICOMP");
    if (ui_uniform == -1) {
      std::cerr << "Could not bind: UI\n";
      exit(0);
    }
  }

  if (uProjLoc_uniformUI == -1) {
    uProjLoc_uniformUI = glGetUniformLocation(m_shaderProgramUI, "uProjUI");
    if (uProjLoc_uniformUI == -1) {
      std::cerr << "Could not bind uniform uProj in UI\n";
      exit(0);
    }
  }

  if (uProjLoc_uniformPS == -1) {
    uProjLoc_uniformPS = glGetUniformLocation(m_shaderProgramPS, "uProjPS");
    if (uProjLoc_uniformPS == -1) {
      std::cerr << "Could not bind uniform uProj in PS\n";
      exit(0);
    }
  }
}

// void draw_axis(unsigned int axis_VAO, unsigned int shaderProgram) {
//   glBindVertexArray(axis_VAO);
//   setupModelTransformationAxis(shaderProgram, 0.0, glm::vec3(0, 0, 1));
//   // glUniform4f(vColor_uniform, 1.0, 0.0, 0.0, 1.0); //Red -> X
//   glDrawArrays(GL_LINES, 0, 2);
//
//   setupModelTransformationAxis(shaderProgram, glm::radians(90.0),
//   glm::vec3(0, 0, 1));
//   // glUniform4f(vColor_uniform, 0.0, 1.0, 0.0, 1.0); //Green -> Y
//   glDrawArrays(GL_LINES, 0, 2);
//
//   setupModelTransformationAxis(shaderProgram, -glm::radians(90.0),
//   glm::vec3(0, 1, 0));
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
//   glBufferData(GL_ARRAY_BUFFER, nVertices * 3 * sizeof(GLfloat),
//   axis_vertices, GL_STATIC_DRAW);
//   glEnableVertexAttribArray(vVertex_attrib_position);
//   glVertexAttribPointer(vVertex_attrib_position, 3, GL_FLOAT, GL_FALSE, 0,
//   0);
//
//   glBindBuffer(GL_ARRAY_BUFFER, 0);
//   glBindVertexArray(0);  // Unbind the VAO to disable changes outside this
//   function.
// }

uint32_t Game::allocatePlayerId() {
  static std::mt19937 rng{std::random_device{}()};
  static std::uniform_int_distribution<int> dist(1, PLAYER_COUNT);
  return dist(rng);
}

void Game::updatePlayer(const std::string &msg) {
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
    if (auto chunk = world->get_chunk_by_center(wst.blockpos).lock()) {
      if (chunk && chunk->m_chunkVa) {
        // check if block within render distance
        world->handleNetworkRequest(wst);
      } else {
        // if not will apply change when block within render distance
        std::cout << "Skipping update chunk not loaded yet\n";
        client_operations.push_back(wst);
      }
    }
  } else {
    std::cerr << "Invalid State Message\n";
  }
}

Game::Game(std::string savePath)
    : m_savePath(savePath), m_thundersound(ThunderPath),
      m_blizzardsound(BlizzardPath) {
  std::cout << "Size of a block: " << sizeof(Block) << '\n';
  std::cout << "Size of a chunk: " << sizeof(Chunk) << '\n';
  std::cout << "Size of a biome: " << sizeof(Biome) << '\n';

  // Create Asset Manager
  asset_manager = std::make_unique<AssetManager>();

  // Create World
  world = std::make_unique<World>(42, _wps);

#if defined(BUILD_CLIENT) || defined(BUILD_LOCAL)
  // Setup window
  _window = std::make_unique<Window>(SCREEN_WIDTH, SCREEN_HEIGHT);

  ImGuiIO &io = ImGui::GetIO(); // Create IO
  m_shaderProgram =
      createProgram(ShaderProgramVS.c_str(), ShaderProgramFS.c_str());
  m_shaderProgram2 =
      createProgram(ShaderProgram2VS.c_str(), ShaderProgram2FS.c_str());
  m_shaderProgramUI =
      createProgram(ShaderProgramUIVS.c_str(), ShaderProgramUIFS.c_str());
  m_shaderProgramPS =
      createProgram(ShaderProgramPSVS.c_str(), ShaderProgramPSFS.c_str());

  bindUniforms();

  // Create UI Render
  m_ui = std::make_unique<UI>();

  m_waterPs = std::make_unique<ParticleSystem>(WaterTexture);
  m_smokePs = std::make_unique<ParticleSystem>(SmokeTexture);
  m_snowPs = std::make_unique<ParticleSystem>(SnowTexture);
  m_rainPs = std::make_unique<ParticleSystem>(RainTexture);

  // Create Sound System
  m_ss = std::make_unique<SoundSystem>();

  // Create Effects
  std::unique_ptr<SmokeEffect> smokeEffect = std::make_unique<SmokeEffect>(128);
  std::unique_ptr<SnowEffect> snowEffect = std::make_unique<SnowEffect>(256);
  std::unique_ptr<RainEffect> rainEffect = std::make_unique<RainEffect>(512);
  std::unique_ptr<WaterEffect> waterEffect = std::make_unique<WaterEffect>(1);

  // Setup Effects
  smokeEffect->setup();
  snowEffect->setup();
  rainEffect->setup();
  waterEffect->setup();

  // Set effects id
  m_smId = m_smokePs->add_effect(std::move(smokeEffect));
  m_snId = m_snowPs->add_effect(std::move(snowEffect));
  m_weId = m_waterPs->add_effect(std::move(waterEffect));
  m_rnId = m_rainPs->add_effect(std::move(rainEffect));

  // Setup ParticleSystems
  m_smokePs->Setup();
  m_waterPs->Setup();
  m_rainPs->Setup();
  m_snowPs->Setup();

  // Audio Setup [Maintain Lifetime]
  m_ss->AddSound("thunder", m_thundersound);
  m_ss->AddSound("blizzard", m_blizzardsound);

  glUseProgram(m_shaderProgramUI);
  UIComponent uicomp = UIComponent(GaugeTexture, glm::vec2(256.0, 256.0), 512);
  auto c_id = m_ui->add_component(std::move(uicomp));
  m_ui->Setup();

  // update atlas
  glUseProgram(m_shaderProgram);
  m_atlas = std::make_unique<Texture>(DefaultTexture);
#endif

#ifdef BUILD_LOCAL
  // Intialize all players
  for (int i = 0; i < players_cnt; i++) {
    players[i] = std::make_unique<Player>(i, m_shaderProgram, m_shaderProgram2);
  }
#endif

#ifdef BUILD_CLIENT
  uint32_t randomPlayerID = allocatePlayerId();
  activePlayer = randomPlayerID;
  for (int i = 0; i < PLAYER_COUNT; i++) {
    if (i == randomPlayerID) {
      auto ptr = std::make_unique<Player>(randomPlayerID, m_shaderProgram,
                                          m_shaderProgram2, "127.0.0.1", "8080",
                                          m_ioc, Game::updatePlayer);
      players[randomPlayerID] = std::move(ptr);
    } else {
      players[i] =
          std::make_unique<Player>(i, m_shaderProgram, m_shaderProgram2);
    }
  }
  m_networkingThread = std::thread([this]() { m_ioc.run(); });
#endif

#ifdef BUILD_SERVER
  // Initalize all potential player
  for (int i = 0; i < PLAYER_COUNT; i++) {
    players[i] = std::make_unique<Player>(i, m_shaderProgram2);
  }

  std::make_shared<Server>(m_ioc, m_port)->on_recv([](const std::string &msg) {
    // handle request
    State st;
    std::memcpy(&st, msg.data(), sizeof(State));
    if (std::holds_alternative<PlayerState>(st._data)) {
      PlayerState pst = std::get<PlayerState>(st._data);
      // set the id of the activePlayer to mimic their movement here
      // activePlayer = pt.id;
      return players.at(st.id)->handle_client_input(msg);
    } else if (std::holds_alternative<WorldState>(st._data)) {
      WorldState wst = std::get<WorldState>(st._data);
      return world->handle_client_input(msg);
    } else {
      std::cerr << "Invalid Packet Received\n";
      return std::make_shared<std::string>("Invalid Message");
    }
  });

  m_networkingThread = std::thread([this]() {
    std::cout << "Server Listening on port: " << m_port << '\n';
    m_ioc.run();
  });
#endif

  // uint64_t handle1 = asset_manager->loadMeshObject(
  //     BunnyModel, shaderProgram2, 0.025, 0.0,
  //     glm::vec3(70.0, 70.0, 70.0), glm::vec3(1.0, 0.0, 0.0));
  // uint64_t handle2 = asset_manager->loadMeshObject(
  //     BuddhaModel, shaderProgram2, 0.025, 180.0,
  //     glm::vec3(70, 70.0, 100.0), glm::normalize(glm::vec3(0.0, 1.0, 1.0)));
  //
  // std::weak_ptr<Mesh> mesh1 = asset_manager->get_mesh(handle1);
  // std::weak_ptr<Mesh> mesh2 = asset_manager->get_mesh(handle2);
  // if (auto s_mesh1 = mesh1.lock()) {
  //   s_mesh1->setup();
  //   meshes.push_back(s_mesh1);
  // }
  //
  // if (auto s_mesh2 = mesh2.lock()) {
  //   s_mesh2->setup();
  //   meshes.push_back(s_mesh2);
  // }
}

void Game::run() {
  glm::mat4 uiProj;
  float last = glfwGetTime();

#ifndef BUILD_SERVER
  while (!glfwWindowShouldClose(_window->GetWindow())) {
    glfwPollEvents();
#else
  while (1) {
#endif

#ifdef BUILD_LOCAL
    if (Input::WasKeyPressed(GLFW_KEY_P)) {
      // Switch Player
      activePlayer += 1;
      activePlayer %= players_cnt;
    }
#endif

    float current = glfwGetTime();
    float dt = current - last;
    last = current;
    // handle player
    auto &player = players.at(activePlayer);
    auto playerpos = player->m_cameracontroller->GetCamera()->GetPosition();
    auto playerdir = player->m_cameracontroller->GetCamera()->GetOrientation();
    glm::dmat4 playervp =
        player->m_cameracontroller->GetCamera()->GetProjectionViewMatrix();

#if defined(BUILD_CLIENT) || defined(BUILD_LOCAL)
    player->update(dt);
    sf::Listener::setPosition(
        {float(playerpos.x), float(playerpos.y), float(playerpos.z)});
    sf::Listener::setDirection(
        {float(playerdir.x), float(playerdir.y), float(playerdir.z)});
#endif

#ifdef BUILD_CLIENT
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

      if (auto chunk = world->get_chunk_by_center(wst.blockpos).lock()) {
        if (chunk && chunk->m_chunkVa) {
          world->handleNetworkRequest(wst);

          std::swap(client_operations[i], client_operations.back());
          client_operations.pop_back();
        } else {
          ++i;
        }
      }
    }
#endif
    // World Calculations
    world->EnqueueVisibleBiomes(playerpos);

    // Setup biomes [first pass]
    world->SetupBiomesPass1();

    // Do Binding for [first pass]
    world->MarkBiomesReadyForPass1();

    // Setup biomes [second pass]
    world->SetupBiomesPass2();

    // Do Binding for second pass
    world->MarkBiomesReadyForPass2();

    // Mark all the waiting biomes ready
    world->MarkBiomesReadyForBoundaryRemoval();

    world->Update_queue(playerpos, playervp);
    // glBindVertexArray(cube_VAO);

#if defined(BUILD_CLIENT) || defined(BUILD_LOCAL)
    int fbw, fbh;
    glfwGetFramebufferSize(_window->GetWindow(), &fbw, &fbh);
    uiProj = glm::ortho(0.0f, (float)fbw, 0.0f, (float)fbh);

    auto *water_effect = m_waterPs->get_effect(m_weId);
    auto *rain_effect = m_rainPs->get_effect(m_rnId);
    rain_effect->set_position(fbw, fbh);
    rain_effect->set_vel_y_factor(-fbh / 2.0);
    rain_effect->fraction = (static_cast<float>(fbw) / 3840.0f);

    auto *snow_effect = m_snowPs->get_effect(m_snId);
    snow_effect->set_position(fbw, fbh);
    snow_effect->set_vel_y_factor(-fbh / 2.0);
    snow_effect->set_vel_x_factor(-fbw / 2.0);
    snow_effect->fraction = (static_cast<float>(fbw) / 3840.0f);

    if (player->InsideBlock() == BLOCK_TYPE::WATER_BLOCK) {
      water_effect->set_initial_size(std::max(fbw, fbh));
      water_effect->effect_visible = true;
      rain_effect->effect_visible = false;
      snow_effect->effect_visible = false;
    } else {
      water_effect->effect_visible = false;
      rain_effect->effect_visible = true;
      snow_effect->effect_visible = true;
    }

    glUseProgram(m_shaderProgram);
    m_atlas->Bind();
    // bind sampler to texture unit 0
    glUniform1i(atlas_uniform, 0);
    // skyColor.Bind();
    // bind sampler to texture unit 0
    glUniform3f(skyColor_uniform, clearColor.x, clearColor.y, clearColor.z);

    // OPAQUE PASS
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    world->Draw(OBJ_TYPE::OPAQUE_, playerpos);

    // TRANSPARENT PASS
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    world->Draw(OBJ_TYPE::TRANSPARENT_, playerpos);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    // PARTICLE SYSTEM
    glUseProgram(m_shaderProgramPS);

    glUniformMatrix4fv(uProjLoc_uniformPS, 1, GL_FALSE, glm::value_ptr(uiProj));
    m_waterPs->Draw(dt);

    if (world->getWeather() == WEATHER::HAILSTORM) {
      m_rainPs->Draw(dt);
      bool isPausedorStopped =
          m_thundersound.getSoundStatus() == SOUNDSTATUS::PAUSED;
      isPausedorStopped |=
          m_thundersound.getSoundStatus() == SOUNDSTATUS::STOPPED;
      if (isPausedorStopped)
        m_thundersound.get_sound().play();
      if (m_blizzardsound.getSoundStatus() == SOUNDSTATUS::PLAYING)
        m_blizzardsound.get_sound().pause();
    } else if (world->getWeather() == WEATHER::SNOWSTORM) {
      m_snowPs->Draw(dt);
      bool isPausedorStopped =
          m_blizzardsound.getSoundStatus() == SOUNDSTATUS::PAUSED;
      isPausedorStopped |=
          m_blizzardsound.getSoundStatus() == SOUNDSTATUS::STOPPED;
      if (isPausedorStopped)
        m_blizzardsound.get_sound().play();
      if (m_thundersound.getSoundStatus() == SOUNDSTATUS::PLAYING)
        m_thundersound.get_sound().pause();
    } else {
      m_blizzardsound.get_sound().pause();
      m_thundersound.get_sound().pause();
    }

    // UI PASS (Keep it at last)
    glUseProgram(m_shaderProgramUI);

    glUniformMatrix4fv(uProjLoc_uniformUI, 1, GL_FALSE, glm::value_ptr(uiProj));
    glUniform1i(ui_uniform, 0);
    // m_ui->Draw();

    // Restore
    glDepthMask(GL_TRUE);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
    // draw_axis(axis_VAO, m_shaderProgram);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(_window->GetWindow());
#endif
  }
}

void Game::shutdown() {
#if defined(BUILD_CLIENT) || defined(BUILD_SERVER)
  m_ioc.stop();
  if (m_networkingThread.joinable()) {
    m_networkingThread.join();
  }
#endif
  // Cleanup
#ifndef BUILD_SERVER
  cleanup(_window->GetWindow());
#endif
}
