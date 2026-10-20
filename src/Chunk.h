#pragma once
#include <memory>
#include <noise/noise.h>
#include <noiseutils.h>

#include "AssertThread.h"
#include "Block.h"
#include "Constants.hpp"
#include "Utils.h"
#include "VertexArray.h"
#include <cstdlib>

extern GLint chunkpos_uniform;
extern GLuint wireframemode;

class Chunk {
public:
  bool m_displayChunk = false, m_dirtyBit = false;
  uint32_t m_id{}, m_cntBlocks{}, m_saveId{}, m_type{}, m_cntBlocksTrans{};
  glm::ivec3 m_biomePos{0}, m_chunkPos{0};

  std::array<
      std::array<std::array<Block, CHUNK_BLOCK_COUNT>, CHUNK_BLOCK_COUNT>,
      CHUNK_BLOCK_COUNT>
      m_blocks;
  std::vector<uint32_t> m_cubeVertices, m_cubeVerticesTrans;

  std::unique_ptr<VertexArray> m_chunkVa, m_chunkVaTrans;
  std::unique_ptr<VertexBuffer> m_chunkVb, m_chunkVbTrans;

  Chunk();
  Chunk(uint32_t, glm::ivec3, glm::ivec3, bool, int);

  void Render(int, bool,
              std::shared_ptr<Chunk>, // left
              std::shared_ptr<Chunk>, // forward
              std::shared_ptr<Chunk>, // right
              std::shared_ptr<Chunk>, // back
              std::shared_ptr<Chunk>, // top
              std::shared_ptr<Chunk>  // bottom
  );

  void Setup_Landscape(uint32_t, uint32_t);
  uint32_t RenderFace(int, int, int);
  inline bool isSolid(int, int, int);
  inline bool isTransparent(int, int, int);
  inline bool isSameKind(int, int, int, int, int, int);
  void Draw(OBJ_TYPE, glm::dvec3);
  void Serialize(std::ostream &) const;
  bool Deserialize(std::istream &);
  void SetupVertexObjects();
  void UpdateVertexObjects();
  void RecycleChunk(uint, glm::ivec3, glm::ivec3, bool, int);
  static void SetupNoise(uint64_t seed) {
    // Set random seed for mountains
    s_mountainTerrain.SetSeed(seed);
    // s_mountainTerrain.SetFrequency(1.0 / 128.0);

    // Set random seed for flat terrain
    s_baseFlatTerrain.SetFrequency(2.0);
    s_baseFlatTerrain.SetSeed(seed);

    s_flatTerrain.SetSourceModule(0, s_baseFlatTerrain);
    s_flatTerrain.SetScale(0.085);
  };

private:
  // 4 5 6 7
  // 0 1 2 3
  static inline noise::module::RidgedMulti s_mountainTerrain;
  static inline noise::module::Billow s_baseFlatTerrain;
  static inline noise::module::ScaleBias s_flatTerrain;
};
