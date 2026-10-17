#include "VertexBuffer.h"

#include "Renderer.h"
#include <iostream>

VertexBuffer::VertexBuffer(const void *data, unsigned int size) {
  GLCall(glGenBuffers(1, &m_RendererID));
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
  GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::VertexBuffer(unsigned int size) {
  GLCall(glGenBuffers(1, &m_RendererID));
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
  GLCall(glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW));
}

VertexBuffer::~VertexBuffer() { GLCall(glDeleteBuffers(1, &m_RendererID)); }

void VertexBuffer::Bind() const {
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
}

void VertexBuffer::Unbind() const { GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0)); }

void VertexBuffer::UpdateBuffer(const void *data, unsigned int size) const {
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
  GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, size, data));
}

void VertexBuffer::Upload(const void *data, size_t bytes) {
  glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
  if (bytes > m_capacity) {         // only reallocate when it outgrows
    m_capacity = bytes + bytes / 2; // 50% headroom
    glBufferData(GL_ARRAY_BUFFER, m_capacity, nullptr, GL_DYNAMIC_DRAW);
  }
  if (bytes)
    glBufferSubData(GL_ARRAY_BUFFER, 0, bytes, data);
}
