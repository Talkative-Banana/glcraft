#pragma once
#include "Utils.h"
#include <../stb/stb_image.h>
#include <../stb/stb_image_write.h>
#include <memory>

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
#include "Server.h"
#include "SmokeEffect.h"
#include "SnowEffect.h"
#include "SoundSystem.hpp"
#include "Texture.h"
#include "UI.h"
#include "WaterEffect.h"
#include "World.h"

class Game {
public:
  Game() = delete;
  Game(std::string savePath = "default.bin");
  void run();
  void shutdown();

private:
  // UI System
  std::unique_ptr<UI> m_ui = nullptr;
  // Sound System
  std::unique_ptr<SoundSystem> m_ss = nullptr;
  // Particle Systems
  std::unique_ptr<ParticleSystem> m_waterPs = nullptr;
  std::unique_ptr<ParticleSystem> m_smokePs = nullptr;
  std::unique_ptr<ParticleSystem> m_snowPs = nullptr;
  std::unique_ptr<ParticleSystem> m_rainPs = nullptr;
  std::unique_ptr<Texture> m_atlas = nullptr;
  GameSound m_thundersound, m_blizzardsound;
  uint32_t m_smId{}, m_snId{}, m_weId{}, m_rnId{};
  std::string m_savePath;
  GLuint m_shaderProgram, m_shaderProgram2;
  GLuint m_shaderProgramUI, m_shaderProgramPS;
  int m_port = 8080;
  boost::asio::io_context m_ioc;
  std::thread m_networkingThread;

private:
  void bindUniforms();
  uint32_t allocatePlayerId();
  static void updatePlayer(const std::string &);
};
