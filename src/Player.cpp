#include "Player.h"

// Constructor with networking client
Player::Player(const uint64_t id, const uint64_t shaderProgram,
               const std::string &host, const std::string &port,
               boost::asio::io_context &io_context,
               std::function<void(const std::string &)> func)
    : m_id(id) {
  m_client = std::make_unique<Client>(io_context, host, port, func);

  m_client->run();
  m_cameracontroller =
      std::make_unique<CameraController>(SCREEN_HEIGHT / SCREEN_WIDTH);
  m_cameracontroller->UpdateCamera(m_position, m_forward);
  window = _window->GetWindow();
  m_meshhandle = asset_manager->loadMeshObject(
      "assets/sphere.obj", shaderProgram, 0.125, 0.0, m_position, m_forward);

  if (auto l_mesh = asset_manager->get_mesh(m_meshhandle).lock()) {
    l_mesh->setup();
    meshes.push_back(l_mesh);
  }
}

Player::Player(const uint64_t id, const glm::vec3 &pos, const glm::vec3 &dir,
               const uint64_t shaderProgram)
    : m_position(pos), m_forward(dir), m_id(id) {
  m_cameracontroller =
      std::make_unique<CameraController>(SCREEN_HEIGHT / SCREEN_WIDTH);
  m_cameracontroller->UpdateCamera(m_position, m_forward);
  window = _window->GetWindow();
  m_meshhandle = asset_manager->loadMeshObject(
      "assets/sphere.obj", shaderProgram, 0.125, 0.0, m_position, m_forward);

  if (auto l_mesh = asset_manager->get_mesh(m_meshhandle).lock()) {
    l_mesh->setup();
    meshes.push_back(l_mesh);
  }
}

Player::Player(const uint64_t id, const uint64_t shaderProgram) : m_id(id) {
  m_cameracontroller =
      std::make_unique<CameraController>(SCREEN_HEIGHT / SCREEN_WIDTH);
  m_cameracontroller->UpdateCamera(m_position, m_forward);
  window = _window->GetWindow();

  m_meshhandle = asset_manager->loadMeshObject(
      "assets/sphere.obj", shaderProgram, 0.5, 0.0, m_position, m_forward);

  if (auto l_mesh = asset_manager->get_mesh(m_meshhandle).lock()) {
    l_mesh->setup();
    meshes.push_back(l_mesh);
  }
}

Player::Player(const uint64_t id) : m_id(id) {
  m_cameracontroller =
      std::make_unique<CameraController>(SCREEN_HEIGHT / SCREEN_WIDTH);
  m_cameracontroller->UpdateCamera(m_position, m_forward);

  m_meshhandle = asset_manager->loadMeshObject(
      "assets/sphere.obj", shaderProgram, 0.5, 0.0, m_position, m_forward);

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

void Player::handle_input(float dt) {

  bool position_updated = false;
  BLOCK_TYPE legblock = BLOCK_TYPE::GRASS_BLOCK;
  // Gravity
  glm::dvec3 v = glm::floor(m_position / BLOCK_SIZE) * BLOCK_SIZE +
                 glm::dvec3(HALF_BLOCK_SIZE);
  if (enable_gravity) {
    bool adjusted = false;
    if (world && (!(world->isStandable(v - glm::dvec3(0, PLAYER_HEIGHT, 0)))) &&
        (v.y > 1.0f)) {
      v.y -= BLOCK_SIZE;
      adjusted = true;
    }

    float height = v.y + BLOCK_SIZE - OFFSET;
    // Case 2: If we're inside a block -> snap up
    if (world && world->isStandable(v - glm::dvec3(0, PLAYER_HEIGHT, 0)) &&
        (v.y < height)) {
      v.y += BLOCK_SIZE; // step up until clear
      adjusted = true;
      auto blk =
          world->get_block_by_center(v - glm::dvec3(0, PLAYER_HEIGHT, 0) +
                                     glm::dvec3(0, 2 * BLOCK_SIZE, 0));
      if (blk)
        inside_block = blk->get_type();

      auto legblk =
          world->get_block_by_center(v - glm::dvec3(0, PLAYER_HEIGHT, 0));
      if (legblk)
        legblock = legblk->get_type();
    }

    if (adjusted) {
      m_position.y = v.y;
      position_updated = true;
    }
  }

  auto toBlockCenter = [](glm::dvec3 pos) {
    glm::ivec3 block = glm::floor(pos / BLOCK_SIZE); // which block
    return (glm::dvec3(block) + 0.5) * BLOCK_SIZE -
           glm::dvec3(0, BLOCK_SIZE, 0); // center of that block
  };

  glm::dvec3 planarvec =
      glm::normalize(glm::dvec3(m_forward.x, 0.0, m_forward.z));

  if (legblock == BLOCK_TYPE::WATER_BLOCK)
    m_speed = SMUGED_SPEED;
  else
    m_speed = WALKING_SPEED;

  if (Input::IsKeyPressed(GLFW_KEY_W)) {
    glm::dvec3 nextPos = m_position + planarvec * m_speed * (double)(dt);
    glm::dvec3 blockCenter_h2 = toBlockCenter(nextPos);
    glm::dvec3 blockCenter_h3 = blockCenter_h2 + glm::dvec3(0, BLOCK_SIZE, 0);
    if (!world->isStandable(blockCenter_h2) &&
            !world->isStandable(blockCenter_h3) ||
        !enable_gravity) {
      m_position = nextPos;
      position_updated = true;
    }
  } else if (Input::IsKeyPressed(GLFW_KEY_S)) {
    glm::dvec3 nextPos = m_position - planarvec * m_speed * (double)(dt);
    glm::dvec3 blockCenter_h2 = toBlockCenter(nextPos);
    glm::dvec3 blockCenter_h3 = blockCenter_h2 + glm::dvec3(0, BLOCK_SIZE, 0);
    if (!world->isStandable(blockCenter_h2) &&
            !world->isStandable(blockCenter_h3) ||
        !enable_gravity) {
      m_position = nextPos;
      position_updated = true;
    }
  }

  if (Input::IsKeyPressed(GLFW_KEY_A)) {
    // Check if obstructed by block
    glm::dvec3 nextPos =
        m_position - m_speed * dt * glm::normalize(glm::cross(m_forward, m_up));
    glm::ivec3 blockCenter_h2 = toBlockCenter(nextPos);
    glm::ivec3 blockCenter_h3 = blockCenter_h2 + glm::ivec3(0, BLOCK_SIZE, 0);
    if (!world->isStandable(blockCenter_h2) &&
            !world->isStandable(blockCenter_h3) ||
        !enable_gravity) {
      m_position = nextPos;
      position_updated = true;
    }
  } else if (Input::IsKeyPressed(GLFW_KEY_D)) {
    // Check if obstructed by block
    glm::dvec3 nextPos =
        m_position + m_speed * dt * glm::normalize(glm::cross(m_forward, m_up));
    glm::ivec3 blockCenter_h2 = toBlockCenter(nextPos);
    glm::ivec3 blockCenter_h3 = blockCenter_h2 + glm::ivec3(0, BLOCK_SIZE, 0);
    if (!world->isStandable(blockCenter_h2) &&
            !world->isStandable(blockCenter_h3) ||
        !enable_gravity) {
      m_position = nextPos;
      position_updated = true;
    }
  }

  // Floating only possible in case gravity is not available
  if (!enable_gravity) {
    if (Input::IsKeyPressed(GLFW_KEY_SPACE)) {
      m_position = m_position + m_up * m_speed * double(dt);
      position_updated = true;
    } else if (Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
      m_position = m_position - m_up * m_speed * double(dt);
      position_updated = true;
    }
  } else {
    // Jump if gravity enabled
    if (Input::WasKeyPressed(GLFW_KEY_SPACE)) {
      m_position = m_position + m_up * m_speed * double(dt) * 100.0;
      position_updated = true;
    }
  }

  auto [x, y] = Input::GetMousePosition();
  double rotx = m_sensitivity * (double)(y - MousePos.y);
  double roty = m_sensitivity * (double)(x - MousePos.x);
  MousePos = glm::vec2(x, y);

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
    glfwGetCursorPos(window, &mouseX, &mouseY);

    Ray ray =
        screenPosToWorldRay(window, mouseX, mouseY, viewRotateT, projectionT);

    if (ray.did_hit(world)) { // Remove a block
      std::cout << "Ray hit a block with center: " << ray.m_hitcords.x << " "
                << ray.m_hitcords.y << " " << ray.m_hitcords.z << '\n';
      auto block = world->get_block_by_center(ray.m_hitcords);
      if (block)
        block->remove();

      if (auto _chunk = world->get_chunk_by_center(ray.m_hitcords).lock()) {
        // Set the dirty bit
        _chunk->dirtybit = true;
      }
      if (auto _biome = world->get_biome_by_center(ray.m_hitcords).lock()) {
        // Set the dirty bit
        _biome->dirtybit = true;
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
    glfwGetCursorPos(window, &mouseX, &mouseY);

    Ray ray =
        screenPosToWorldRay(window, mouseX, mouseY, viewRotateT, projectionT);

    if (ray.did_hit(world)) {
      std::cout << "Ray hit a block with center: " << ray.m_hitcords.x << " "
                << ray.m_hitcords.y << " " << ray.m_hitcords.z << '\n';
      glm::ivec3 prev_blk = ray.m_hitcordsprev;
      glm::ivec3 hit_blk = ray.m_hitcords;
      auto hitblk = world->get_block_by_center(hit_blk);
      auto block = world->get_block_by_center(prev_blk);
      if (hitblk && hitblk->get_type() == BLOCK_TYPE::WATER_BLOCK) {
        hitblk->add(static_cast<BLOCK_TYPE>(bltype));
      } else {
        if (block) {
          block->add(static_cast<BLOCK_TYPE>(bltype));
        } else {
          return;
        }
      }

      if (auto _chunk = world->get_chunk_by_center(ray.m_hitcords).lock()) {
        // Update dirty bit
        _chunk->dirtybit = true;
      }
      if (auto _biome = world->get_biome_by_center(ray.m_hitcords).lock()) {
        // Update dirty bit
        _biome->dirtybit = true;
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
      glfwGetCursorPos(window, &mouseX, &mouseY);

      Ray ray =
          screenPosToWorldRay(window, mouseX, mouseY, viewRotateT, projectionT);

      if (ray.did_hit(world)) {
        std::cout << "[SHIFT] Ray hit a block with center: " << ray.m_hitcords.x
                  << " " << ray.m_hitcords.y << " " << ray.m_hitcords.z << '\n';
        auto chunk = world->get_chunk_by_center(ray.m_hitcords);

        if (mdtype != 0) {
          auto vec = ray.m_hitcords + glm::ivec3(0, BLOCK_SIZE, 0);
          world->load_model(vec, "models/" + MODEL_ARRAY[mdtype] + ".bin");
          // instead of sending all of the blocks send block to render model
          // there damn i'm so smart!!!
          ws.blockpos = vec;
          ws.model_idx = mdtype;
          world_updated = true;
          world->RefreshChunks(ray.m_hitcords + glm::ivec3(0, BLOCK_SIZE, 0));
        }
      }
    }
  }

  if (Nokeypressed) {
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

BLOCK_TYPE Player::InsideBlock() { return inside_block; }

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
    ImGui::Text("Block Selected: %s", BLOCK_ARRAY[bltype].c_str());
    glm::vec3 playerPos = m_cameracontroller->GetCamera()->GetPosition();
    glm::vec3 playerOri = m_cameracontroller->GetCamera()->GetOrientation();
    playerPos.y -= (BIOME_COUNTY - 1) * BIOME_HEIGHT;
    ImGui::Text("Player %d position: (%.2f, %.2f, %.2f)", activePlayer,
                playerPos.x, playerPos.y, playerPos.z);
    ImGui::Text("Player %d orientation: (%.2f, %.2f, %.2f)", activePlayer,
                playerOri.x, playerOri.y, playerOri.z);
  }

  // Enable Physics
  ImGui::BeginChild("Child##Physics", ImVec2(400, 60), true);
  ImGui::Text("Physics");
  ImGui::Checkbox("Enable physics", &enable_gravity);
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
    if (ImGui::RadioButton(BLOCK_ARRAY[i].c_str(), (int *)&bltype, i)) {
    }
  }

  ImGui::EndChild();

  // Selected Model
  // Begin a child region with fixed height and automatic scrollbar
  ImGui::BeginChild("Selected Model", ImVec2(400, 75), true,
                    ImGuiWindowFlags_HorizontalScrollbar);

  ImGui::Text("Selected Model");
  for (int i = 0; i < MODEL_TYPES; i++) {
    if (ImGui::RadioButton(MODEL_ARRAY[i].c_str(), (int *)&mdtype, i)) {
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
  glfwGetFramebufferSize(window, &display_w, &display_h);
  _window->SetHeight(display_h);
  _window->SetWidth(display_w);
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
  m_cameracontroller->SetAspectRatio((float)display_w / (float)display_h);
  glViewport(0, 0, display_w, display_h);

  glUseProgram(shaderProgram);
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

  setupModelTransformationCube(shaderProgram);
  setupViewTransformation(shaderProgram, m_cameracontroller);
  setupProjectionTransformation(shaderProgram, m_cameracontroller);
}

void Player::update(float dt) {
  // handle player Input
  handle_input(dt);
  // Display stats
  handle_stats();

  handle_transformations();
}
