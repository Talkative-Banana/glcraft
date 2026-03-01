#pragma once

#include <glm/gtx/transform.hpp>

#include "Constants.hpp"

class Camera {
private:
  glm::dmat4 m_ProjectionMatrix;
  glm::dmat4 m_ViewMatrix, m_ViewRotateMatrix;
  glm::dmat4 m_ViewProjectionMatrix, m_ViewProjectionRenderMatrix;

  glm::dvec3 m_Up = glm::dvec3(0.0, 1.0, 0.0);
  glm::dvec3 m_Position = glm::dvec3(0.0, 0.0, -80.0);
  glm::dvec3 m_Orientation = glm::dvec3(0.0, 0.0, 1.0);
  double m_VerticalFOV = 45.0f;
  double m_AspectRatio = 1.0f;

  void RecalculateViewMatrix();

  void RecalculateViewRenderMatrix();

public:
  Camera(double left, double right, double bottom, double top);

  const glm::dvec3 &GetPosition() const { return m_Position; }

  void SetPosition(const glm::dvec3 &position) {
    m_Position = position;
    RecalculateViewMatrix();
    RecalculateViewRenderMatrix();
  }

  const glm::dvec3 &GetOrientation() const { return m_Orientation; }

  void SetOrientation(const glm::dvec3 &orientation) {
    m_Orientation = glm::normalize(orientation);
    RecalculateViewMatrix();
    RecalculateViewRenderMatrix();
  }

  const glm::dvec3 GetUp() const { return m_Up; }

  void SetUp(glm::dvec3 Up) {
    m_Up = Up;
    RecalculateViewMatrix();
    RecalculateViewRenderMatrix();
  }

  const glm::dmat4 &GetProjectionMatrix() const { return m_ProjectionMatrix; }

  const glm::dmat4 &GetViewMatrix() const { return m_ViewMatrix; }

  const glm::dmat4 &GetProjectionViewMatrix() const {
    return m_ViewProjectionMatrix;
  }

  const glm::dmat4 &GetViewRenderMatrix() const { return m_ViewRotateMatrix; }

  const glm::dmat4 &GetProjectionViewRenderMatrix() const {
    return m_ViewProjectionRenderMatrix;
  }

  void SetAspectRatio(double aspectratio);

  double GetHorizontalFOV() const;
};
