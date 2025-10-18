#include "Player.h"

Player::Player(const glm::vec3 &pos, const glm::vec3 &dir) : m_position(pos), m_forward(dir) {
  m_cameracontroller = std::make_unique<CameraController>(SCREEN_HEIGHT / SCREEN_WIDTH);
  m_cameracontroller->UpdateCamera(m_position, m_forward);
  window = _window->GetWindow();
}

Player::Player() {
  m_cameracontroller = std::make_unique<CameraController>(SCREEN_HEIGHT / SCREEN_WIDTH);
  m_cameracontroller->UpdateCamera(m_position, m_forward);
  window = _window->GetWindow();
}

void Player::handle_input() {

  // Gravity
  glm::vec3 v = glm::floor(m_position / BLOCK_SIZE) * BLOCK_SIZE + glm::vec3(HALF_BLOCK_SIZE);
  if (enable_gravity) {
    bool adjusted = false;
    if (world && (!(world->isSolid(v - glm::vec3(0, PLAYER_HEIGHT, 0)))) && (v.y > 1.0f)) {
      v.y -= BLOCK_SIZE;
      adjusted = true;
    }

    float height = v.y + BLOCK_SIZE - OFFSET;
    // Case 2: If we're inside a block -> snap up
    if (world && world->isSolid(v - glm::vec3(0, PLAYER_HEIGHT, 0)) && (v.y < height)) {
      v.y += BLOCK_SIZE;  // step up until clear
      adjusted = true;
    }

    if (adjusted) m_position.y = v.y;
  }

  auto toBlockCenter = [](glm::vec3 pos) {
    glm::ivec3 block = glm::floor(pos / BLOCK_SIZE);  // which block
    return (glm::vec3(block) + 0.5f) * BLOCK_SIZE -
           glm::vec3(0, BLOCK_SIZE, 0);  // center of that block
  };

  glm::vec3 planarvec = glm::normalize(glm::vec3(m_forward.x, 0.0f, m_forward.z));

  if (Input::IsKeyPressed(GLFW_KEY_W)) {
    glm::vec3 nextPos = m_position + planarvec * m_speed;
    glm::vec3 blockCenter_h2 = toBlockCenter(nextPos);
    glm::vec3 blockCenter_h3 = blockCenter_h2 + glm::vec3(0, BLOCK_SIZE, 0);
    if (!world->isSolid(blockCenter_h2) && !world->isSolid(blockCenter_h3) || !enable_gravity) {
      m_position = nextPos;
    }
  } else if (Input::IsKeyPressed(GLFW_KEY_S)) {
    glm::vec3 nextPos = m_position - planarvec * m_speed;
    glm::vec3 blockCenter_h2 = toBlockCenter(nextPos);
    glm::vec3 blockCenter_h3 = blockCenter_h2 + glm::vec3(0, BLOCK_SIZE, 0);
    if (!world->isSolid(blockCenter_h2) && !world->isSolid(blockCenter_h3) || !enable_gravity) {
      m_position = nextPos;
    }
  }

  if (Input::IsKeyPressed(GLFW_KEY_A)) {
    // Check if obstructed by block
    glm::vec3 nextPos = m_position - m_speed * glm::normalize(glm::cross(m_forward, m_up));
    glm::vec3 blockCenter_h2 = toBlockCenter(nextPos);
    glm::vec3 blockCenter_h3 = blockCenter_h2 + glm::vec3(0, BLOCK_SIZE, 0);
    if (!world->isSolid(blockCenter_h2) && !world->isSolid(blockCenter_h3) || !enable_gravity) {
      m_position = nextPos;
    }
  } else if (Input::IsKeyPressed(GLFW_KEY_D)) {
    // Check if obstructed by block
    glm::vec3 nextPos = m_position + m_speed * glm::normalize(glm::cross(m_forward, m_up));
    glm::vec3 blockCenter_h2 = toBlockCenter(nextPos);
    glm::vec3 blockCenter_h3 = blockCenter_h2 + glm::vec3(0, BLOCK_SIZE, 0);
    if (!world->isSolid(blockCenter_h2) && !world->isSolid(blockCenter_h3) || !enable_gravity) {
      m_position = nextPos;
    }
  }


  // Floating only possible in case gravity is not available
  if (!enable_gravity) {
    if (Input::IsKeyPressed(GLFW_KEY_SPACE)) {
      m_position = m_position + m_up * m_speed;
    } else if (Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
      m_position = m_position - m_up * m_speed;
    }
  }


  auto [x, y] = Input::GetMousePosition();
  float rotx = m_sensitivity * (float)(y - MousePos.y);
  float roty = m_sensitivity * (float)(x - MousePos.x);
  MousePos = glm::vec2(x, y);

  ImGuiIO &io = ImGui::GetIO();
  if (!io.WantCaptureMouse) {
    if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
      glm::vec3 right = glm::normalize(glm::cross(m_forward, m_up));

      // glm::quat qx = glm::normalize(
      //    glm::cross(glm::angleAxis(-rotx, right), glm::angleAxis(roty, m_Camera->GetUp())));
      // m_Camera->SetOrientation(glm::normalize(qx * m_Camera->GetOrientation()));

      glm::quat qx = glm::angleAxis(-rotx, right);
      glm::quat qy = glm::angleAxis(-roty, m_up);
      glm::quat rotation = glm::normalize(qy * qx);
      m_forward = glm::normalize(rotation * m_forward);
    }
  }

  // Update the camera as well
  m_cameracontroller->UpdateCamera(m_position, m_forward);

  auto get_neighbors = [](glm::ivec3 vec) -> std::vector<std::shared_ptr<Chunk>> {
    std::shared_ptr<Chunk> left, front, right, back;
    left = world->get_chunk_by_center(
        vec + glm::ivec3(static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0, 0));
    front = world->get_chunk_by_center(
        vec + glm::ivec3(0, 0, static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
    right = world->get_chunk_by_center(
        vec - glm::ivec3(static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0, 0));
    back = world->get_chunk_by_center(
        vec - glm::ivec3(0, 0, static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
    return {left, front, right, back};
  };

  if (!Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL) &&
      Input::WasMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
    strcpy(textKeyStatus, "Right click");
    strcpy(textKeyDescription, "Casting ray");

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    Ray ray = screenPosToWorldRay(window, mouseX, mouseY, viewT, projectionT);

    if (ray.did_hit(world)) {  // Remove a block
      std::cout << "Ray hit a block with center: " << ray.m_hitcords.x << " " << ray.m_hitcords.y
                << " " << ray.m_hitcords.z << std::endl;
      auto block = world->get_block_by_center(ray.m_hitcords);
      if (block) block->remove();
      auto _chunk = world->get_chunk_by_center(ray.m_hitcords);
      auto _biome = world->get_biome_by_center(ray.m_hitcords);
      // Set the dirty bit
      _chunk->dirtybit = true;
      _biome->dirtybit = true;
      auto vec = ray.m_hitcords;

      int cordz = (ray.m_hitcords.z % static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE));
      int cordx = (ray.m_hitcords.x % static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE));
      // If last block update adjacent chunk
      auto chunks1 = get_neighbors(vec);
      _chunk->Render(0, true, nullptr, nullptr, nullptr, nullptr);

      bool update_boundary = false;
      if (cordz == static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE) - 1) {
        auto chunks2 =
            get_neighbors(vec + glm::ivec3(0, 0, static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
        if (chunks1[1]) {
          std::cout << "[FRONT] Updating neighbouring chunk\n";
          chunks1[1]->Render(0, true, nullptr, nullptr, nullptr, nullptr);
          chunks1[1]->Render(0, false, chunks2[0], chunks2[1], chunks2[2], chunks2[3]);
        }
      }

      if (cordx == static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE) - 1) {
        auto chunks2 =
            get_neighbors(vec + glm::ivec3(static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0, 0));
        if (chunks1[0]) {
          std::cout << "[LEFT] Updating neighbouring chunk\n";
          chunks1[0]->Render(0, true, nullptr, nullptr, nullptr, nullptr);
          chunks1[0]->Render(0, false, chunks2[0], chunks2[1], chunks2[2], chunks2[3]);
        }
      }

      if (cordz == 1) {
        auto chunks2 =
            get_neighbors(vec - glm::ivec3(0, 0, static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
        if (chunks1[3]) {
          std::cout << "[BACK] Updating neighbouring chunk\n";
          chunks1[3]->Render(0, true, nullptr, nullptr, nullptr, nullptr);
          chunks1[3]->Render(0, false, chunks2[0], chunks2[1], chunks2[2], chunks2[3]);
        }
      }

      if (cordx == 1) {
        auto chunks2 =
            get_neighbors(vec - glm::ivec3(static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0, 0));
        if (chunks1[2]) {
          std::cout << "[RIGHT] Updating neighbouring chunk\n";
          chunks1[2]->Render(0, true, nullptr, nullptr, nullptr, nullptr);
          chunks1[2]->Render(0, false, chunks2[0], chunks2[1], chunks2[2], chunks2[3]);
        }
      }
      _chunk->Render(0, false, chunks1[0], chunks1[1], chunks1[2], chunks1[3]);

      // world->RenderWorld();
    } else {
      std::cout << "Ray didn't hit any block\n";
    }
  } else if (
      !Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL) &&
      Input::WasMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {  // Add a block
    strcpy(textKeyStatus, "Middle click");
    strcpy(textKeyDescription, "Casting ray");

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    Ray ray = screenPosToWorldRay(window, mouseX, mouseY, viewT, projectionT);

    if (ray.did_hit(world)) {
      std::cout << "Ray hit a block with center: " << ray.m_hitcords.x << " " << ray.m_hitcords.y
                << " " << ray.m_hitcords.z << std::endl;
      glm::ivec3 prev_blk = ray.m_hitcords + ray.m_hitnormal * static_cast<int>(BLOCK_SIZE);
      auto block = world->get_block_by_center(prev_blk);
      if (block) block->add(bltype);
      auto _chunk = world->get_chunk_by_center(ray.m_hitcords);
      auto _biome = world->get_biome_by_center(ray.m_hitcords);
      // Update dirty bit
      _chunk->dirtybit = true;
      _biome->dirtybit = true;

      auto vec = ray.m_hitcords;

      int cordz = (ray.m_hitcords.z % static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE));
      int cordx = (ray.m_hitcords.x % static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE));
      // If last block update adjacent chunk
      auto chunks1 = get_neighbors(vec);
      _chunk->Render(0, true, nullptr, nullptr, nullptr, nullptr);

      if (cordz == static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE) - 1) {
        auto chunks2 =
            get_neighbors(vec + glm::ivec3(0, 0, static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
        if (chunks1[1]) {
          std::cout << "[FRONT] Updating neighbouring chunk\n";
          chunks1[1]->Render(0, true, nullptr, nullptr, nullptr, nullptr);
          chunks1[1]->Render(0, false, chunks2[0], chunks2[1], chunks2[2], chunks2[3]);
        }
      }

      if (cordx == static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE) - 1) {
        auto chunks2 =
            get_neighbors(vec + glm::ivec3(static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0, 0));
        if (chunks1[0]) {
          std::cout << "[LEFT] Updating neighbouring chunk\n";
          chunks1[0]->Render(0, true, nullptr, nullptr, nullptr, nullptr);
          chunks1[0]->Render(0, false, chunks2[0], chunks2[1], chunks2[2], chunks2[3]);
        }
      }

      if (cordz == 1) {
        auto chunks2 =
            get_neighbors(vec - glm::ivec3(0, 0, static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
        if (chunks1[3]) {
          std::cout << "[BACK] Updating neighbouring chunk\n";
          chunks1[3]->Render(0, true, nullptr, nullptr, nullptr, nullptr);
          chunks1[3]->Render(0, false, chunks2[0], chunks2[1], chunks2[2], chunks2[3]);
        }
      }

      if (cordx == 1) {
        auto chunks2 =
            get_neighbors(vec - glm::ivec3(static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0, 0));
        if (chunks1[2]) {
          std::cout << "[RIGHT] Updating neighbouring chunk\n";
          chunks1[2]->Render(0, true, nullptr, nullptr, nullptr, nullptr);
          chunks1[2]->Render(0, false, chunks2[0], chunks2[1], chunks2[2], chunks2[3]);
        }
      }
      _chunk->Render(0, false, chunks1[0], chunks1[1], chunks1[2], chunks1[3]);
    } else {
      std::cout << "Ray didn't hit any block\n";
    }
  }

  if (Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL)) {
    if (Input::WasMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
      double mouseX, mouseY;
      glfwGetCursorPos(window, &mouseX, &mouseY);

      Ray ray = screenPosToWorldRay(window, mouseX, mouseY, viewT, projectionT);

      if (ray.did_hit(world)) {
        std::cout << "[SHIFT] Ray hit a block with center: " << ray.m_hitcords.x << " "
                  << ray.m_hitcords.y << " " << ray.m_hitcords.z << std::endl;
        auto chunk = world->get_chunk_by_center(ray.m_hitcords);
        world->load_model(
            ray.m_hitcords + glm::ivec3(0, BLOCK_SIZE, 0),
            "models/" + MODEL_ARRAY[mdtype] + ".bin");
      }
    }
  }

  if (Nokeypressed) {
    strcpy(textKeyStatus, "Listening for key events...");
    strcpy(textKeyDescription, "Listening for key events...");
  }
}

void Player::handle_stats() {
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // ImGui UI menu
  ImGui::Begin("Main", NULL, ImGuiWindowFlags_AlwaysAutoResize);
  if (ImGui::CollapsingHeader("Information", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text(
        "%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text("Key Status: %s", textKeyStatus);
    ImGui::Text("Key Description: %s", textKeyDescription);
    ImGui::Text("Active Player: %d", activePlayer);
    ImGui::Text("Block Selected: %s", BLOCK_ARRAY[bltype].c_str());
    ImGui::Text(
        "Player %d position: (%.2f, %.2f, %.2f)",
        activePlayer,
        m_cameracontroller->GetCamera()->GetPosition().x,
        m_cameracontroller->GetCamera()->GetPosition().y,
        m_cameracontroller->GetCamera()->GetPosition().z);
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

  // Selected Block
  // Begin a child region with fixed height and automatic scrollbar
  ImGui::BeginChild("Selected Block", ImVec2(400, 75), true, ImGuiWindowFlags_HorizontalScrollbar);

  ImGui::Text("Selected Block");
  for (int i = 0; i < BLOCK_TYPES; i++) {
    if (ImGui::RadioButton(BLOCK_ARRAY[i].c_str(), (int *)&bltype, i)) {
    }
  }

  ImGui::EndChild();

  // Selected Model
  // Begin a child region with fixed height and automatic scrollbar
  ImGui::BeginChild("Selected Model", ImVec2(400, 75), true, ImGuiWindowFlags_HorizontalScrollbar);

  ImGui::Text("Selected Model");
  for (int i = 0; i < MODEL_TYPES; i++) {
    if (ImGui::RadioButton(MODEL_ARRAY[i].c_str(), (int *)&mdtype, i)) {
    }
  }

  ImGui::EndChild();


  // Save Model
  ImGui::BeginChild("Save", ImVec2(400, 150), true, ImGuiWindowFlags_HorizontalScrollbar);
  static int X = 0;
  static int Y = 0;
  static char model_name[32];

  ImGui::Text("Save Model");
  // Input fields
  ImGui::InputInt("Chunk X", &X);
  ImGui::InputInt("Chunk Y", &Y);

  // Clamp negative values
  if (X < 0) X = 0;
  if (Y < 0) Y = 0;
  ImGui::InputText("Model Name", model_name, IM_ARRAYSIZE(model_name));

  if (ImGui::Button("Save")) {
    // Save the model in chunk 0 included between ref
    auto chunk = world->get_chunk_by_center(
        glm::ivec3(
            X * CHUNK_BLOCK_COUNT * BLOCK_SIZE + HALF_BLOCK_SIZE,
            HALF_BLOCK_SIZE,
            Y * CHUNK_BLOCK_COUNT * BLOCK_SIZE + HALF_BLOCK_SIZE));

    world->save_model(chunk, std::string(model_name));
  };

  ImGui::EndChild();


  // Save World
  ImGui::BeginChild("Save World", ImVec2(400, 150), true, ImGuiWindowFlags_HorizontalScrollbar);
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
  // Modelling transformations (Model -> World coordinates)
  modelT = glm::scale(glm::mat4(1.0f), glm::vec3(1.0, 1.0, 1.0));
  modelT = glm::translate(modelT, glm::vec3(0.0f, 0.0f, 0.0f));

  // Pass on the modelling matrix to the vertex shader
  glUseProgram(program);
  vModel_uniform = glGetUniformLocation(program, "vModel");
  if (vModel_uniform == -1) {
    fprintf(stderr, "Could not bind location: vModel\n");
    exit(0);
  }
  glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(modelT));
}

void Player::setupModelTransformationAxis(
    unsigned int &program,
    float rot_angle,
    glm::vec3 rot_axis) {
  // Modelling transformations (Model -> World coordinates)
  modelT = glm::rotate(glm::mat4(1.0f), rot_angle, rot_axis);

  // Pass on the modelling matrix to the vertex shader
  glUseProgram(program);
  vModel_uniform = glGetUniformLocation(program, "vModel");
  if (vModel_uniform == -1) {
    fprintf(stderr, "Could not bind location: vModel\n");
    exit(0);
  }
  glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(modelT));
}

void Player::setupViewTransformation(
    unsigned int &program,
    std::unique_ptr<CameraController> &occ) {
  // Viewing transformations (World -> Camera coordinates
  //  viewT = glm::lookAt(glm::vec3(camPosition), glm::vec3(0.0, 0.0, 0.0),
  //  glm::vec3(0.0, 1.0, 0.0));
  viewT = occ->GetCamera()->GetViewMatrix();

  // Pass-on the viewing matrix to the vertex shader
  glUseProgram(program);
  vView_uniform = glGetUniformLocation(program, "vView");
  if (vView_uniform == -1) {
    fprintf(stderr, "Could not bind location: vView\n");
    exit(0);
  }
  glUniformMatrix4fv(vView_uniform, 1, GL_FALSE, glm::value_ptr(viewT));
}

void Player::setupProjectionTransformation(
    unsigned int &program,
    std::unique_ptr<CameraController> &occ) {
  // Projection transformation
  projectionT = occ->GetCamera()->GetProjectionMatrix();

  // Pass on the projection matrix to the vertex shader
  glUseProgram(program);
  vProjection_uniform = glGetUniformLocation(program, "vProjection");
  if (vProjection_uniform == -1) {
    fprintf(stderr, "Could not bind location: vProjection\n");
    exit(0);
  }
  glUniformMatrix4fv(vProjection_uniform, 1, GL_FALSE, glm::value_ptr(projectionT));
}

void Player::handle_transformations() {
  m_cameracontroller->SetAspectRatio((float)display_w / (float)display_h);
  glViewport(0, 0, display_w, display_h);

  glUseProgram(shaderProgram);
  // Setup MVP matrix
  glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
  glEnable(GL_DEPTH_TEST);  // restore
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_CULL_FACE);  // Enable OC
  glEnable(GL_BLEND);      // Enable BLENDING
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  // Render the mesh
  for (auto mesh : meshes) {
    mesh->render(m_cameracontroller);
  }

  glUniform3f(
      lightpos_uniform,
      m_cameracontroller->GetCamera()->GetPosition().x,
      m_cameracontroller->GetCamera()->GetPosition().y,
      m_cameracontroller->GetCamera()->GetPosition().z);

  glUniform3f(
      cameraPos_uniform,
      m_cameracontroller->GetCamera()->GetPosition().x,
      m_cameracontroller->GetCamera()->GetPosition().y,
      m_cameracontroller->GetCamera()->GetPosition().z);

  setupModelTransformationCube(shaderProgram);
  setupViewTransformation(shaderProgram, m_cameracontroller);
  setupProjectionTransformation(shaderProgram, m_cameracontroller);
}

void Player::update() {
  // handle player Input
  handle_input();
  // Display stats
  handle_stats();

  handle_transformations();
}
