#include "Ray.h"

glm::vec3 toBlockCenter(const glm::vec3 &p) {
  return glm::floor(p / BLOCK_SIZE) * BLOCK_SIZE + glm::vec3(HALF_BLOCK_SIZE);
}

bool Ray::did_hit(std::unique_ptr<World> &world) {
  const double maxDistance = TOTAL_STEPS * STEP_SIZE;

  glm::dvec3 rayOrigin = m_pos;
  glm::dvec3 rayDir = glm::normalize(m_dir);

  // voxel index (integer grid)
  glm::ivec3 voxel(int(std::floor(rayOrigin.x / BLOCK_SIZE)),
                   int(std::floor(rayOrigin.y / BLOCK_SIZE)),
                   int(std::floor(rayOrigin.z / BLOCK_SIZE)));

  glm::ivec3 step;
  glm::dvec3 tMax;
  glm::dvec3 tDelta;

  for (int axis = 0; axis < 3; ++axis) {
    if (rayDir[axis] > 0.0) {
      step[axis] = 1;

      double nextBoundary = (double(voxel[axis] + 1) * BLOCK_SIZE);

      tMax[axis] = (nextBoundary - rayOrigin[axis]) / rayDir[axis];

      tDelta[axis] = BLOCK_SIZE / rayDir[axis];
    } else if (rayDir[axis] < 0.0) {
      step[axis] = -1;

      double nextBoundary = (double(voxel[axis]) * BLOCK_SIZE);

      tMax[axis] = (nextBoundary - rayOrigin[axis]) / rayDir[axis];

      tDelta[axis] = -BLOCK_SIZE / rayDir[axis];
    } else {
      step[axis] = 0;
      tMax[axis] = std::numeric_limits<double>::infinity();
      tDelta[axis] = std::numeric_limits<double>::infinity();
    }
  }

  double traveled = 0.0;

  while (traveled <= maxDistance) {
    glm::ivec3 blockCenter =
        voxel * static_cast<int>(BLOCK_SIZE) + glm::ivec3(HALF_BLOCK_SIZE);

    if (world->isSolid(blockCenter)) {
      m_hitcords = blockCenter;

      glm::ivec3 normal(0);
      if (tMax.x < tMax.y && tMax.x < tMax.z)
        normal.x = -step.x;
      else if (tMax.y < tMax.z)
        normal.y = -step.y;
      else
        normal.z = -step.z;

      m_hitnormal = normal;
      return m_hit = true;
    } else {
      m_hitcordsprev = blockCenter;
    }

    if (tMax.x < tMax.y) {
      if (tMax.x < tMax.z) {
        voxel.x += step.x;
        traveled = tMax.x;
        tMax.x += tDelta.x;
      } else {
        voxel.z += step.z;
        traveled = tMax.z;
        tMax.z += tDelta.z;
      }
    } else {
      if (tMax.y < tMax.z) {
        voxel.y += step.y;
        traveled = tMax.y;
        tMax.y += tDelta.y;
      } else {
        voxel.z += step.z;
        traveled = tMax.z;
        tMax.z += tDelta.z;
      }
    }
  }
  return m_hit = false;
}

Ray screenPosToWorldRay(GLFWwindow *window, double mouseX, double mouseY,
                        const glm::dmat4 &view, const glm::dmat4 &projection) {
  int width, height;
  glfwGetWindowSize(window, &width, &height);

  glm::ivec4 viewport(0, 0, width, height);

  glm::dvec3 screenNear(mouseX, height - mouseY, 0.0);
  glm::dvec3 screenFar(mouseX, height - mouseY, 1.0);

  glm::dvec3 worldNear = glm::unProject(screenNear, view, projection, viewport);

  glm::dvec3 worldFar = glm::unProject(screenFar, view, projection, viewport);

  return Ray(worldNear, glm::normalize(worldFar - worldNear));
}
