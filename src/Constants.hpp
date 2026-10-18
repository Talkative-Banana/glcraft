#pragma once

#include "Utils.h"
#include <array>
#include <string>
#include <variant>
constexpr static double BLOCK_SIZE = 2.0;      // BLOCK LENGTH
constexpr static double HALF_BLOCK_SIZE = 1.0; // HALF_BLOCK LENGTH
constexpr static double NEAR_PLANE = 0.01f;
constexpr static double FAR_PLANE = 1000.0f;
constexpr static uint64_t TOTAL_STEPS = 32;
constexpr static double STEP_SIZE = 1.0;
constexpr static uint64_t CHUNK_COUNTX = 4;
constexpr static uint64_t BIOME_COUNTX = 8192;
constexpr static uint64_t CHUNK_COUNTZ = 4;
constexpr static uint64_t BIOME_COUNTZ = 8192;
constexpr static uint64_t BIOME_COUNTY = 8192;
constexpr static uint64_t CHUNK_BLOCK_COUNT = 32;
constexpr static uint64_t BIOME_SIZE =
    BLOCK_SIZE * CHUNK_BLOCK_COUNT * CHUNK_COUNTX;
constexpr static uint64_t RENDER_DISTANCE = BIOME_SIZE * 2; // 2 BIOME
// Maximum # biomes that can be addressed in a frame to avoid freezing
constexpr static int MAX_PROCESS_BIOME_IN_FRAME = 16;

constexpr static int BIOME_HEIGHT = CHUNK_BLOCK_COUNT * BLOCK_SIZE;
constexpr static int BIOME_LENGTH = CHUNK_COUNTX * BIOME_HEIGHT;
constexpr static int BIOME_WIDTH = CHUNK_COUNTZ * BIOME_HEIGHT;

constexpr static uint64_t CHUNK_HEIGHT = CHUNK_BLOCK_COUNT * BLOCK_SIZE;
constexpr static uint64_t CHUNK_LENGTH = CHUNK_BLOCK_COUNT * BLOCK_SIZE;

constexpr static double OFFSET = 0.01f;
constexpr static double PLAYER_HEIGHT = 2 * BLOCK_SIZE;
constexpr double GRAVITY_ACCEL = -32.0 * BLOCK_SIZE; // -64 units/s^2
// 2.5 units, clears a 2-unit block
constexpr double JUMP_HEIGHT = 1.25 * BLOCK_SIZE;
// ~17.9 units/s
const double JUMP_SPEED = std::sqrt(2.0 * -GRAVITY_ACCEL * JUMP_HEIGHT);
constexpr double TERMINAL_VEL = -60.0 * BLOCK_SIZE; // -120 units/s
constexpr double PLAYER_RADIUS = 0.3 * BLOCK_SIZE;  // 0.6 units
constexpr static uint64_t PLAYER_COUNT = 128;
constexpr static uint64_t MODEL_TYPES = 4;
constexpr static uint64_t BIOME_TYPES = 4;
constexpr static uint64_t BIOME_BLOCK_COUNT = 6;

constexpr static uint64_t BLOCK_POSX = 31ull << 10;
constexpr static uint64_t BLOCK_POSY = 31ull << 5;
constexpr static uint64_t BLOCK_POSZ = 31ull;
constexpr static uint64_t BACK_FACE = 1ull << 17;
constexpr static uint64_t FRONT_FACE = 1ull << 18;
constexpr static uint64_t LEFT_FACE = 1ull << 19;
constexpr static uint64_t RIGHT_FACE = 1ull << 20;
constexpr static uint64_t TOP_FACE = 1ull << 21;
constexpr static uint64_t BOTTOM_FACE = 1ull << 22;
constexpr static uint64_t FACE_MASK_BITS = 6ull;
constexpr static uint64_t TYPE_MASK_BITS = 9ull;
constexpr static uint64_t FACE_MASK = ((1ull << FACE_MASK_BITS) - 1) << 17;
constexpr static uint64_t TYPE_MASK = ((1ull << TYPE_MASK_BITS) - 1) << 23;
constexpr static float WALKING_SPEED = 20.0f;
constexpr static float RUNNING_SPEED = 30.0f;
constexpr static float SMUGED_SPEED = 10.0f;

constexpr static int IMGUI_TEXT_CAPACITY = 256;
// Non const
static int SCREEN_HEIGHT = 640;
static int SCREEN_WIDTH = 640;

enum class BLOCK_TYPE {
  REF_BLOCK,
  DIRT_BLOCK,
  STONE_BLOCK,
  BARK_BLOCK,
  LEAF_BLOCK,
  GRASS_BLOCK,
  IRON_BLOCK,
  WOOD_BLOCK,
  WATER_BLOCK,
  GRAVEL_BLOCK,
  SAND_BLOCK,
  BEDROCK_BLOCK,
  BRICK_BLOCK,
  SNOW_BLOCK,
  HARD_SNOW_BLOCK,
  LAVA_BLOCK,
  GLASS_BLOCK,
  NUM_BLOCK,
};

enum class WEATHER {
  CLOUDY,
  HAILSTORM,
  SNOWSTORM,
};

enum class SOUNDSTATUS {
  STOPPED,
  PAUSED,
  PLAYING,
  UNKNOWN,
};

enum class BIOMESTATUS {
  IDLE,
  SETUP,
  REFRESH,
  WAITING,
  FINAL,
};

enum class OBJ_TYPE {
  OPAQUE_,
  TRANSPARENT_,
  UI_,
};

static std::array<std::string, static_cast<int>(BLOCK_TYPE::NUM_BLOCK)>
    BLOCK_ARRAY = {
        "REF",   "DIRT", "STONE",     "BARK",   "LEAF",  "GRASS",
        "IRON",  "WOOD", "WATER",     "GRAVEL", "SAND",  "BEDROCK",
        "BRICK", "SNOW", "HARD_SNOW", "LAVA",   "GLASS",
};

static std::array<std::string, MODEL_TYPES> MODEL_ARRAY = {
    "monostate", "tree", "tree_std", "furnace"};

static std::array<std::string, BIOME_TYPES> BIOME_ARRAY = {
    "GRASSLAND", "DESERT", "SAVANNA", "ICE"};

static std::array<std::array<BLOCK_TYPE, BIOME_BLOCK_COUNT>, BIOME_TYPES>
    BIOME_BLOCK_TYPES = {{{{BLOCK_TYPE::GRASS_BLOCK, BLOCK_TYPE::DIRT_BLOCK,
                            BLOCK_TYPE::GRAVEL_BLOCK, BLOCK_TYPE::STONE_BLOCK,
                            BLOCK_TYPE::SAND_BLOCK, BLOCK_TYPE::WATER_BLOCK}},
                          {{BLOCK_TYPE::SNOW_BLOCK, BLOCK_TYPE::SNOW_BLOCK,
                            BLOCK_TYPE::SNOW_BLOCK, BLOCK_TYPE::HARD_SNOW_BLOCK,
                            BLOCK_TYPE::SAND_BLOCK, BLOCK_TYPE::WATER_BLOCK}},
                          {{BLOCK_TYPE::SAND_BLOCK, BLOCK_TYPE::SAND_BLOCK,
                            BLOCK_TYPE::SAND_BLOCK, BLOCK_TYPE::SAND_BLOCK,
                            BLOCK_TYPE::SAND_BLOCK, BLOCK_TYPE::WATER_BLOCK}},
                          {{BLOCK_TYPE::SAND_BLOCK, BLOCK_TYPE::SAND_BLOCK,
                            BLOCK_TYPE::SAND_BLOCK, BLOCK_TYPE::SAND_BLOCK,
                            BLOCK_TYPE::SAND_BLOCK, BLOCK_TYPE::WATER_BLOCK}}}};

struct PlayerState {
  glm::dvec3 pos, fwd, vel, up;
};

struct WorldState {
  glm::ivec3 blockpos;
  uint64_t blk;
  GLuint model_idx;
};

struct State {
  std::variant<std::monostate, PlayerState, WorldState> _data;
  GLuint id;
  bool enforce{false};
  float ts;
};

const std::string ThunderPath = "./assets/audio/calm-thunderstorm-mono.wav";
const std::string BlizzardPath = "./assets/audio/blizzard.ogg";

const std::string ShaderProgramVS = "./shaders/vshaderWorld.vs";
const std::string ShaderProgramFS = "./shaders/fshaderWorld.fs";
const std::string ShaderProgramGS = "./shaders/gshaderWorld.gs";

const std::string ShaderProgram2VS = "./shaders/vshaderAsset.vs";
const std::string ShaderProgram2FS = "./shaders/fshaderAsset.fs";

const std::string ShaderProgramUIVS = "./shaders/vshaderUI.vs";
const std::string ShaderProgramUIFS = "./shaders/fshaderUI.fs";

const std::string ShaderProgramPSVS = "./shaders/vshaderPS.vs";
const std::string ShaderProgramPSFS = "./shaders/fshaderPS.fs";

const std::string WaterTexture = "./textures/water.png";
const std::string SmokeTexture = "./textures/smoke.png";
const std::string SnowTexture = "./textures/snow.png";
const std::string RainTexture = "./textures/rain.png";

const std::string GaugeTexture = "./textures/gauge.png";
const std::string DefaultTexture = "./textures/default_texture.png";

const std::string BunnyModel = "./assets/bunny.obj";
const std::string BuddhaModel = "./assets/buddha.obj";
