#pragma once

class VertexBuffer {
 private:
  unsigned int m_RendererID;

 public:
  VertexBuffer(const void*, unsigned int);  // STATIC BUFFER
  VertexBuffer(unsigned int);               // DYNAMIC BUFFER
  ~VertexBuffer();

  void Bind() const;
  void Unbind() const;
};
