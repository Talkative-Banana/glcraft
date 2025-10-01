#include "Camera.h"

Camera::Camera(float left, float right, float bottom, float top)
    : m_ViewMatrix(1.0f), m_AspectRatio(1.0f) {
  // Perspective Addition
  m_ProjectionMatrix = glm::perspective(glm::radians(m_VerticalFOV), 1.0f, NEAR_PLANE, FAR_PLANE);
  m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void Camera::RecalculateViewMatrix() {

  m_ViewMatrix = glm::lookAt(m_Position, m_Position + glm::normalize(m_Orientation), m_Up);
  m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void Camera::SetAspectRatio(float aspectratio) {
  m_AspectRatio = aspectratio;
  m_ProjectionMatrix =
      glm::perspective(glm::radians(m_VerticalFOV), aspectratio, NEAR_PLANE, FAR_PLANE);
}

// Compute horizontal FOV based on aspect ratio
float Camera::GetHorizontalFOV() const {
  float vFovRad = glm::radians(m_VerticalFOV);
  float hFovRad = 2.0f * atan(tan(vFovRad / 2.0f) * m_AspectRatio);
  return glm::degrees(hFovRad);
}
