#include <memory>
#include <noise/noise.h>
#include <noiseutils.h>

#include "Block.h"
#include "Constants.hpp"
#include "IndexBuffer.h"
#include "Utils.h"
#include "VertexArray.h"

#pragma once

extern GLint chunkpos_uniform;
extern GLuint wireframemode;

class Chunk {
public:
  GLboolean m_displayChunk, m_dirtyBit;
  GLuint m_id, m_count, m_cntBlocks, m_saveId, m_type;
  GLuint m_countTrans, m_cntBlocksTrans;
  glm::ivec3 m_biomePos, m_chunkPos;

  std::array<
      std::array<std::array<Block, CHUNK_BLOCK_COUNT>, CHUNK_BLOCK_COUNT>,
      CHUNK_BLOCK_COUNT>
      m_blocks;
  std::vector<GLuint> m_cubeVertices, m_cubeVerticesTrans;
  std::vector<GLuint> m_cubeIndices, m_cubeIndicesTrans;

  std::unique_ptr<VertexArray> m_chunkVa, m_chunkVaTrans;
  std::unique_ptr<VertexBuffer> m_chunkVb, m_chunkVbTrans;
  std::unique_ptr<IndexBuffer> m_chunkIb, m_chunkIbTrans;

  Chunk();
  Chunk(uint, glm::ivec3, glm::ivec3, GLboolean, int);

  void Render(int, bool,
              std::shared_ptr<Chunk>,  // left
              std::shared_ptr<Chunk>,  // forward
              std::shared_ptr<Chunk>,  // right
              std::shared_ptr<Chunk>); // back
  void Setup_Landscape(GLint, GLint);
  GLuint RenderFace(int, int, int);
  inline GLboolean isSolid(int, int, int);
  inline GLboolean isTransparent(int, int, int);
  inline GLboolean isSameKind(int, int, int, int, int, int);
  void Draw(OBJ_TYPE, glm::dvec3);
  void Serialize(std::ostream &) const;
  bool Deserialize(std::istream &);
  void SetupVertexObjects();
  void UpdateVertexObjects();
  void RecycleChunk(uint, glm::ivec3, glm::ivec3, GLboolean, int);
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
