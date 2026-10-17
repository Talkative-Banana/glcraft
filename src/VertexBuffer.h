#pragma once
#include <cstdint>

class VertexBuffer {
private:
  unsigned int m_RendererID;
  std::size_t m_capacity = 0;

public:
  VertexBuffer(const void *, unsigned int); // STATIC BUFFER
  VertexBuffer(unsigned int);               // DYNAMIC BUFFER
  ~VertexBuffer();

  void Bind() const;
  void Unbind() const;
  void UpdateBuffer(const void *, unsigned int) const;
  void Upload(const void *, std::size_t);
};
