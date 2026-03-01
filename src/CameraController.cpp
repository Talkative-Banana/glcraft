#include "CameraController.h"

CameraController::CameraController(double aspectratio) {
  m_AspectRatio = aspectratio;
  m_Camera = new Camera(-1.0, 1.0, -1.0, 1.0);
}

CameraController::~CameraController() {
  delete m_Camera;
  m_Camera = nullptr;
}

void CameraController::SetAspectRatio(double aspectratio) {
  m_AspectRatio = aspectratio;
  m_Camera->SetAspectRatio(m_AspectRatio);
}

Camera *CameraController::GetCamera() { return m_Camera; }

void CameraController::UpdateCamera(glm::dvec3 pos, glm::dvec3 dir) {
  m_Camera->SetPosition(pos);
  m_Camera->SetOrientation(dir);
}
