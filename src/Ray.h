#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <memory>

#include "Constants.hpp"
#include "Utils.h"
#include "World.h"

class Ray {
 public:
  glm::vec3 m_pos;
  glm::vec3 m_dir;
  glm::ivec3 m_hitcords;
  glm::ivec3 m_hitnormal;
  bool m_hit;

  Ray(const glm::vec3 &pos, const glm::vec3 &dir) : m_pos(pos), m_dir(dir), m_hit(false) {};
  bool did_hit(std::unique_ptr<World> &world);
};

Ray screenPosToWorldRay(
    GLFWwindow *window,
    double mouseX,
    double mouseY,
    const glm::mat4 &view,
    const glm::mat4 &projection);
