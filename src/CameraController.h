#pragma once
#include "Camera.h"
#include "Input.h"
#include "Main.h"

class CameraController {
private:
  double m_AspectRatio;
  Camera *m_Camera = nullptr;

public:
  void UpdateCamera(glm::dvec3 pos, glm::dvec3 dir);
  Camera *GetCamera();
  CameraController(double aspectratio);
  ~CameraController();
  void SetAspectRatio(double aspectratio);
};
