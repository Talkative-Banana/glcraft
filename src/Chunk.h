#include <noise/noise.h>
#include <noiseutils.h>

#include <memory>

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

  std::array<std::array<std::array<Block, CHUNK_BLOCK_COUNT>, CHUNK_BLOCK_COUNT>, CHUNK_BLOCK_COUNT>
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
  std::vector<std::pair<std::vector<GLuint>, std::vector<GLuint>>> renderverttrans;

  Chunk();
  Chunk(uint _id, glm::ivec3 biomepos, glm::ivec3 position, GLboolean display, int type);

  void Render(
      int setup,
      bool firstRun,
      std::shared_ptr<Chunk>,   // left
      std::shared_ptr<Chunk>,   // forward
      std::shared_ptr<Chunk>,   // right
      std::shared_ptr<Chunk>);  // back
  void Setup_Landscape(GLint X, GLint Y);
  GLuint RenderFace(std::vector<GLint> &&position);
  inline GLboolean isSolid(const std::vector<GLint> &postion);
  inline GLboolean isTransparent(const std::vector<GLint> &position);
  void Draw(OBJ_TYPE type);
  void Serialize(std::ostream &os) const;
  bool Deserialize(std::istream &is);
};
