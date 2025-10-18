#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/transform.hpp>

#include "Constants.hpp"

class Camera {
 private:
  glm::mat4 m_ProjectionMatrix;
  glm::mat4 m_ViewMatrix;
  glm::mat4 m_ViewProjectionMatrix;

  glm::vec3 m_Up = glm::vec3(0.0, 1.0, 0.0);
  glm::vec3 m_Position = glm::vec3(0.0, 0.0, -80.0);
  glm::vec3 m_Orientation = glm::vec3(0.0, 0.0, 1.0);
  float m_VerticalFOV = 45.0f;
  float m_AspectRatio = 1.0f;

  void RecalculateViewMatrix();

 public:
  Camera(float left, float right, float bottom, float top);

  const glm::vec3& GetPosition() const {
    return m_Position;
  }

  void SetPosition(const glm::vec3& position) {
    m_Position = position;
    RecalculateViewMatrix();
  }

  const glm::vec3& GetOrientation() const {
    return m_Orientation;
  }

  void SetOrientation(const glm::vec3& orientation) {
    m_Orientation = orientation;
    glm::normalize(m_Orientation);
    RecalculateViewMatrix();
  }

  const glm::vec3 GetUp() const {
    return m_Up;
  }

  void SetUp(glm::vec3 Up) {
    m_Up = Up;
    RecalculateViewMatrix();
  }

  const glm::mat4& GetProjectionMatrix() const {
    return m_ProjectionMatrix;
  }

  const glm::mat4& GetViewMatrix() const {
    return m_ViewMatrix;
  }

  const glm::mat4& GetProjectionViewMatrix() const {
    return m_ViewProjectionMatrix;
  }

  void SetAspectRatio(float aspectratio);

  float GetHorizontalFOV() const;
};
