#pragma once
#include <cstdint>

class IndexBuffer {
private:
  unsigned int m_RendererID;
  unsigned int m_Count;
  std::size_t m_capacity = 0;

public:
  IndexBuffer(const unsigned int *data, unsigned int count);
  ~IndexBuffer();

  void Bind() const;
  void Unbind() const;

  inline unsigned int GetCount() const { return m_Count; }
  void UpdateBuffer(const void *, unsigned int);
  void Upload(const void *, std::size_t);
};
