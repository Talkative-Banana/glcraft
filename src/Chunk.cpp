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
  id = _id;
  count = 0;
  counttrans = 0;
  type = _type;
  biomepos = _biomepos;
  displaychunk = display;
  int i = abs(_biomepos.x / (BIOME_LENGTH));
  int j = abs(_biomepos.z / (BIOME_LENGTH));
  int k = abs(_biomepos.y) / (BIOME_HEIGHT);
  uint biome_uq_id = BIOME_COUNTZ * BIOME_COUNTX * k + BIOME_COUNTZ * j + i;
  save_id =
      (biome_uq_id << static_cast<int>(log2(CHUNK_COUNTZ * CHUNK_COUNTX))) +
      _id;
  chunkpos =
      glm::ivec3(CHUNK_LENGTH * (id / CHUNK_COUNTX) + biomepos.x, biomepos.y,
                 CHUNK_LENGTH * (id % CHUNK_COUNTZ) + biomepos.z);

  if (world->load_map.find(save_id) != world->load_map.end()) {
    // Empty Chunk
    std::cout << "Setting A chunk with loaded ID: " << save_id << '\n';
    Chunk &loaded_chunk = world->load_map[save_id];
    blocks = loaded_chunk.blocks;
    dirtybit = true;
  } else {
    Setup_Landscape(position.x + (_biomepos.z / (CHUNK_LENGTH)),
                    position.z + (_biomepos.x / (CHUNK_LENGTH)));
    dirtybit = false;
  }

  // Reserve for vectors
  cube_indices.reserve(50'000);
  cube_indicestrans.reserve(50'000);

  cube_vertices.reserve(8'00'000);
  cube_verticestrans.reserve(8'00'000);
}

inline GLboolean Chunk::isSolid(int x, int y, int z) {
  if ((x >= 0) && (x < CHUNK_BLOCK_COUNT) && (y >= 0) &&
      (y < CHUNK_BLOCK_COUNT) && (z >= 0) && (z < CHUNK_BLOCK_COUNT)) {

    // Check if neibhourung block is solid
    return ((blocks[x][y][z].is_solid()));
  }
  return false;
}

inline GLboolean Chunk::isTransparent(int x, int y, int z) {
  if ((x >= 0) && (x < CHUNK_BLOCK_COUNT) && (y >= 0) &&
      (y < CHUNK_BLOCK_COUNT) && (z >= 0) && (z < CHUNK_BLOCK_COUNT)) {

    // Check if neibhourung block is transparent
    return ((blocks[x][y][z].is_transparent()));
  }
  return false;
}

inline GLboolean Chunk::isSameKind(int x, int y, int z, int X, int Y, int Z) {
  if ((x >= 0) && (x < CHUNK_BLOCK_COUNT) && (y >= 0) &&
      (y < CHUNK_BLOCK_COUNT) && (z >= 0) && (z < CHUNK_BLOCK_COUNT)) {

    // Check if neibhourung blocks are of same kind
    bool res = (blocks[x][y][z].get_type() == blocks[X][Y][Z].get_type());
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
  if (chunkpos.y != ((BIOME_COUNTY - 1) * CHUNK_HEIGHT)) {
    for (int x = 0; x < CHUNK_BLOCK_COUNT; x++) {
      for (int z = 0; z < CHUNK_BLOCK_COUNT; z++) {
        for (int y = 0; y < CHUNK_BLOCK_COUNT; y++) {
          glm::ivec3 ofs = {z, y, x};
          auto &biome_bltypes = BIOME_BLOCK_TYPES[type];
          BLOCK_TYPE &bltype = biome_bltypes[3];
          blocks[z][y][x] = Block(std::move(ofs), true, bltype);
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
        auto biome_bltypes = BIOME_BLOCK_TYPES[type];
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
        blocks[z][y][x] = Block(ofs, (y < height || y <= 15), bltype);
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
      return &this->blocks[newPos.x][newPos.y][newPos.z];
    }

    bool validX = newPos.x != -1 && newPos.x != CHUNK_BLOCK_COUNT;
    bool validY = newPos.y != -1 && newPos.y != CHUNK_BLOCK_COUNT;
    bool validZ = newPos.z != -1 && newPos.z != CHUNK_BLOCK_COUNT;

    if (!validX && validY && validZ) {
      if (newPos.x == -1 && right) {
        return &right->blocks[CHUNK_BLOCK_COUNT - 1][newPos.y][newPos.z];
      }
      if (newPos.x == CHUNK_BLOCK_COUNT && left) {
        return &left->blocks[0][newPos.y][newPos.z];
      }
    }

    if (!validZ && validX && validY) {
      if (newPos.z == -1 && back) {
        return &back->blocks[newPos.x][newPos.y][CHUNK_BLOCK_COUNT - 1];
      }
      if (newPos.z == CHUNK_BLOCK_COUNT && front) {
        return &front->blocks[newPos.x][newPos.y][0];
      }
    }

    // TODO: Remove it if not needed [just here for optimisation]
    if (!validY && validX && validZ) {
      return nullptr;
    }

    glm::ivec3 block_pos =
        chunkpos + glm::ivec3(BLOCK_SIZE * i, BLOCK_SIZE * j, BLOCK_SIZE * k) +
        glm::ivec3(HALF_BLOCK_SIZE, HALF_BLOCK_SIZE, HALF_BLOCK_SIZE) +
        offset * static_cast<int>(BLOCK_SIZE);
    return world->get_block_by_center(blockpos);
  };

  {
    cube_vertices.clear();
    cube_indices.clear();
    count = 0, cntblocks = 0;
    GLuint idx = 0;

    for (int i = 0; i < CHUNK_BLOCK_COUNT; i++) {
      for (int k = 0; k < CHUNK_BLOCK_COUNT; k++) {
        for (int j = 0; j < CHUNK_BLOCK_COUNT; j++) {
          // filled[0][0][0] = 1;
          if (!blocks[i][j][k].is_solid() || blocks[i][j][k].is_transparent()) {
            // break;  // unsolid blocks
            continue;
          }
          GLuint mask = 0;
          if (firstRun) {
            // If its first run just save the mask
            mask = Chunk::RenderFace(i, j, k);
            blocks[i][j][k].blmask &= ~FACE_MASK;
            blocks[i][j][k].blmask |= (mask << 17);
            // continue;
          } else {
            // Second Run extract the mask
            if (i == 0 && right) {
              auto &blk1 = blocks[0][j][k];
              auto &blk2 = right->blocks[CHUNK_BLOCK_COUNT - 1][j][k];
              if ((blk1.is_solid()) &&
                  (blk2.is_solid() && !blk2.is_transparent())) {
                // Remove left face from current block
                blk1.blmask &= ~LEFT_FACE;
              }
            } else if (i == CHUNK_BLOCK_COUNT - 1 && left) {
              auto &blk1 = blocks[CHUNK_BLOCK_COUNT - 1][j][k];
              auto &blk2 = left->blocks[0][j][k];
              if ((blk1.is_solid()) &&
                  (blk2.is_solid() && !blk2.is_transparent())) {
                // Remove right face from current block
                blk1.blmask &= ~RIGHT_FACE;
              }
            }
            if (k == 0 && back) {
              auto &blk1 = blocks[i][j][0];
              auto &blk2 = back->blocks[i][j][CHUNK_BLOCK_COUNT - 1];
              if ((blk1.is_solid()) &&
                  (blk2.is_solid() && !blk2.is_transparent())) {
                // Remove back face from current block
                blk1.blmask &= ~BACK_FACE;
              }
            } else if (k == CHUNK_BLOCK_COUNT - 1 && front) {
              auto &blk1 = blocks[i][j][CHUNK_BLOCK_COUNT - 1];
              auto &blk2 = front->blocks[i][j][0];
              if ((blk1.is_solid()) &&
                  (blk2.is_solid() && !blk2.is_transparent())) {
                // Remove front face from current block
                blk1.blmask &= ~FRONT_FACE;
              }
            }
            mask = (blocks[i][j][k].blmask >> 17) & 63;
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
              auto &neighbor = blocks[Idx.x][Idx.y][Idx.z];
              if (neighbor.is_standable()) {
                ac |= (1u << n); // set bit if solid
              }
            }

            auto &b0 = blocks[block_pos.x][block_pos.y + 1][block_pos.z - 1];
            if (b0.is_standable()) {
              ac |= (1u << 8);
            }
          }
          cntblocks += blocks[i][j][k].Render(mask, ac, idx, cube_indices,
                                              cube_vertices);
          idx += 24, count++;
        }
      }
    }
  }

  // Render transparent blocks
  {
    cube_verticestrans.clear();
    cube_indicestrans.clear();
    counttrans = 0, cntblockstrans = 0;
    GLuint idx = 0;

    for (int i = 0; i < CHUNK_BLOCK_COUNT; i++) {
      for (int k = 0; k < CHUNK_BLOCK_COUNT; k++) {
        for (int j = 0; j < CHUNK_BLOCK_COUNT; j++) {
          // filled[0][0][0] = 1;
          if (!blocks[i][j][k].is_solid() ||
              !blocks[i][j][k].is_transparent()) {
            // break;  // unsolid blocks
            continue;
          }
          GLuint mask = 0;
          if (firstRun) {
            // If its first run just save the mask
            mask = Chunk::RenderFace(i, j, k);
            blocks[i][j][k].blmask &= ~FACE_MASK;
            blocks[i][j][k].blmask |= (mask << 17);
            // continue;
          } else {
            // Second Run extract the mask
            if (i == 0 && right) {
              auto &blk1 = blocks[0][j][k];
              auto &blk2 = right->blocks[CHUNK_BLOCK_COUNT - 1][j][k];
              if ((blk1.is_solid()) && (blk2.is_solid())) {
                // Remove left face from current block
                blk1.blmask &= ~LEFT_FACE;
              }
            } else if (i == CHUNK_BLOCK_COUNT - 1 && left) {
              auto &blk1 = blocks[CHUNK_BLOCK_COUNT - 1][j][k];
              auto &blk2 = left->blocks[0][j][k];
              if ((blk1.is_solid()) && (blk2.is_solid())) {
                // Remove right face from current block
                blk1.blmask &= ~RIGHT_FACE;
              }
            }
            if (k == 0 && back) {
              auto &blk1 = blocks[i][j][0];
              auto &blk2 = back->blocks[i][j][CHUNK_BLOCK_COUNT - 1];
              if ((blk1.is_solid()) && (blk2.is_solid())) {
                // Remove back face from current block
                blk1.blmask &= ~BACK_FACE;
              }
            } else if (k == CHUNK_BLOCK_COUNT - 1 && front) {
              auto &blk1 = blocks[i][j][CHUNK_BLOCK_COUNT - 1];
              auto &blk2 = front->blocks[i][j][0];
              if ((blk1.is_solid()) && (blk2.is_solid())) {
                // Remove front face from current block
                blk1.blmask &= ~FRONT_FACE;
              }
            } else {
              // No update needed middle block
            }
            mask = (blocks[i][j][k].blmask >> 17) & 63;
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
              auto &neighbor = blocks[Idx.x][Idx.y][Idx.z];
              if (neighbor.is_standable()) {
                ac |= (1u << n); // set bit if solid
              }
            }

            auto &b0 = blocks[block_pos.x][block_pos.y + 1][block_pos.z - 1];
            if (b0.is_standable()) {
              ac |= (1u << 8);
            }
          }
          cntblockstrans += blocks[i][j][k].Render(
              mask, ac, idx, cube_indicestrans, cube_verticestrans);
          idx += 24, counttrans++;
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
  chunkvb->UpdateBuffer(cube_vertices.data(),
                        cube_vertices.size() * sizeof(GLuint));

  chunkvbtrans->UpdateBuffer(cube_verticestrans.data(),
                             cube_verticestrans.size() * sizeof(GLuint));

  chunkib->UpdateBuffer(cube_indices.data(), cube_indices.size());

  chunkibtrans->UpdateBuffer(cube_indicestrans.data(),
                             cube_indicestrans.size());
}

void Chunk::SetupVertexObjects() {
  // Return early in case of server as we dont want to perform any graphic
  // operations there
#ifdef BUILD_SERVER
  return;
#endif
  // OPAQUE PASS
  chunkva = std::make_unique<VertexArray>();
  chunkva->Bind();
  VertexBufferLayout layout;
  layout.Push(GL_UNSIGNED_INT, 1);
  chunkvb =
      std::make_unique<VertexBuffer>(cube_vertices.capacity() * sizeof(GLuint));
  chunkva->AddBuffer(*(chunkvb), layout);
  chunkib = std::make_unique<IndexBuffer>(cube_indices.data(),
                                          cube_indices.capacity());
  chunkib->Bind();
  chunkva->Unbind();

  // TRANSPARENT PASS
  chunkvatrans = std::make_unique<VertexArray>();
  chunkvatrans->Bind();
  VertexBufferLayout layouttrans;
  layouttrans.Push(GL_UNSIGNED_INT, 1);
  chunkvbtrans = std::make_unique<VertexBuffer>(cube_verticestrans.capacity() *
                                                sizeof(GLuint));
  chunkvatrans->AddBuffer(*(chunkvbtrans), layouttrans);
  chunkibtrans = std::make_unique<IndexBuffer>(cube_indicestrans.data(),
                                               cube_indicestrans.capacity());

  chunkibtrans->Bind();
  chunkvatrans->Unbind();
}

void Chunk::Draw(OBJ_TYPE type, glm::dvec3 cameraPos) {
  if (!displaychunk)
    return;
  if (type == OBJ_TYPE::OPAQUE_) {
    chunkva->Bind();
    glUniform3f(chunkpos_uniform, chunkpos.x - cameraPos.x,
                chunkpos.y - cameraPos.y, chunkpos.z - cameraPos.z);
    if (wireframemode) {
      // glUniform4f(vColor_uniform, 0.0, 0.0, 0.0, 1.0);
      glDrawElements(GL_LINES, cntblocks, GL_UNSIGNED_INT, nullptr);
    } else {
      // glUniform4f(vColor_uniform, 0.5, 0.5, 0.5, 1.0);
      // 12 * Total Number of attributes
      glDrawElements(GL_TRIANGLES, cntblocks, GL_UNSIGNED_INT, nullptr);
      // glUniform4f(vColor_uniform, 0.0, 0.0, 0.0, 1.0);
      // glDrawElements(GL_LINES, cntblocks * 12 * 1, GL_UNSIGNED_INT, nullptr);
    }
    chunkva->Unbind();
  } else if (type == OBJ_TYPE::TRANSPARENT_) {
    chunkvatrans->Bind();
    glUniform3f(chunkpos_uniform, chunkpos.x - cameraPos.x,
                chunkpos.y - cameraPos.y, chunkpos.z - cameraPos.z);

    if (wireframemode) {
      // glUniform4f(vColor_uniform, 0.0, 0.0, 0.0, 1.0);
      glDrawElements(GL_LINES, cntblockstrans, GL_UNSIGNED_INT, nullptr);
    } else {
      // glUniform4f(vColor_uniform, 0.5, 0.5, 0.5, 1.0);
      // 12 * Total Number of attributes
      glDrawElements(GL_TRIANGLES, cntblockstrans, GL_UNSIGNED_INT, nullptr);
      // glUniform4f(vColor_uniform, 0.0, 0.0, 0.0, 1.0);
      // glDrawElements(GL_LINES, cntblocks * 12 * 1, GL_UNSIGNED_INT, nullptr);
    }
    chunkvatrans->Unbind();
  }
}

void Chunk::Serialize(std::ostream &os) const {
  os.write(reinterpret_cast<const char *>(&id), sizeof(id));
  os.write(reinterpret_cast<const char *>(&save_id), sizeof(save_id));
  os.write(reinterpret_cast<const char *>(&biomepos.x), sizeof(biomepos.x));
  os.write(reinterpret_cast<const char *>(&biomepos.y), sizeof(biomepos.y));
  os.write(reinterpret_cast<const char *>(&biomepos.z), sizeof(biomepos.z));
  os.write(reinterpret_cast<const char *>(&chunkpos.x), sizeof(chunkpos.x));
  os.write(reinterpret_cast<const char *>(&chunkpos.y), sizeof(chunkpos.y));
  os.write(reinterpret_cast<const char *>(&chunkpos.z), sizeof(chunkpos.z));
  os.write(reinterpret_cast<const char *>(&blocks), sizeof(blocks));
}

bool Chunk::Deserialize(std::istream &is) {
  if (!is.read(reinterpret_cast<char *>(&id), sizeof(id)))
    return false;
  if (!is.read(reinterpret_cast<char *>(&save_id), sizeof(save_id)))
    return false;
  if (!is.read(reinterpret_cast<char *>(&biomepos.x), sizeof(biomepos.x)))
    return false;
  if (!is.read(reinterpret_cast<char *>(&biomepos.y), sizeof(biomepos.y)))
    return false;
  if (!is.read(reinterpret_cast<char *>(&biomepos.z), sizeof(biomepos.z)))
    return false;
  if (!is.read(reinterpret_cast<char *>(&chunkpos.x), sizeof(chunkpos.x)))
    return false;
  if (!is.read(reinterpret_cast<char *>(&chunkpos.y), sizeof(chunkpos.y)))
    return false;
  if (!is.read(reinterpret_cast<char *>(&chunkpos.z), sizeof(chunkpos.z)))
    return false;
  if (!is.read(reinterpret_cast<char *>(&blocks), sizeof(blocks)))
    return false;
  return true;
}
