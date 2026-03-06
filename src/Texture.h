#pragma once

#include "Renderer.h"
#include <string>
class Texture {
private:
  unsigned int m_RendererID;
  std::string m_FilePath;
  unsigned char *m_LocalBuffer;
  int m_Width, m_Height, m_BPP;

public:
  // type 0 normal
  // type 1 cubemap
  Texture(const std::string &path);
  ~Texture();

  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;

  Texture(Texture &&other) noexcept {
    m_RendererID = other.m_RendererID;
    m_LocalBuffer = other.m_LocalBuffer;
    m_Width = other.m_Width;
    m_Height = other.m_Height;
    m_BPP = other.m_BPP;

    other.m_RendererID = 0;
  }

  Texture &operator=(Texture &&other) noexcept {
    if (this != &other) {
      glDeleteTextures(1, &m_RendererID);

      m_RendererID = other.m_RendererID;
      m_LocalBuffer = other.m_LocalBuffer;
      m_Width = other.m_Width;
      m_Height = other.m_Height;
      m_BPP = other.m_BPP;

      other.m_RendererID = 0;
    }
    return *this;
  }

  void Bind(unsigned int slot = 0) const;
  void Unbind() const;

  inline int GetWidth() const { return m_Width; }
  inline int GetHeight() const { return m_Height; }
};
