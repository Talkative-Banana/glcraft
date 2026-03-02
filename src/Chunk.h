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
  GLboolean displaychunk, dirtybit;
  GLuint id, count, cntblocks, save_id, type;
  GLuint counttrans, cntblockstrans;
  glm::ivec3 biomepos, chunkpos;

  std::array<
      std::array<std::array<Block, CHUNK_BLOCK_COUNT>, CHUNK_BLOCK_COUNT>,
      CHUNK_BLOCK_COUNT>
      blocks;
  std::vector<GLuint> cube_vertices;
  std::vector<GLuint> cube_indices;
  std::unique_ptr<VertexArray> chunkva;
  std::unique_ptr<VertexBuffer> chunkvb;
  std::unique_ptr<IndexBuffer> chunkib;
  std::vector<std::pair<std::vector<GLuint>, std::vector<GLuint>>> rendervert;

  std::vector<GLuint> cube_verticestrans;
  std::vector<GLuint> cube_indicestrans;
  std::unique_ptr<VertexArray> chunkvatrans;
  std::unique_ptr<VertexBuffer> chunkvbtrans;
  std::unique_ptr<IndexBuffer> chunkibtrans;
  std::vector<std::pair<std::vector<GLuint>, std::vector<GLuint>>>
      renderverttrans;

  Chunk();
  Chunk(uint _id, glm::ivec3 biomepos, glm::ivec3 position, GLboolean display,
        int type);

  void Render(int setup, bool firstRun,
              std::shared_ptr<Chunk>,  // left
              std::shared_ptr<Chunk>,  // forward
              std::shared_ptr<Chunk>,  // right
              std::shared_ptr<Chunk>); // back
  void Setup_Landscape(GLint X, GLint Y);
  GLuint RenderFace(std::vector<GLint> &&position);
  inline GLboolean isSolid(const std::vector<GLint> &postion);
  inline GLboolean isTransparent(const std::vector<GLint> &position);
  inline GLboolean isSameKind(const std::vector<GLint> &,
                              const std::vector<GLint> &);
  void Draw(OBJ_TYPE type, glm::dvec3 cameraPos);
  void Serialize(std::ostream &os) const;
  bool Deserialize(std::istream &is);
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
