#pragma once

#include <array>
#include <string>
constexpr static float BLOCK_SIZE = 2.0;       // BLOCK LENGTH
constexpr static float HALF_BLOCK_SIZE = 1.0;  // HALF_BLOCK LENGTH
constexpr static float NEAR_PLANE = 0.01f;
constexpr static float FAR_PLANE = 1000.0f;
constexpr static int TOTAL_STEPS = 32;
constexpr static float STEP_SIZE = 1.0;
constexpr static int CHUNK_COUNTX = 4;
constexpr static int BIOME_COUNTX = 1024;
constexpr static int CHUNK_COUNTZ = 4;
constexpr static int BIOME_COUNTZ = 1024;
constexpr static int CHUNK_BLOCK_COUNT = 32;
constexpr static int BIOME_SIZE = BLOCK_SIZE * CHUNK_BLOCK_COUNT * CHUNK_COUNTX;
constexpr static int RENDER_DISTANCE = BIOME_SIZE * 2;  // 2 BIOME

constexpr static float OFFSET = 0.01f;
constexpr static float PLAYER_HEIGHT = 2 * BLOCK_SIZE;
constexpr static float GRAVITY = 0.98f;
constexpr static int PLAYER_COUNT = 128;
constexpr static int BLOCK_TYPES = 12;
constexpr static int MODEL_TYPES = 2;
constexpr static int BIOME_TYPES = 4;
constexpr static int BIOME_BLOCK_COUNT = 5;

constexpr static int BACK_FACE = 1 << 17;
constexpr static int FRONT_FACE = 1 << 18;
constexpr static int LEFT_FACE = 1 << 19;
constexpr static int RIGHT_FACE = 1 << 20;
constexpr static int FACE_MASK_BITS = 6;
constexpr static int TYPE_MASK_BITS = 5;
constexpr static int FACE_MASK = ((1 << FACE_MASK_BITS) - 1) << 17;
constexpr static int TYPE_MASK = ((1 << TYPE_MASK_BITS) - 1) << 23;

constexpr static int IMGUI_TEXT_CAPACITY = 256;
// Non const
static int SCREEN_HEIGHT = 640;
static int SCREEN_WIDTH = 640;

constexpr static int REF_BLOCK = 0;
constexpr static int DIRT_BLOCK = 1;
constexpr static int STONE_BLOCK = 2;
constexpr static int BARK_BLOCK = 3;
constexpr static int LEAF_BLOCK = 4;
constexpr static int GRASS_BLOCK = 5;
constexpr static int IRON_BLOCK = 6;
constexpr static int WOOD_BLOCK = 7;
constexpr static int WATER_BLOCK = 8;
constexpr static int GRAVEL_BLOCK = 9;
constexpr static int SAND_BLOCK = 10;
constexpr static int BEDROCK_BLOCK = 11;

static std::array<std::string, BLOCK_TYPES> BLOCK_ARRAY = {
    "REF",
    "DIRT",
    "STONE",
    "BARK",
    "LEAF",
    "GRASS",
    "IRON",
    "WOOD",
    "WATER",
    "GRAVEL",
    "SAND",
    "BEDROCK"};

static std::array<std::string, MODEL_TYPES> MODEL_ARRAY = {"tree", "tree_std"};

static std::array<std::string, BIOME_TYPES> BIOME_ARRAY = {"GRASSLAND", "DESERT", "SAVANNA", "ICE"};

static std::array<std::array<int, BIOME_BLOCK_COUNT>, BIOME_TYPES> BIOME_BLOCK_TYPES = {
    {{{GRASS_BLOCK, DIRT_BLOCK, GRAVEL_BLOCK, STONE_BLOCK, BEDROCK_BLOCK}},
     {{SAND_BLOCK, SAND_BLOCK, SAND_BLOCK, SAND_BLOCK, BEDROCK_BLOCK}},
     {{SAND_BLOCK, SAND_BLOCK, SAND_BLOCK, SAND_BLOCK, BEDROCK_BLOCK}},
     {{SAND_BLOCK, SAND_BLOCK, SAND_BLOCK, SAND_BLOCK, BEDROCK_BLOCK}}}};
