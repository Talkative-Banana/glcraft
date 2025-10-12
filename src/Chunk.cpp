#include "Chunk.h"

#include <cstdlib>
#include <random>
#include <string>

#include "Renderer.h"
#include "World.h"

extern std::unique_ptr<World> world;

Chunk::Chunk() {
}

Chunk::Chunk(uint _id, glm::ivec3 _biomepos, glm::ivec3 position, GLboolean display, int _type) {
  // Check if the chunk needs to be loaded from disk
  id = _id;
  count = 0;
  type = _type;
  biomepos = _biomepos;
  displaychunk = display;
  uint biome_uq_id = BIOME_COUNTZ * _biomepos.x + _biomepos.z;
  save_id = (biome_uq_id << static_cast<int>(log2(CHUNK_COUNTZ * CHUNK_COUNTX))) + _id;
  chunkpos = glm::vec3(
      CHUNK_BLOCK_COUNT * BLOCK_SIZE * (id / CHUNK_COUNTX) + biomepos.x,
      0.0,
      CHUNK_BLOCK_COUNT * BLOCK_SIZE * (id % CHUNK_COUNTZ) + biomepos.z);

  if (world->load_map.find(save_id) != world->load_map.end()) {
    // Empty Chunk
    std::cout << "Setting A chunk with loaded ID: " << save_id << std::endl;
    Chunk &loaded_chunk = world->load_map[save_id];
    blocks = loaded_chunk.blocks;
    dirtybit = true;
  } else {
    Setup_Landscape(
        position.x + (_biomepos.z / (CHUNK_BLOCK_COUNT * BLOCK_SIZE)),
        position.z + (_biomepos.x / (CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
    dirtybit = false;
  }
}

inline GLboolean Chunk::isSolid(const std::vector<GLint> &position) {
  if ((position[0] >= 0) && (position[0] < CHUNK_BLOCK_COUNT) && (position[1] >= 0) &&
      (position[1] < CHUNK_BLOCK_COUNT) && (position[2] >= 0) &&
      (position[2] < CHUNK_BLOCK_COUNT)) {

    // Check if neibhourung block is solid
    return ((blocks[position[0]][position[1]][position[2]].blmask >> 15 & 1));
  }
  return false;
}

// k blue i red j green
// ctrl x -> red facing me

GLuint Chunk::RenderFace(std::vector<GLint> &&position) {
  // 1 -> back face: 2 -> front face: 3 -> left face: 4 -> right face: 5 -> top
  // face: 6 -> bottom face
  GLuint mask = 0;
  std::vector<GLint> tmp = position;
  for (GLuint face = 1; face <= 6; face++) {
    if (face == 1) {
      // No Need to draw back face if block behind is solid
      tmp[2] -= 1;
      if (!isSolid(tmp)) mask |= (1 << (face - 1));
      tmp[2] += 1;
    } else if (face == 2) {
      // No Need to draw front face if block in front is solid
      tmp[2] += 1;
      if (!isSolid(tmp)) mask |= (1 << (face - 1));
      tmp[2] -= 1;
    } else if (face == 3) {
      // No Need to draw left face if block in left is solid
      tmp[0] -= 1;
      if (!isSolid(tmp)) mask |= (1 << (face - 1));
      tmp[0] += 1;
    } else if (face == 4) {
      // No Need to draw right face if block in right is solid
      tmp[0] += 1;
      if (!isSolid(tmp)) mask |= (1 << (face - 1));
      tmp[0] -= 1;
    } else if (face == 5) {
      // No Need to draw top face if block on top is solid
      tmp[1] += 1;
      if (!isSolid(tmp)) mask |= (1 << (face - 1));
      tmp[1] -= 1;
    } else if (face == 6) {
      // No Need to draw bottom face if block on bottom is solid
      tmp[1] -= 1;
      if (!isSolid(tmp)) mask |= (1 << (face - 1));
      tmp[1] += 1;
    }
  }
  return mask;
}

void Chunk::Setup_Landscape(GLint X, GLint Z) {
  // Generate a random seed
  int randomSeed = world->getSeed();

  // 4 5 6 7
  // 0 1 2 3
  noise::module::RidgedMulti mountainTerrain;
  mountainTerrain.SetSeed(randomSeed);  // Set random seed for mountains

  noise::module::Billow baseFlatTerrain;
  baseFlatTerrain.SetFrequency(2.0);
  baseFlatTerrain.SetSeed(randomSeed);  // Set random seed for flat terrain

  noise::module::ScaleBias flatTerrain;
  flatTerrain.SetSourceModule(0, baseFlatTerrain);
  flatTerrain.SetScale(0.085);

  noise::utils::NoiseMap heightMap;
  noise::utils::NoiseMapBuilderPlane heightMapBuilder;
  heightMapBuilder.SetSourceModule(mountainTerrain);
  heightMapBuilder.SetDestNoiseMap(heightMap);
  heightMapBuilder.SetDestSize(128, 128);
  int biomex = X / 4, biomez = Z / 4;
  heightMapBuilder.SetBounds(biomex, biomex + 1, biomez, biomez + 1);
  heightMapBuilder.Build();

  noise::utils::RendererImage renderer;
  noise::utils::Image image;
  renderer.SetSourceNoiseMap(heightMap);
  renderer.SetDestImage(image);
  renderer.Render();

  noise::utils::WriterBMP writer;
  writer.SetSourceImage(image);
  writer.SetDestFilename("maps/tutorial" + std::to_string((4 * X + Z) / 16) + ".bmp");
  X %= 4, Z %= 4;
  writer.WriteDestFile();

  for (int x = 0; x < CHUNK_BLOCK_COUNT; x++) {
    for (int z = 0; z < CHUNK_BLOCK_COUNT; z++) {
      // Use the noise library to get the height value of x, z
      noise::utils::Color color =
          image.GetValue(CHUNK_BLOCK_COUNT * Z + x, CHUNK_BLOCK_COUNT * X + z);
      // Extract the height value from the color's red channel (assuming height
      // is encoded in the red channel)
      // Donot expose BEDROCK
      int height = std::max(2, static_cast<int>((color.blue / 255.0f) * 32.0f));
      // Use the height map texture to get the height value of x, z
      for (int y = 0; y < CHUNK_BLOCK_COUNT; y++) {
        glm::ivec3 ofs = {z, y, x};
        auto biome_bltypes = BIOME_BLOCK_TYPES[type];
        GLuint bltype = -1;
        if (y == 0) {
          bltype = biome_bltypes[4];  // BASE Block BEDROCK
        } else if (y == height - 1) {
          if (y <= 10) {
            bltype = biome_bltypes[2];  // Depth top block DIRT
          } else if (y <= 15) {
            bltype = biome_bltypes[1];  // Depth top middle block GRAVEL
          } else {
            bltype = biome_bltypes[0];  // Top Block GRASS
          }
        } else if (y <= 10) {
          bltype = biome_bltypes[3];  // Depth Block
        } else {
          bltype = biome_bltypes[1];
        }
        blocks[z][y][x] = Block(ofs, y < height, bltype);  // mark them solid
      }
    }
  }
}

void Chunk::Render(
    int setup,
    bool firstRun,
    std::shared_ptr<Chunk> left,
    std::shared_ptr<Chunk> front,
    std::shared_ptr<Chunk> right,
    std::shared_ptr<Chunk> back) {
  // Rerendering
  // if (!displaychunk) return;
  rendervert.clear();
  cube_vertices.clear();
  cube_indices.clear();
  count = 0;
  GLuint idx = 0;

  for (int i = 0; i < CHUNK_BLOCK_COUNT; i++) {
    for (int k = 0; k < CHUNK_BLOCK_COUNT; k++) {
      for (int j = 0; j < CHUNK_BLOCK_COUNT; j++) {
        // filled[0][0][0] = 1;
        if (!((blocks[i][j][k].blmask >> 15) & 1)) {
          // break;  // unsolid blocks
          continue;
        }
        GLuint mask = 0;
        if (firstRun) {
          // If its first run just save the mask
          mask = Chunk::RenderFace({i, j, k});
          blocks[i][j][k].blmask &= ~FACE_MASK;
          blocks[i][j][k].blmask |= (mask << 17);
          // continue;
        } else {
          // Second Run extract the mask
          if (i == 0 && right) {
            auto &blk1 = blocks[0][j][k];
            auto &blk2 = right->blocks[CHUNK_BLOCK_COUNT - 1][j][k];
            if (((blk1.blmask >> 15) & 1) && ((blk2.blmask >> 15) & 1)) {
              // Remove left face from current block
              blk1.blmask &= ~LEFT_FACE;
            }
          } else if (i == CHUNK_BLOCK_COUNT - 1 && left) {
            auto &blk1 = blocks[CHUNK_BLOCK_COUNT - 1][j][k];
            auto &blk2 = left->blocks[0][j][k];
            if (((blk1.blmask >> 15) & 1) && ((blk2.blmask >> 15) & 1)) {
              // Remove right face from current block
              blk1.blmask &= ~RIGHT_FACE;
            }
          }
          if (k == 0 && back) {
            auto &blk1 = blocks[i][j][0];
            auto &blk2 = back->blocks[i][j][CHUNK_BLOCK_COUNT - 1];
            if (((blk1.blmask >> 15) & 1) && ((blk2.blmask >> 15) & 1)) {
              // Remove back face from current block
              blk1.blmask &= ~BACK_FACE;
            }
          } else if (k == CHUNK_BLOCK_COUNT - 1 && front) {
            auto &blk1 = blocks[i][j][CHUNK_BLOCK_COUNT - 1];
            auto &blk2 = front->blocks[i][j][0];
            if (((blk1.blmask >> 15) & 1) && ((blk2.blmask >> 15) & 1)) {
              // Remove front face from current block
              blk1.blmask &= ~FRONT_FACE;
            }
          } else {
            // No update needed middle block
          }
          mask = (blocks[i][j][k].blmask >> 17) & 63;
        }

        glm::ivec3 block_pos = chunkpos +
                               glm::ivec3(BLOCK_SIZE * i, BLOCK_SIZE * j, BLOCK_SIZE * k) +
                               glm::ivec3(HALF_BLOCK_SIZE, HALF_BLOCK_SIZE, HALF_BLOCK_SIZE);

        // Offsets for 8 neighbors around this block (XZ plane)
        static const glm::ivec3 neighborOffsets[8] = {
            {0, BLOCK_SIZE, -BLOCK_SIZE},            // b0
            {-BLOCK_SIZE, BLOCK_SIZE, -BLOCK_SIZE},  // b1   543
            {-BLOCK_SIZE, BLOCK_SIZE, 0},            // b2   6 2
            {-BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE},   // b3   701
            {0, BLOCK_SIZE, BLOCK_SIZE},             // b4
            {BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE},    // b5
            {BLOCK_SIZE, BLOCK_SIZE, 0},             // b6
            {BLOCK_SIZE, BLOCK_SIZE, -BLOCK_SIZE}    // b7
        };

        GLuint ac = 0;
        for (int n = 0; n < 8; n++) {
          auto neighbor = world->get_block_by_center(block_pos + neighborOffsets[n]);
          if (neighbor && neighbor->isSolid()) {
            ac |= (1u << n);  // set bit if solid
          }
        }

        if (auto b0 =
                world->get_block_by_center(block_pos + glm::ivec3(0, BLOCK_SIZE, -BLOCK_SIZE))) {
          if (b0->isSolid()) ac |= (1u << 8);
        }
        std::vector<GLuint> indices;
        std::vector<GLuint> blockrendervert;
        blocks[i][j][k].Render(mask, ac, indices, blockrendervert);
        for (auto &ind : indices) ind += idx;
        rendervert.push_back({blockrendervert, indices});
        idx += 24, count++;
      }
    }
  }


  const GLuint cnt = count;
  const GLuint rsize = static_cast<GLuint>(rendervert.size());

  // std::vector<int> FrustumCull(rsize, 0);
  GLuint vcnt = 0, icnt = 0;

  for (GLuint i = 0; i < rsize; ++i) {
    auto &vert_ind = rendervert[i];
    const auto &verts = vert_ind.first;
    const auto &inds = vert_ind.second;

    vcnt += static_cast<GLuint>(verts.size());
    // Diabled for now //  TODO:
    // if (FrustumCulling(verts[0])) {
    //  FrustumCull[i] = 1;
    //  std::cout << "Frustum Culled!\n";
    //  continue;
    // }
    icnt += static_cast<GLuint>(inds.size());
  }

  cntblocks = icnt;
  cube_vertices.reserve(vcnt);
  cube_indices.reserve(icnt);

  for (GLuint i = 0; i < rsize; ++i) {
    // if (FrustumCull[i]) continue;
    auto &vert_ind = rendervert[i];
    const auto &verts = vert_ind.first;
    const auto &inds = vert_ind.second;

    cube_vertices.insert(cube_vertices.end(), verts.begin(), verts.end());
    cube_indices.insert(cube_indices.end(), inds.begin(), inds.end());
  }

  if (!setup) {
    chunkva->Bind();
    VertexBufferLayout layout;
    layout.Push(GL_UNSIGNED_INT, 1);
    VertexBuffer vb(cube_vertices.data(), cube_vertices.size() * sizeof(GLuint));
    chunkva->AddBuffer(vb, layout);
    IndexBuffer ib(cube_indices.data(), cube_indices.size());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }
}

void Chunk::Draw() {
  if (!displaychunk) return;
  chunkva->Bind();
  glUniform3f(chunkpos_uniform, chunkpos.x, chunkpos.y, chunkpos.z);
  if (wireframemode) {
    // glUniform4f(vColor_uniform, 0.0, 0.0, 0.0, 1.0);
    glDrawElements(GL_LINES, cntblocks * 12 * 1, GL_UNSIGNED_INT, nullptr);
  } else {
    // glUniform4f(vColor_uniform, 0.5, 0.5, 0.5, 1.0);
    // 12 * Total Number of attributes
    glDrawElements(GL_TRIANGLES, cntblocks * 12 * 1, GL_UNSIGNED_INT, nullptr);
    // glUniform4f(vColor_uniform, 0.0, 0.0, 0.0, 1.0);
    // glDrawElements(GL_LINES, cntblocks * 12 * 1, GL_UNSIGNED_INT, nullptr);
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
  if (!is.read(reinterpret_cast<char *>(&id), sizeof(id))) return false;
  if (!is.read(reinterpret_cast<char *>(&save_id), sizeof(save_id))) return false;
  if (!is.read(reinterpret_cast<char *>(&biomepos.x), sizeof(biomepos.x))) return false;
  if (!is.read(reinterpret_cast<char *>(&biomepos.y), sizeof(biomepos.y))) return false;
  if (!is.read(reinterpret_cast<char *>(&biomepos.z), sizeof(biomepos.z))) return false;
  if (!is.read(reinterpret_cast<char *>(&chunkpos.x), sizeof(chunkpos.x))) return false;
  if (!is.read(reinterpret_cast<char *>(&chunkpos.y), sizeof(chunkpos.y))) return false;
  if (!is.read(reinterpret_cast<char *>(&chunkpos.z), sizeof(chunkpos.z))) return false;
  if (!is.read(reinterpret_cast<char *>(&blocks), sizeof(blocks))) return false;
  return true;
}
