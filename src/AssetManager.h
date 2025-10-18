#include <chrono>
#include <memory>
#include <unordered_map>

#include "AssetVertex.h"
#include "Mesh.h"
#include "Utils.h"
#include "string.h"

class AssetManager {
 public:
  AssetManager() = default;
  [[nodiscard]] uint64_t loadMeshObject(
      const char* modelpath,
      unsigned int shaderProgram,
      float scale,
      float angle,
      glm::vec3 pos,
      glm::vec3 axis);

  std::shared_ptr<Mesh> get_mesh(uint64_t handle) {
    auto it = m_assets.find(handle);
    if (it == m_assets.end()) return nullptr;
    return it->second;
  }

 private:
  static AssetManager* s_assetmanager;
  FILE* LoadModel(const char* path);
  std::unordered_map<uint64_t, std::shared_ptr<Mesh>> m_assets;
};
