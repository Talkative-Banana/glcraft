#include "Camera.h"

Camera::Camera(double left, double right, double bottom, double top)
    : m_ViewMatrix(1.0), m_AspectRatio(1.0) {
  // Perspective Addition
  m_ProjectionMatrix = glm::perspective(glm::radians(double(m_VerticalFOV)),
                                        1.0, NEAR_PLANE, FAR_PLANE);
  m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;

  m_ViewProjectionRenderMatrix = m_ProjectionMatrix * m_ViewRotateMatrix;
}

void Camera::RecalculateViewMatrix() {

  m_ViewMatrix =
      glm::lookAt(m_Position, m_Position + glm::normalize(m_Orientation), m_Up);
  m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void Camera::RecalculateViewRenderMatrix() {
  glm::dmat4 view =
      glm::lookAt(m_Position, m_Position + glm::normalize(m_Orientation), m_Up);

  // Remove translation (keep rotation only)
  m_ViewRotateMatrix = glm::dmat4(glm::dmat3(view));

  m_ViewProjectionRenderMatrix = m_ProjectionMatrix * m_ViewRotateMatrix;
}

void Camera::SetAspectRatio(double aspectratio) {
  m_AspectRatio = aspectratio;
  m_ProjectionMatrix =
      glm::perspective(glm::radians(double(m_VerticalFOV)), double(aspectratio),
                       NEAR_PLANE, FAR_PLANE);
}

// Compute horizontal FOV based on aspect ratio
double Camera::GetHorizontalFOV() const {
  double vFovRad = glm::radians(m_VerticalFOV);
  double hFovRad = 2.0 * atan(tan(vFovRad / 2.0) * m_AspectRatio);
  return glm::degrees(hFovRad);
}
