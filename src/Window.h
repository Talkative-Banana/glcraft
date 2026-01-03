#pragma once
#include "Utils.h"

class Window {
private:
  GLFWwindow *window;
  int screen_height, screen_width;

public:
  Window(int, int);
  GLFWwindow *GetWindow() const;
  void setup(int, int);
  const int GetWidth() const;
  const int GetHeight() const;
  void SetWidth(int);
  void SetHeight(int);
};
