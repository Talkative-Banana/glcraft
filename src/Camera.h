#pragma once

#include <glm/gtx/transform.hpp>

#include "Constants.hpp"

class Camera {
private:
  glm::mat4 m_ProjectionMatrix;
  glm::mat4 m_ViewMatrix, m_ViewRotateMatrix;
  glm::mat4 m_ViewProjectionMatrix, m_ViewProjectionRenderMatrix;

  glm::vec3 m_Up = glm::vec3(0.0, 1.0, 0.0);
  glm::vec3 m_Position = glm::vec3(0.0, 0.0, -80.0);
  glm::vec3 m_Orientation = glm::vec3(0.0, 0.0, 1.0);
  float m_VerticalFOV = 45.0f;
  float m_AspectRatio = 1.0f;

  void RecalculateViewMatrix();

  void RecalculateViewRenderMatrix();

public:
  Camera(float left, float right, float bottom, float top);

  const glm::vec3 &GetPosition() const { return m_Position; }

  void SetPosition(const glm::vec3 &position) {
    m_Position = position;
    RecalculateViewMatrix();
    RecalculateViewRenderMatrix();
  }

  const glm::vec3 &GetOrientation() const { return m_Orientation; }

  void SetOrientation(const glm::vec3 &orientation) {
    m_Orientation = glm::normalize(orientation);
    RecalculateViewMatrix();
    RecalculateViewRenderMatrix();
  }

  const glm::vec3 GetUp() const { return m_Up; }

  void SetUp(glm::vec3 Up) {
    m_Up = Up;
    RecalculateViewMatrix();
    RecalculateViewRenderMatrix();
  }

  const glm::mat4 &GetProjectionMatrix() const { return m_ProjectionMatrix; }

  const glm::mat4 &GetViewMatrix() const { return m_ViewMatrix; }

  const glm::mat4 &GetProjectionViewMatrix() const {
    return m_ViewProjectionMatrix;
  }

  const glm::mat4 &GetViewRenderMatrix() const { return m_ViewRotateMatrix; }

  const glm::mat4 &GetProjectionViewRenderMatrix() const {
    return m_ViewProjectionRenderMatrix;
  }

  void SetAspectRatio(float aspectratio);

  float GetHorizontalFOV() const;
};
