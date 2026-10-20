#include "Player.h"

// Constructor with networking client
Player::Player(const uint64_t id, const uint64_t shaderProgram,
               const uint64_t shaderProgram2, const std::string &host,
               const std::string &port, boost::asio::io_context &io_context,
               std::function<void(const std::string &)> func)
    : m_id(id), m_shaderProgram(shaderProgram),
      m_shaderProgram2(shaderProgram2) {
  m_client = std::make_unique<Client>(io_context, host, port, func);

  m_client->run();
  m_cameracontroller =
      std::make_unique<CameraController>(SCREEN_HEIGHT / SCREEN_WIDTH);
  m_cameracontroller->UpdateCamera(m_position, m_forward);
  m_window = _window->GetWindow();
  m_meshhandle = asset_manager->loadMeshObject(
      "assets/sphere.obj", shaderProgram2, 0.125, 0.0, m_position, m_forward);

  if (auto l_mesh = asset_manager->get_mesh(m_meshhandle).lock()) {
    l_mesh->setup();
    meshes.push_back(l_mesh);
  }
}

Player::Player(const uint64_t id, const glm::vec3 &pos, const glm::vec3 &dir,
               const uint64_t shaderProgram, const uint64_t shaderProgram2)
    : m_position(pos), m_forward(dir), m_id(id), m_shaderProgram(shaderProgram),
      m_shaderProgram2(shaderProgram2) {
  m_cameracontroller =
      std::make_unique<CameraController>(SCREEN_HEIGHT / SCREEN_WIDTH);
  m_cameracontroller->UpdateCamera(m_position, m_forward);
  m_window = _window->GetWindow();
  m_meshhandle = asset_manager->loadMeshObject(
      "assets/sphere.obj", shaderProgram2, 0.125, 0.0, m_position, m_forward);

  if (auto l_mesh = asset_manager->get_mesh(m_meshhandle).lock()) {
    l_mesh->setup();
    meshes.push_back(l_mesh);
  }
}

Player::Player(const uint64_t id, const uint64_t shaderProgram,
               const uint64_t shaderProgram2)
    : m_id(id), m_shaderProgram(shaderProgram),
      m_shaderProgram2(shaderProgram2) {
  m_cameracontroller =
      std::make_unique<CameraController>(SCREEN_HEIGHT / SCREEN_WIDTH);
  m_cameracontroller->UpdateCamera(m_position, m_forward);
  m_window = _window->GetWindow();

  m_meshhandle = asset_manager->loadMeshObject(
      "assets/sphere.obj", shaderProgram2, 0.5, 0.0, m_position, m_forward);

  if (auto l_mesh = asset_manager->get_mesh(m_meshhandle).lock()) {
    l_mesh->setup();
    meshes.push_back(l_mesh);
  }
}

Player::Player(const uint64_t id, const uint64_t shaderProgram2)
    : m_id(id), m_shaderProgram2(shaderProgram2) {
  m_cameracontroller =
      std::make_unique<CameraController>(SCREEN_HEIGHT / SCREEN_WIDTH);
  m_cameracontroller->UpdateCamera(m_position, m_forward);

  m_meshhandle = asset_manager->loadMeshObject(
      "assets/sphere.obj", shaderProgram2, 0.5, 0.0, m_position, m_forward);

  if (auto l_mesh = asset_manager->get_mesh(m_meshhandle).lock()) {
    meshes.push_back(l_mesh);
  }
}

void Player::handleNetworkRequest(PlayerState &pt) {
  // Update from client
  m_position = pt.pos;
  m_forward = pt.fwd;
  m_velocity = pt.vel;
  m_up = pt.up;

  // Update the camera as well
  m_cameracontroller->UpdateCamera(m_position, m_forward);
  if (auto l_mesh = asset_manager->get_mesh(m_meshhandle).lock()) {
    l_mesh->pos = m_position;
  } else {
    std::cerr << "Mesh not found\n";
  }
}

bool Player::collides(const glm::dvec3 &p) const {
  if (!world)
    return false;

  // Shrink the box slightly so resting exactly on a surface or against a wall
  // doesn't count as overlapping it.
  constexpr double EPS = 1e-4;

  // m_position is the eye/top of the body; feet are PLAYER_HEIGHT below.
  const double minX = p.x - PLAYER_RADIUS + EPS;
  const double maxX = p.x + PLAYER_RADIUS - EPS;
  const double minY = p.y - PLAYER_HEIGHT + EPS;
  const double maxY = p.y - EPS;
  const double minZ = p.z - PLAYER_RADIUS + EPS;
  const double maxZ = p.z + PLAYER_RADIUS - EPS;

  // Range of block cells the box overlaps.
  const int x0 = (int)std::floor(minX / BLOCK_SIZE);
  const int x1 = (int)std::floor(maxX / BLOCK_SIZE);
  const int y0 = (int)std::floor(minY / BLOCK_SIZE);
  const int y1 = (int)std::floor(maxY / BLOCK_SIZE);
  const int z0 = (int)std::floor(minZ / BLOCK_SIZE);
  const int z1 = (int)std::floor(maxZ / BLOCK_SIZE);

  for (int ix = x0; ix <= x1; ++ix)
    for (int iy = y0; iy <= y1; ++iy)
      for (int iz = z0; iz <= z1; ++iz) {
        // isStandable takes block centers, as in your original code
        const glm::dvec3 center((ix + 0.5) * BLOCK_SIZE,
                                (iy + 0.5) * BLOCK_SIZE,
                                (iz + 0.5) * BLOCK_SIZE);
        if (world->isStandable(center))
          return true;
      }

  return false;
}

glm::dvec3 Player::snapToCenter(const glm::dvec3 &p) {
  return glm::floor(p / BLOCK_SIZE) * BLOCK_SIZE + glm::dvec3(HALF_BLOCK_SIZE);
}

void Player::handle_input(float dtRaw) {
  // avoid tunnelling on lag spikes
  const double dt = std::min<double>(dtRaw, 0.05);
  bool position_updated = false;

  // Block at the feet decides speed, block at the head feeds InsideBlock()
  if (world) {
    auto feetBlk = world->get_block_by_center(
        snapToCenter(m_position - glm::dvec3(0, PLAYER_HEIGHT - 0.01, 0)));
    auto headBlk = world->get_block_by_center(snapToCenter(m_position));
    if (headBlk)
      m_insideBlock = headBlk->get_type();
    bool inWater = feetBlk && feetBlk->get_type() == BLOCK_TYPE::WATER_BLOCK;
    m_speed = inWater ? SMUGED_SPEED : WALKING_SPEED;
  }

  // Horizontal wish direction, normalized so diagonals aren't faster
  glm::dvec3 fwd = glm::normalize(glm::dvec3(m_forward.x, 0.0, m_forward.z));
  glm::dvec3 right = glm::normalize(glm::cross(m_forward, m_up));
  glm::dvec3 wish(0.0);
  if (Input::IsKeyPressed(GLFW_KEY_W))
    wish += fwd;
  if (Input::IsKeyPressed(GLFW_KEY_S))
    wish -= fwd;
  if (Input::IsKeyPressed(GLFW_KEY_D))
    wish += right;
  if (Input::IsKeyPressed(GLFW_KEY_A))
    wish -= right;
  if (glm::length(wish) > 0.0)
    wish = glm::normalize(wish);
  const glm::dvec3 delta = wish * m_speed * dt;

  if (!m_enableGravity) {
    m_position += delta;
    if (Input::IsKeyPressed(GLFW_KEY_SPACE))
      m_position += m_up * m_speed * dt;
    if (Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
      m_position -= m_up * m_speed * dt;
    position_updated = true;
  } else {
    // Horizontal, one axis at a time => wall sliding
    for (int axis : {0, 2}) {
      glm::dvec3 next = m_position;
      next[axis] += delta[axis];
      if (!collides(next)) {
        m_position = next;
        position_updated = true;
      }
    }

    // Jump
    if (m_onGround && Input::IsKeyPressed(GLFW_KEY_SPACE)) {
      m_velocity.y = JUMP_SPEED;
      m_onGround = false;
    }

    // Gravity, substepped (your existing loop is fine)
    m_velocity.y = std::max(m_velocity.y + GRAVITY_ACCEL * dt, TERMINAL_VEL);
    const double totalDy = m_velocity.y * dt;
    const int steps =
        std::max(1, (int)std::ceil(std::abs(totalDy) / (BLOCK_SIZE * 0.5)));
    const double stepDy = totalDy / steps;

    m_onGround = false;
    for (int i = 0; i < steps; ++i) {
      glm::dvec3 next = m_position;
      next.y += stepDy;
      if (!collides(next)) {
        m_position = next;
      } else {
        if (m_velocity.y < 0.0) {
          double feetY = next.y - PLAYER_HEIGHT;
          m_position.y = (std::floor(feetY / BLOCK_SIZE) + 1.0) * BLOCK_SIZE +
                         PLAYER_HEIGHT;
          m_onGround = true;
        }
        m_velocity.y = 0.0; // also handles head bumps
        break;
      }
    }
    position_updated = true;
  }

  auto [x, y] = Input::GetMousePosition();
  double rotx = m_sensitivity * (double)(y - m_mousePos.y);
  double roty = m_sensitivity * (double)(x - m_mousePos.x);
  m_mousePos = glm::vec2(x, y);

  ImGuiIO &io = ImGui::GetIO();
  if (!io.WantCaptureMouse) {
    if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
      glm::dvec3 right = glm::normalize(glm::cross(m_forward, m_up));

      // glm::quat qx = glm::normalize(
      //    glm::cross(glm::angleAxis(-rotx, right), glm::angleAxis(roty,
      //    m_Camera->GetUp())));
      // m_Camera->SetOrientation(glm::normalize(qx *
      // m_Camera->GetOrientation()));

      glm::dquat qx = glm::angleAxis(-rotx, right);
      glm::dquat qy = glm::angleAxis(-roty, m_up);
      glm::dquat rotation = glm::normalize(qy * qx);
      m_forward = glm::normalize(rotation * m_forward);
      position_updated = true;
    }
  }

  // Update the camera as well
  m_cameracontroller->UpdateCamera(m_position, m_forward);
  if (auto l_mesh = asset_manager->get_mesh(m_meshhandle).lock()) {
    l_mesh->pos = m_position;
  } else {
    std::cerr << "Mesh not found\n";
  }

  // default initialize to indicate no updates
  WorldState ws{};
  bool world_updated = false;
  if (!Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL) &&
      Input::WasMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
    strcpy(textKeyStatus, "Right click");
    strcpy(textKeyDescription, "Casting ray");

    double mouseX, mouseY;
    glfwGetCursorPos(m_window, &mouseX, &mouseY);

    Ray ray =
        screenPosToWorldRay(m_window, mouseX, mouseY, viewRotateT, projectionT);

    if (ray.did_hit(world)) { // Remove a block
      std::cout << "Ray hit a block with center: " << ray.m_hitcords.x << " "
                << ray.m_hitcords.y << " " << ray.m_hitcords.z << '\n';
      auto block = world->get_block_by_center(ray.m_hitcords);
      if (block)
        block->remove();

      if (auto _chunk = world->get_chunk_by_center(ray.m_hitcords).lock()) {
        // Set the dirty bit
        _chunk->m_dirtyBit = true;
      }
      if (auto _biome = world->get_biome_by_center(ray.m_hitcords).lock()) {
        // Set the dirty bit
        _biome->m_dirtyBit = true;
      }

      // Update ws to send to server
      ws.blockpos = ray.m_hitcords;
      ws.blk = block->blmask;

      world_updated = true;
      world->RefreshChunks(ray.m_hitcords);
      // world->RenderWorld();
    } else {
      std::cout << "Ray didn't hit any block\n";
    }
  } else if (!Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL) &&
             Input::WasMouseButtonPressed(
                 GLFW_MOUSE_BUTTON_MIDDLE)) { // Add a block
    strcpy(textKeyStatus, "Middle click");
    strcpy(textKeyDescription, "Casting ray");

    double mouseX, mouseY;
    glfwGetCursorPos(m_window, &mouseX, &mouseY);

    Ray ray =
        screenPosToWorldRay(m_window, mouseX, mouseY, viewRotateT, projectionT);

    if (ray.did_hit(world)) {
      std::cout << "Ray hit a block with center: " << ray.m_hitcords.x << " "
                << ray.m_hitcords.y << " " << ray.m_hitcords.z << '\n';
      glm::ivec3 prev_blk = ray.m_hitcordsprev;
      glm::ivec3 hit_blk = ray.m_hitcords;
      auto hitblk = world->get_block_by_center(hit_blk);
      auto block = world->get_block_by_center(prev_blk);
      if (hitblk && hitblk->get_type() == BLOCK_TYPE::WATER_BLOCK) {
        hitblk->add(static_cast<BLOCK_TYPE>(m_blType));
      } else {
        if (block) {
          block->add(static_cast<BLOCK_TYPE>(m_blType));
        } else {
          return;
        }
      }

      if (auto _chunk = world->get_chunk_by_center(ray.m_hitcords).lock()) {
        // Update dirty bit
        _chunk->m_dirtyBit = true;
      }
      if (auto _biome = world->get_biome_by_center(ray.m_hitcords).lock()) {
        // Update dirty bit
        _biome->m_dirtyBit = true;
      }

      // Update ws to send to server
      ws.blockpos = prev_blk;
      ws.blk = block->blmask;
      world_updated = true;
      world->RefreshChunks(ray.m_hitcords);
    } else {
      std::cout << "Ray didn't hit any block\n";
    }
  }

  if (Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL)) {
    if (Input::WasMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
      double mouseX, mouseY;
      glfwGetCursorPos(m_window, &mouseX, &mouseY);

      Ray ray = screenPosToWorldRay(m_window, mouseX, mouseY, viewRotateT,
                                    projectionT);

      if (ray.did_hit(world)) {
        std::cout << "[CTRL] Ray hit a block with center: " << ray.m_hitcords.x
                  << " " << ray.m_hitcords.y << " " << ray.m_hitcords.z << '\n';
        auto chunk = world->get_chunk_by_center(ray.m_hitcords);

        if (m_mdType != 0) {
          auto vec = ray.m_hitcords + glm::ivec3(0, BLOCK_SIZE, 0);
          world->load_model(vec, "models/" + MODEL_ARRAY[m_mdType] + ".bin");
          // instead of sending all of the blocks send block to render model
          // there damn i'm so smart!!!
          ws.blockpos = vec;
          ws.model_idx = m_mdType;
          world_updated = true;
          world->RefreshChunks(ray.m_hitcords + glm::ivec3(0, BLOCK_SIZE, 0));
        }
      }
    }
  }

  if (m_noKeyPressed) {
    strcpy(textKeyStatus, "Listening for key events...");
    strcpy(textKeyDescription, "Listening for key events...");
  }

  // Send player update
  if (position_updated && m_client)
    m_client->send(std::make_shared<std::string>(get_state()));
  // Send world update
  if (world_updated && m_client)
    m_client->send(std::make_shared<std::string>(world->get_state(ws)));
}

// returns true if a valid move
bool Player::Valid(PlayerState &ps) {
  // update server state
  m_position = ps.pos;
  m_forward = ps.fwd;
  m_velocity = ps.vel;
  m_up = ps.up;
  return true;
}

BLOCK_TYPE Player::InsideBlock() { return m_insideBlock; }

// handle input [for server]
std::shared_ptr<std::string>
Player::handle_client_input(const std::string &msg) {
  State st;
  std::memcpy(&st, msg.data(), sizeof(State));

  PlayerState pst = std::get<PlayerState>(st._data);

  if (!Valid(pst)) { // if not valid return last state
    return std::make_shared<std::string>(get_state());
  }
  // if verified return new state
  return std::make_shared<std::string>(msg);
}

// return pointer to underlying client [for client]
Client *const Player::get_client() { return m_client.get(); }

const std::string Player::get_state() {
  PlayerState pst;
  State st;
  st.ts = glfwGetTime();
  st.id = m_id;
  pst.pos = m_position;
  pst.fwd = m_forward;
  pst.vel = m_velocity;
  pst.up = m_up;
  st._data = pst;

  std::string out(sizeof(State), '\0');
  std::memcpy(out.data(), &st, sizeof(State));
  return out;
}

void Player::handle_stats() {
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // ImGui UI menu
  ImGui::Begin("Main", NULL, ImGuiWindowFlags_AlwaysAutoResize);
  if (ImGui::CollapsingHeader("Information", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::Text("Key Status: %s", textKeyStatus);
    ImGui::Text("Key Description: %s", textKeyDescription);
    ImGui::Text("Active Player: %d", activePlayer);
    ImGui::Text("Block Selected: %s", BLOCK_ARRAY[m_blType].c_str());
    glm::vec3 playerPos = m_cameracontroller->GetCamera()->GetPosition();
    glm::vec3 playerOri = m_cameracontroller->GetCamera()->GetOrientation();
    playerPos.x -= ((BIOME_COUNTX)*CHUNK_LENGTH / 2.0);
    playerPos.z -= ((BIOME_COUNTZ)*CHUNK_LENGTH / 2.0);
    playerPos.y -= ((BIOME_COUNTY)*CHUNK_HEIGHT / 2.0) + PLAYER_HEIGHT;
    ImGui::Text("Player %d position: (%.2f, %.2f, %.2f)", activePlayer,
                playerPos.x, playerPos.y, playerPos.z);
    ImGui::Text("Player %d orientation: (%.2f, %.2f, %.2f)", activePlayer,
                playerOri.x, playerOri.y, playerOri.z);
    if (ImGui::InputInt("Render Distance", &NUM_BIOMES_TO_TRAVERSE, 1, 5)) {
      NUM_BIOMES_TO_TRAVERSE = std::clamp(NUM_BIOMES_TO_TRAVERSE, 2, 100);
      FAR_PLANE = 1000.0 * NUM_BIOMES_TO_TRAVERSE;
      RENDER_DISTANCE = BIOME_SIZE * NUM_BIOMES_TO_TRAVERSE;
      m_cameracontroller->GetCamera()->RecalculateProjectionMatrix();
    }
  }

  // Enable Physics
  ImGui::BeginChild("Child##Physics", ImVec2(400, 60), true);
  ImGui::Text("Physics");
  ImGui::Checkbox("Enable physics", &m_enableGravity);
  ImGui::EndChild();

  // Display Mode
  ImGui::BeginChild("Child##Rendering", ImVec2(400, 80), true);

  ImGui::Text("Rendering");
  ImGui::RadioButton("Fill Mode", (int *)&wireframemode, 0);
  ImGui::RadioButton("Wireframe Mode", (int *)&wireframemode, 1);
  ImGui::EndChild();

  // Weather
  ImGui::BeginChild("Child##Weather", ImVec2(400, 80), true);

  int whtr = static_cast<int>(world->getWeather());
  ImGui::Text("Weather");
  ImGui::RadioButton("CLOUDY", (int *)&whtr, 0);
  ImGui::RadioButton("HAILSTORM", (int *)&whtr, 1);
  ImGui::RadioButton("SNOWSTORM", (int *)&whtr, 2);
  ImGui::EndChild();

  world->setWeather(static_cast<WEATHER>(whtr));

  // Selected Block
  // Begin a child region with fixed height and automatic scrollbar
  ImGui::BeginChild("Selected Block", ImVec2(400, 75), true,
                    ImGuiWindowFlags_HorizontalScrollbar);

  ImGui::Text("Selected Block");
  for (int i = 0; i < static_cast<int>(BLOCK_TYPE::NUM_BLOCK); i++) {
    if (ImGui::RadioButton(BLOCK_ARRAY[i].c_str(), (int *)&m_blType, i)) {
    }
  }

  ImGui::EndChild();

  // Selected Model
  // Begin a child region with fixed height and automatic scrollbar
  ImGui::BeginChild("Selected Model", ImVec2(400, 75), true,
                    ImGuiWindowFlags_HorizontalScrollbar);

  ImGui::Text("Selected Model");
  for (int i = 0; i < MODEL_TYPES; i++) {
    if (ImGui::RadioButton(MODEL_ARRAY[i].c_str(), (int *)&m_mdType, i)) {
    }
  }

  ImGui::EndChild();

  // Save Model
  ImGui::BeginChild("Save", ImVec2(400, 150), true,
                    ImGuiWindowFlags_HorizontalScrollbar);
  static int X = 0;
  static int Y = 0;
  static char model_name[32];

  ImGui::Text("Save Model");
  // Input fields
  ImGui::InputInt("Chunk X", &X);
  ImGui::InputInt("Chunk Y", &Y);

  // Clamp negative values
  if (X < 0)
    X = 0;
  if (Y < 0)
    Y = 0;
  ImGui::InputText("Model Name", model_name, IM_ARRAYSIZE(model_name));

  if (ImGui::Button("Save")) {
    // Save the model in chunk 0 included between ref
    glm::ivec3 tmp =
        glm::ivec3(X * CHUNK_LENGTH + HALF_BLOCK_SIZE, HALF_BLOCK_SIZE,
                   Y * CHUNK_LENGTH + HALF_BLOCK_SIZE);
    if (auto chunk = world->get_chunk_by_center(tmp).lock()) {
      world->save_model(chunk, std::string(model_name));
    }
  };

  ImGui::EndChild();

  // Save World
  ImGui::BeginChild("Save World", ImVec2(400, 150), true,
                    ImGuiWindowFlags_HorizontalScrollbar);
  static char save_file_name[32];

  ImGui::Text("Save");

  ImGui::InputText("World Name", save_file_name, IM_ARRAYSIZE(save_file_name));

  if (ImGui::Button("Save")) {
    // Save the world
    world->save(std::string(save_file_name));
  };

  ImGui::EndChild();
  ImGui::End();

  // Rendering
  ImGui::Render();
  glfwGetFramebufferSize(m_window, &m_displayW, &m_displayH);
  _window->SetHeight(m_displayH);
  _window->SetWidth(m_displayW);
}

void Player::setupModelTransformationCube(unsigned int &program) {
  // Modelling transfordmations (Model -> World coordinates)
  modelT = glm::scale(glm::dmat4(1.0f), glm::dvec3(1.0, 1.0, 1.0));
  // modelT = glm::translate(modelT, glm::dvec3(0.0, 0.0, 0.0));

  // Pass on the modelling matrix to the vertex shader
  glUseProgram(program);
  vModel_uniform = glGetUniformLocation(program, "vModel");
  if (vModel_uniform == -1) {
    fprintf(stderr, "Could not bind location: vModel\n");
    exit(0);
  }

  glm::mat4 modelTf = glm::mat4(modelT);
  glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(modelTf));
}

void Player::setupModelTransformationAxis(unsigned int &program,
                                          double rot_angle,
                                          glm::dvec3 rot_axis) {
  // Modelling transformations (Model -> World coordinates)
  modelT = glm::rotate(glm::dmat4(1.0f), rot_angle, rot_axis);

  // Pass on the modelling matrix to the vertex shader
  glUseProgram(program);
  vModel_uniform = glGetUniformLocation(program, "vModel");
  if (vModel_uniform == -1) {
    fprintf(stderr, "Could not bind location: vModel\n");
    exit(0);
  }

  glm::mat4 modelTf = glm::mat4(modelT);
  glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(modelTf));
}

void Player::setupViewTransformation(unsigned int &program,
                                     std::unique_ptr<CameraController> &occ) {
  // Viewing transformations (World -> Camera coordinates
  //  viewT = glm::lookAt(glm::vec3(camPosition), glm::vec3(0.0, 0.0, 0.0),
  //  glm::vec3(0.0, 1.0, 0.0));
  viewT = occ->GetCamera()->GetViewRenderMatrix();
  viewRotateT = occ->GetCamera()->GetViewMatrix();

  // Pass-on the viewing matrix to the vertex shader
  glUseProgram(program);
  vView_uniform = glGetUniformLocation(program, "vView");
  if (vView_uniform == -1) {
    fprintf(stderr, "Could not bind location: vView\n");
    exit(0);
  }

  glm::mat4 viewTf = glm::mat4(viewT);
  glUniformMatrix4fv(vView_uniform, 1, GL_FALSE, glm::value_ptr(viewTf));
}

void Player::setupProjectionTransformation(
    unsigned int &program, std::unique_ptr<CameraController> &occ) {
  // Projection transformation
  projectionT = occ->GetCamera()->GetProjectionMatrix();

  // Pass on the projection matrix to the vertex shader
  glUseProgram(program);
  vProjection_uniform = glGetUniformLocation(program, "vProjection");
  if (vProjection_uniform == -1) {
    fprintf(stderr, "Could not bind location: vProjection\n");
    exit(0);
  }

  glm::mat4 projectionTf = glm::mat4(projectionT);
  glUniformMatrix4fv(vProjection_uniform, 1, GL_FALSE,
                     glm::value_ptr(projectionTf));
}

void Player::handle_transformations() {
  m_cameracontroller->SetAspectRatio((float)m_displayW / (float)m_displayH);
  glViewport(0, 0, m_displayW, m_displayH);

  // Setup MVP matrix
  glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
  glEnable(GL_DEPTH_TEST); // restore
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_CULL_FACE); // Enable OC
  glEnable(GL_BLEND);     // Enable BLENDING
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  // Render the mesh
  for (auto mesh : meshes) {
    mesh->render(m_cameracontroller, m_position);
  }

  setupModelTransformationCube(m_shaderProgram);
  setupViewTransformation(m_shaderProgram, m_cameracontroller);
  setupProjectionTransformation(m_shaderProgram, m_cameracontroller);
}

void Player::update(float dt) {
  // handle player Input
  handle_input(dt);
  // Display stats
  handle_stats();

  handle_transformations();
}
