#include "Chunk.h"

#include <cstdlib>
#include <random>
#include <string>

#include "Renderer.h"
#include "World.h"

extern std::unique_ptr<World> world;

Chunk::Chunk() {}

Chunk::Chunk(uint _id, glm::ivec3 _biomepos, glm::ivec3 position,
             GLboolean display, int _type) {
  // Check if the chunk needs to be loaded from disk
  m_id = _id;
  m_count = 0;
  m_countTrans = 0;
  m_type = _type;
  m_biomePos = _biomepos;
  m_displayChunk = display;
  int i = abs(_biomepos.x / (BIOME_LENGTH));
  int j = abs(_biomepos.z / (BIOME_LENGTH));
  int k = abs(_biomepos.y) / (BIOME_HEIGHT);
  uint biome_uq_id = BIOME_COUNTZ * BIOME_COUNTX * k + BIOME_COUNTZ * j + i;
  m_saveId =
      (biome_uq_id << static_cast<int>(log2(CHUNK_COUNTZ * CHUNK_COUNTX))) +
      _id;
  m_chunkPos = glm::ivec3(CHUNK_LENGTH * (m_id / CHUNK_COUNTX) + m_biomePos.x,
                          m_biomePos.y,
                          CHUNK_LENGTH * (m_id % CHUNK_COUNTZ) + m_biomePos.z);

  if (world->m_loadMap.find(m_saveId) != world->m_loadMap.end()) {
    // Empty Chunk
    std::cout << "Setting A chunk with loaded ID: " << m_saveId << '\n';
    Chunk &loaded_chunk = world->m_loadMap[m_saveId];
    m_blocks = loaded_chunk.m_blocks;
    m_dirtyBit = true;
  } else {
    Setup_Landscape(position.x + (_biomepos.z / (CHUNK_LENGTH)),
                    position.z + (_biomepos.x / (CHUNK_LENGTH)));
    m_dirtyBit = false;
  }

  // Reserve for vectors
  m_cubeIndices.resize(50'000);
  m_cubeIndicesTrans.resize(50'000);

  m_cubeVertices.resize(8'00'000);
  m_cubeVerticesTrans.resize(8'00'000);
}

void Chunk::RecycleChunk(uint _id, glm::ivec3 _biomepos, glm::ivec3 position,
                         GLboolean display, int _type) {
  // Check if the chunk needs to be loaded from disk
  m_id = _id;
  m_count = 0;
  m_countTrans = 0;
  m_type = _type;
  m_biomePos = _biomepos;
  m_displayChunk = display;
  int i = abs(_biomepos.x / (BIOME_LENGTH));
  int j = abs(_biomepos.z / (BIOME_LENGTH));
  int k = abs(_biomepos.y) / (BIOME_HEIGHT);
  uint biome_uq_id = BIOME_COUNTZ * BIOME_COUNTX * k + BIOME_COUNTZ * j + i;
  m_saveId =
      (biome_uq_id << static_cast<int>(log2(CHUNK_COUNTZ * CHUNK_COUNTX))) +
      _id;
  m_chunkPos = glm::ivec3(CHUNK_LENGTH * (m_id / CHUNK_COUNTX) + m_biomePos.x,
                          m_biomePos.y,
                          CHUNK_LENGTH * (m_id % CHUNK_COUNTZ) + m_biomePos.z);

  if (world->m_loadMap.find(m_saveId) != world->m_loadMap.end()) {
    // Empty Chunk
    std::cout << "Setting A chunk with loaded ID: " << m_saveId << '\n';
    Chunk &loaded_chunk = world->m_loadMap[m_saveId];
    m_blocks = loaded_chunk.m_blocks;
    m_dirtyBit = true;
  } else {
    Setup_Landscape(position.x + (_biomepos.z / (CHUNK_LENGTH)),
                    position.z + (_biomepos.x / (CHUNK_LENGTH)));
    m_dirtyBit = false;
  }
}

inline GLboolean Chunk::isSolid(int x, int y, int z) {
  if ((x >= 0) && (x < CHUNK_BLOCK_COUNT) && (y >= 0) &&
      (y < CHUNK_BLOCK_COUNT) && (z >= 0) && (z < CHUNK_BLOCK_COUNT)) {

    // Check if neibhourung block is solid
    return ((m_blocks[x][y][z].is_solid()));
  }
  return false;
}

inline GLboolean Chunk::isTransparent(int x, int y, int z) {
  if ((x >= 0) && (x < CHUNK_BLOCK_COUNT) && (y >= 0) &&
      (y < CHUNK_BLOCK_COUNT) && (z >= 0) && (z < CHUNK_BLOCK_COUNT)) {

    // Check if neibhourung block is transparent
    return ((m_blocks[x][y][z].is_transparent()));
  }
  return false;
}

inline GLboolean Chunk::isSameKind(int x, int y, int z, int X, int Y, int Z) {
  if ((x >= 0) && (x < CHUNK_BLOCK_COUNT) && (y >= 0) &&
      (y < CHUNK_BLOCK_COUNT) && (z >= 0) && (z < CHUNK_BLOCK_COUNT)) {

    // Check if neibhourung blocks are of same kind
    bool res = (m_blocks[x][y][z].get_type() == m_blocks[X][Y][Z].get_type());
    return res;
  }
  return false;
}

// k blue i red j green
// ctrl x -> red facing me

GLuint Chunk::RenderFace(int x, int y, int z) {
  // 1 -> back face
  // 2 -> front face
  // 3 -> left face
  // 4 -> right face
  // 5 -> top face
  // 6 -> bottom face

  GLuint mask = 0;

  // Back face (z - 1)
  // No Need to draw back face if block behind is solid
  if (!isSolid(x, y, z - 1) ||
      (isTransparent(x, y, z - 1) && !isSameKind(x, y, z, x, y, z - 1)))
    mask |= (1 << 0);

  // Front face (z + 1)
  // No Need to draw front face if block in front is solid
  if (!isSolid(x, y, z + 1) ||
      (isTransparent(x, y, z + 1) && !isSameKind(x, y, z, x, y, z + 1)))
    mask |= (1 << 1);

  // Left face (x - 1)
  // No Need to draw left face if block in left is solid
  if (!isSolid(x - 1, y, z) ||
      (isTransparent(x - 1, y, z) && !isSameKind(x, y, z, x - 1, y, z)))
    mask |= (1 << 2);

  // Right face (x + 1)
  // No Need to draw right face if block in right is solid
  if (!isSolid(x + 1, y, z) ||
      (isTransparent(x + 1, y, z) && !isSameKind(x, y, z, x + 1, y, z)))
    mask |= (1 << 3);

  // Top face (y + 1)
  // No Need to draw top face if block on top is solid
  if (!isSolid(x, y + 1, z) ||
      (isTransparent(x, y + 1, z) && !isSameKind(x, y, z, x, y + 1, z)))
    mask |= (1 << 4);

  // Bottom face (y - 1)
  // No Need to draw bottom face if block on bottom is solid
  if (!isSolid(x, y - 1, z) ||
      (isTransparent(x, y - 1, z) && !isSameKind(x, y, z, x, y - 1, z)))
    mask |= (1 << 5);

  return mask;
}

void Chunk::Setup_Landscape(GLint X, GLint Z) {
  // Early return
  if (m_chunkPos.y != ((BIOME_COUNTY - 1) * CHUNK_HEIGHT)) {
    for (int x = 0; x < CHUNK_BLOCK_COUNT; x++) {
      for (int z = 0; z < CHUNK_BLOCK_COUNT; z++) {
        for (int y = 0; y < CHUNK_BLOCK_COUNT; y++) {
          glm::ivec3 ofs = {z, y, x};
          auto &biome_bltypes = BIOME_BLOCK_TYPES[m_type];
          BLOCK_TYPE &bltype = biome_bltypes[3];
          m_blocks[z][y][x] = Block(std::move(ofs), true, bltype);
        }
      }
    }
    return;
  }
  //
  // noise::utils::NoiseMap heightMap;
  // noise::utils::NoiseMapBuilderPlane heightMapBuilder;
  // heightMapBuilder.SetSourceModule(s_mountainTerrain);
  // heightMapBuilder.SetDestNoiseMap(heightMap);
  // heightMapBuilder.SetDestSize(128, 128);
  // int biomex = X / 4, biomez = Z / 4;
  // heightMapBuilder.SetBounds(biomex, biomex + 1, biomez, biomez + 1);
  // heightMapBuilder.Build();
  //
  // noise::utils::RendererImage renderer;
  // noise::utils::Image image;
  // renderer.SetSourceNoiseMap(heightMap);
  // renderer.SetDestImage(image);
  // renderer.Render();

  // noise::utils::WriterBMP writer;
  // writer.SetSourceImage(image);
  // writer.SetDestFilename("maps/tutorial" + std::to_string((4 * X + Z) / 16) +
  //                        ".bmp");
  // writer.WriteDestFile();

  // X %= 4, Z %= 4;

  int biomex = X / 4, biomez = Z / 4;
  X %= 4, Z %= 4;

  for (int x = 0; x < CHUNK_BLOCK_COUNT; x++) {
    for (int z = 0; z < CHUNK_BLOCK_COUNT; z++) {
      // Use the noise library to get the height value of x, z
      // noise::utils::Color color = image.GetValue(chunkx + x, chunkz + z);
      // Use the height map texture to get the height value of x, z
      // int height = std::max(2, static_cast<int>((color.blue / 255.0f)
      // * 32.0f));

      int chunkx = CHUNK_BLOCK_COUNT * Z, chunkz = CHUNK_BLOCK_COUNT * X;
      int pixelX = chunkx + x;
      int pixelZ = chunkz + z;

      double nx = biomex + (double)pixelX / 128.0;
      double nz = biomez + (double)pixelZ / 128.0;

      double value = s_mountainTerrain.GetValue(nx, 0, nz);

      // Normalize exactly like RendererImage
      double normalized = (value + 1.0) * 0.5;

      // Clamp to [0,1] because renderer clamps
      normalized = glm::clamp(normalized, 0.0, 1.0);

      // Convert to 0–255
      unsigned char blue = static_cast<unsigned char>(normalized * 255.0 + 0.5);
      int height = std::max(2, static_cast<int>((blue / 255.0f) * 32.0f));

      for (int y = 0; y < CHUNK_BLOCK_COUNT; y++) {
        glm::ivec3 ofs = {z, y, x};
        auto &biome_bltypes = BIOME_BLOCK_TYPES[m_type];
        BLOCK_TYPE bltype;
        if (y == height - 1) {
          if (y <= 10) {
            bltype = biome_bltypes[2]; // Depth top block GRAVEL
          } else if (y <= 15) {
            bltype = biome_bltypes[4]; // Depth top middle block SAND
          } else {
            bltype = biome_bltypes[0]; // Top Block GRASS
          }
        } else if (y <= 10) {
          bltype = biome_bltypes[3]; // Depth Block
        } else if (y >= height && y <= 15) {
          bltype = biome_bltypes[5]; // Water Block
        } else {
          bltype = biome_bltypes[1];
        }
        // mark them solid
        m_blocks[z][y][x] = Block(ofs, (y < height || y <= 15), bltype);
      }
    }
  }
}

void Chunk::Render(int setup, bool firstRun, std::shared_ptr<Chunk> left,
                   std::shared_ptr<Chunk> front, std::shared_ptr<Chunk> right,
                   std::shared_ptr<Chunk> back) {
  // Rerendering
  // if (!displaychunk) return;
  // Render OPAQUE blocks

  static const glm::ivec3 neighborOffsetsIdx[8] = {
      {0, 1, -1},  // b0
      {-1, 1, -1}, // b1   543
      {-1, 1, 0},  // b2   6 2
      {-1, 1, 1},  // b3   701
      {0, 1, 1},   // b4
      {1, 1, 1},   // b5
      {1, 1, 0},   // b6
      {1, 1, -1}   // b7
  };

  auto GetBlock = [&](glm::ivec3 &blockpos, glm::ivec3 offset) -> Block * {
    int i = blockpos.x, j = blockpos.y, k = blockpos.z;
    glm::ivec3 newPos = blockpos + offset;

    bool withinChunk = true;
    withinChunk &= newPos.x != -1 && newPos.x != CHUNK_BLOCK_COUNT;
    withinChunk &= newPos.y != -1 && newPos.y != CHUNK_BLOCK_COUNT;
    withinChunk &= newPos.z != -1 && newPos.z != CHUNK_BLOCK_COUNT;

    if (withinChunk) {
      return &this->m_blocks[newPos.x][newPos.y][newPos.z];
    }

    bool validX = newPos.x != -1 && newPos.x != CHUNK_BLOCK_COUNT;
    bool validY = newPos.y != -1 && newPos.y != CHUNK_BLOCK_COUNT;
    bool validZ = newPos.z != -1 && newPos.z != CHUNK_BLOCK_COUNT;

    if (!validX && validY && validZ) {
      if (newPos.x == -1 && right) {
        return &right->m_blocks[CHUNK_BLOCK_COUNT - 1][newPos.y][newPos.z];
      }
      if (newPos.x == CHUNK_BLOCK_COUNT && left) {
        return &left->m_blocks[0][newPos.y][newPos.z];
      }
    }

    if (!validZ && validX && validY) {
      if (newPos.z == -1 && back) {
        return &back->m_blocks[newPos.x][newPos.y][CHUNK_BLOCK_COUNT - 1];
      }
      if (newPos.z == CHUNK_BLOCK_COUNT && front) {
        return &front->m_blocks[newPos.x][newPos.y][0];
      }
    }

    // TODO: Remove it if not needed [just here for optimisation]
    if (!validY && validX && validZ) {
      return nullptr;
    }

    glm::ivec3 block_pos =
        m_chunkPos +
        glm::ivec3(BLOCK_SIZE * i, BLOCK_SIZE * j, BLOCK_SIZE * k) +
        glm::ivec3(HALF_BLOCK_SIZE, HALF_BLOCK_SIZE, HALF_BLOCK_SIZE) +
        offset * static_cast<int>(BLOCK_SIZE);
    return world->get_block_by_center(blockpos);
  };

  {
    m_cubeVertices.clear();
    m_cubeIndices.clear();
    m_count = 0, m_cntBlocks = 0;
    GLuint idx = 0;

    for (int i = 0; i < CHUNK_BLOCK_COUNT; i++) {
      for (int k = 0; k < CHUNK_BLOCK_COUNT; k++) {
        for (int j = 0; j < CHUNK_BLOCK_COUNT; j++) {
          // filled[0][0][0] = 1;
          if (!m_blocks[i][j][k].is_solid() ||
              m_blocks[i][j][k].is_transparent()) {
            // break;  // unsolid blocks
            continue;
          }
          GLuint mask = 0;
          if (firstRun) {
            // If its first run just save the mask
            mask = Chunk::RenderFace(i, j, k);
            m_blocks[i][j][k].blmask &= ~FACE_MASK;
            m_blocks[i][j][k].blmask |= (mask << 17);
            // continue;
          } else {
            // Second Run extract the mask
            if (i == 0 && right) {
              auto &blk1 = m_blocks[0][j][k];
              auto &blk2 = right->m_blocks[CHUNK_BLOCK_COUNT - 1][j][k];
              if ((blk1.is_solid()) &&
                  (blk2.is_solid() && !blk2.is_transparent())) {
                // Remove left face from current block
                blk1.blmask &= ~LEFT_FACE;
              }
            } else if (i == CHUNK_BLOCK_COUNT - 1 && left) {
              auto &blk1 = m_blocks[CHUNK_BLOCK_COUNT - 1][j][k];
              auto &blk2 = left->m_blocks[0][j][k];
              if ((blk1.is_solid()) &&
                  (blk2.is_solid() && !blk2.is_transparent())) {
                // Remove right face from current block
                blk1.blmask &= ~RIGHT_FACE;
              }
            }
            if (k == 0 && back) {
              auto &blk1 = m_blocks[i][j][0];
              auto &blk2 = back->m_blocks[i][j][CHUNK_BLOCK_COUNT - 1];
              if ((blk1.is_solid()) &&
                  (blk2.is_solid() && !blk2.is_transparent())) {
                // Remove back face from current block
                blk1.blmask &= ~BACK_FACE;
              }
            } else if (k == CHUNK_BLOCK_COUNT - 1 && front) {
              auto &blk1 = m_blocks[i][j][CHUNK_BLOCK_COUNT - 1];
              auto &blk2 = front->m_blocks[i][j][0];
              if ((blk1.is_solid()) &&
                  (blk2.is_solid() && !blk2.is_transparent())) {
                // Remove front face from current block
                blk1.blmask &= ~FRONT_FACE;
              }
            }
            mask = (m_blocks[i][j][k].blmask >> 17) & 63;
          }

          GLuint ac = 0;
          bool isBoundary = false;
          isBoundary |= i == 0 || i == CHUNK_BLOCK_COUNT - 1;
          isBoundary |= j == 0 || j == CHUNK_BLOCK_COUNT - 1;
          isBoundary |= k == 0 || k == CHUNK_BLOCK_COUNT - 1;

          if (isBoundary) {
            glm::ivec3 block_pos = glm::ivec3(i, j, k);
            for (int n = 0; n < 8; n++) {
              auto neighbor = GetBlock(block_pos, neighborOffsetsIdx[n]);
              if (neighbor && neighbor->is_standable()) {
                ac |= (1u << n); // set bit if solid
              }
            }
            glm::ivec3 offset = glm::ivec3(0, 1, -1);
            if (auto b0 = GetBlock(block_pos, offset)) {
              if (b0->is_standable())
                ac |= (1u << 8);
            }
          } else {
            glm::ivec3 block_pos = glm::ivec3(i, j, k);
            for (int n = 0; n < 8; n++) {
              auto Idx = block_pos + neighborOffsetsIdx[n];
              auto &neighbor = m_blocks[Idx.x][Idx.y][Idx.z];
              if (neighbor.is_standable()) {
                ac |= (1u << n); // set bit if solid
              }
            }

            auto &b0 = m_blocks[block_pos.x][block_pos.y + 1][block_pos.z - 1];
            if (b0.is_standable()) {
              ac |= (1u << 8);
            }
          }
          auto cnt = m_blocks[i][j][k].Render(mask, ac, idx, m_cubeIndices,
                                              m_cubeVertices);
          m_cntBlocks += cnt;
          idx += 24 * (cnt != 0), m_count += (cnt != 0);
        }
      }
    }
  }

  // Render transparent blocks
  {
    m_cubeVerticesTrans.clear();
    m_cubeIndicesTrans.clear();
    m_countTrans = 0, m_cntBlocksTrans = 0;
    GLuint idx = 0;

    for (int i = 0; i < CHUNK_BLOCK_COUNT; i++) {
      for (int k = 0; k < CHUNK_BLOCK_COUNT; k++) {
        for (int j = 0; j < CHUNK_BLOCK_COUNT; j++) {
          // filled[0][0][0] = 1;
          if (!m_blocks[i][j][k].is_solid() ||
              !m_blocks[i][j][k].is_transparent()) {
            // break;  // unsolid blocks
            continue;
          }
          GLuint mask = 0;
          if (firstRun) {
            // If its first run just save the mask
            mask = Chunk::RenderFace(i, j, k);
            m_blocks[i][j][k].blmask &= ~FACE_MASK;
            m_blocks[i][j][k].blmask |= (mask << 17);
            // continue;
          } else {
            // Second Run extract the mask
            if (i == 0 && right) {
              auto &blk1 = m_blocks[0][j][k];
              auto &blk2 = right->m_blocks[CHUNK_BLOCK_COUNT - 1][j][k];
              if ((blk1.is_solid()) && (blk2.is_solid())) {
                // Remove left face from current block
                blk1.blmask &= ~LEFT_FACE;
              }
            } else if (i == CHUNK_BLOCK_COUNT - 1 && left) {
              auto &blk1 = m_blocks[CHUNK_BLOCK_COUNT - 1][j][k];
              auto &blk2 = left->m_blocks[0][j][k];
              if ((blk1.is_solid()) && (blk2.is_solid())) {
                // Remove right face from current block
                blk1.blmask &= ~RIGHT_FACE;
              }
            }
            if (k == 0 && back) {
              auto &blk1 = m_blocks[i][j][0];
              auto &blk2 = back->m_blocks[i][j][CHUNK_BLOCK_COUNT - 1];
              if ((blk1.is_solid()) && (blk2.is_solid())) {
                // Remove back face from current block
                blk1.blmask &= ~BACK_FACE;
              }
            } else if (k == CHUNK_BLOCK_COUNT - 1 && front) {
              auto &blk1 = m_blocks[i][j][CHUNK_BLOCK_COUNT - 1];
              auto &blk2 = front->m_blocks[i][j][0];
              if ((blk1.is_solid()) && (blk2.is_solid())) {
                // Remove front face from current block
                blk1.blmask &= ~FRONT_FACE;
              }
            } else {
              // No update needed middle block
            }
            mask = (m_blocks[i][j][k].blmask >> 17) & 63;
          }

          GLuint ac = 0;
          bool isBoundary = false;
          isBoundary |= i == 0 || i == CHUNK_BLOCK_COUNT - 1;
          isBoundary |= j == 0 || j == CHUNK_BLOCK_COUNT - 1;
          isBoundary |= k == 0 || k == CHUNK_BLOCK_COUNT - 1;

          if (isBoundary) {
            glm::ivec3 block_pos = glm::ivec3(i, j, k);
            for (int n = 0; n < 8; n++) {
              auto neighbor = GetBlock(block_pos, neighborOffsetsIdx[n]);
              if (neighbor && neighbor->is_standable()) {
                ac |= (1u << n); // set bit if solid
              }
            }
            glm::ivec3 offset = glm::ivec3(0, 1, -1);
            if (auto b0 = GetBlock(block_pos, offset)) {
              if (b0->is_standable())
                ac |= (1u << 8);
            }
          } else {
            glm::ivec3 block_pos = glm::ivec3(i, j, k);
            for (int n = 0; n < 8; n++) {
              auto Idx = block_pos + neighborOffsetsIdx[n];
              auto &neighbor = m_blocks[Idx.x][Idx.y][Idx.z];
              if (neighbor.is_standable()) {
                ac |= (1u << n); // set bit if solid
              }
            }

            auto &b0 = m_blocks[block_pos.x][block_pos.y + 1][block_pos.z - 1];
            if (b0.is_standable()) {
              ac |= (1u << 8);
            }
          }
          auto cnt = m_blocks[i][j][k].Render(mask, ac, idx, m_cubeIndicesTrans,
                                              m_cubeVerticesTrans);
          m_cntBlocksTrans += cnt;
          idx += 24 * (cnt != 0), m_countTrans += (cnt != 0);
        }
      }
    }
  }

  // firstRun is called by worker threads (cant make opengl calls from here)
  // !firstRun is called by main threads
  if (!firstRun)
    UpdateVertexObjects();
}

void Chunk::UpdateVertexObjects() {
  // Return early in case of server as we dont want to perform any graphics
  // operations there
#ifdef BUILD_SERVER
  return;
#endif
  m_chunkVb->UpdateBuffer(m_cubeVertices.data(),
                          m_cubeVertices.size() * sizeof(GLuint));

  m_chunkVbTrans->UpdateBuffer(m_cubeVerticesTrans.data(),
                               m_cubeVerticesTrans.size() * sizeof(GLuint));

  m_chunkIb->UpdateBuffer(m_cubeIndices.data(), m_cubeIndices.size());

  m_chunkIbTrans->UpdateBuffer(m_cubeIndicesTrans.data(),
                               m_cubeIndicesTrans.size());
}

void Chunk::SetupVertexObjects() {
  // Return early in case of server as we dont want to perform any graphic
  // operations there
#ifdef BUILD_SERVER
  return;
#endif
  // OPAQUE PASS
  m_chunkVa = std::make_unique<VertexArray>();
  m_chunkVa->Bind();
  VertexBufferLayout layout;
  layout.Push(GL_UNSIGNED_INT, 1);
  m_chunkVb = std::make_unique<VertexBuffer>(m_cubeVertices.capacity() *
                                             sizeof(GLuint));
  m_chunkVa->AddBuffer(*(m_chunkVb), layout);
  m_chunkIb = std::make_unique<IndexBuffer>(m_cubeIndices.data(),
                                            m_cubeIndices.capacity());
  m_chunkIb->Bind();
  m_chunkVa->Unbind();

  // TRANSPARENT PASS
  m_chunkVaTrans = std::make_unique<VertexArray>();
  m_chunkVaTrans->Bind();
  VertexBufferLayout layouttrans;
  layouttrans.Push(GL_UNSIGNED_INT, 1);
  m_chunkVbTrans = std::make_unique<VertexBuffer>(
      m_cubeVerticesTrans.capacity() * sizeof(GLuint));
  m_chunkVaTrans->AddBuffer(*(m_chunkVbTrans), layouttrans);
  m_chunkIbTrans = std::make_unique<IndexBuffer>(m_cubeIndicesTrans.data(),
                                                 m_cubeIndicesTrans.capacity());

  m_chunkIbTrans->Bind();
  m_chunkVaTrans->Unbind();
}

void Chunk::Draw(OBJ_TYPE type, glm::dvec3 cameraPos) {
  if (!m_displayChunk)
    return;
  if (type == OBJ_TYPE::OPAQUE_) {
    m_chunkVa->Bind();
    glUniform3f(chunkpos_uniform, m_chunkPos.x - cameraPos.x,
                m_chunkPos.y - cameraPos.y, m_chunkPos.z - cameraPos.z);
    if (wireframemode) {
      // glUniform4f(vColor_uniform, 0.0, 0.0, 0.0, 1.0);
      glDrawElements(GL_LINES, m_cntBlocks, GL_UNSIGNED_INT, nullptr);
    } else {
      // glUniform4f(vColor_uniform, 0.5, 0.5, 0.5, 1.0);
      // 12 * Total Number of attributes
      glDrawElements(GL_TRIANGLES, m_cntBlocks, GL_UNSIGNED_INT, nullptr);
      // glUniform4f(vColor_uniform, 0.0, 0.0, 0.0, 1.0);
      // glDrawElements(GL_LINES, cntblocks * 12 * 1, GL_UNSIGNED_INT, nullptr);
    }
    m_chunkVa->Unbind();
  } else if (type == OBJ_TYPE::TRANSPARENT_) {
    m_chunkVaTrans->Bind();
    glUniform3f(chunkpos_uniform, m_chunkPos.x - cameraPos.x,
                m_chunkPos.y - cameraPos.y, m_chunkPos.z - cameraPos.z);

    if (wireframemode) {
      // glUniform4f(vColor_uniform, 0.0, 0.0, 0.0, 1.0);
      glDrawElements(GL_LINES, m_cntBlocksTrans, GL_UNSIGNED_INT, nullptr);
    } else {
      // glUniform4f(vColor_uniform, 0.5, 0.5, 0.5, 1.0);
      // 12 * Total Number of attributes
      glDrawElements(GL_TRIANGLES, m_cntBlocksTrans, GL_UNSIGNED_INT, nullptr);
      // glUniform4f(vColor_uniform, 0.0, 0.0, 0.0, 1.0);
      // glDrawElements(GL_LINES, cntblocks * 12 * 1, GL_UNSIGNED_INT, nullptr);
    }
    m_chunkVaTrans->Unbind();
  }
}

void Chunk::Serialize(std::ostream &os) const {
  os.write(reinterpret_cast<const char *>(&m_id), sizeof(m_id));
  os.write(reinterpret_cast<const char *>(&m_saveId), sizeof(m_saveId));
  os.write(reinterpret_cast<const char *>(&m_biomePos.x), sizeof(m_biomePos.x));
  os.write(reinterpret_cast<const char *>(&m_biomePos.y), sizeof(m_biomePos.y));
  os.write(reinterpret_cast<const char *>(&m_biomePos.z), sizeof(m_biomePos.z));
  os.write(reinterpret_cast<const char *>(&m_chunkPos.x), sizeof(m_chunkPos.x));
  os.write(reinterpret_cast<const char *>(&m_chunkPos.y), sizeof(m_chunkPos.y));
  os.write(reinterpret_cast<const char *>(&m_chunkPos.z), sizeof(m_chunkPos.z));
  os.write(reinterpret_cast<const char *>(&m_blocks), sizeof(m_blocks));
}

bool Chunk::Deserialize(std::istream &is) {
  if (!is.read(reinterpret_cast<char *>(&m_id), sizeof(m_id)))
    return false;
  if (!is.read(reinterpret_cast<char *>(&m_saveId), sizeof(m_saveId)))
    return false;
  if (!is.read(reinterpret_cast<char *>(&m_biomePos.x), sizeof(m_biomePos.x)))
    return false;
  if (!is.read(reinterpret_cast<char *>(&m_biomePos.y), sizeof(m_biomePos.y)))
    return false;
  if (!is.read(reinterpret_cast<char *>(&m_biomePos.z), sizeof(m_biomePos.z)))
    return false;
  if (!is.read(reinterpret_cast<char *>(&m_chunkPos.x), sizeof(m_chunkPos.x)))
    return false;
  if (!is.read(reinterpret_cast<char *>(&m_chunkPos.y), sizeof(m_chunkPos.y)))
    return false;
  if (!is.read(reinterpret_cast<char *>(&m_chunkPos.z), sizeof(m_chunkPos.z)))
    return false;
  if (!is.read(reinterpret_cast<char *>(&m_blocks), sizeof(m_blocks)))
    return false;
  return true;
}
