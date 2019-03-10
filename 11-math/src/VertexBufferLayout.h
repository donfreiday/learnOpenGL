#pragma once

#include <iostream>

#include <GL/glew.h>
#include <vector>

#include "Renderer.h"

struct VertexBufferElement {
  unsigned int type;
  unsigned int count;
  unsigned char normalized;

  static unsigned int GetSizeOfType(unsigned int type) {
    switch (type) {
    case GL_FLOAT:
      return 4;
    case GL_UNSIGNED_INT:
      return 4;
    case GL_UNSIGNED_BYTE:
      return 1;
    }
    ASSERT(false);
    return 0;
  }
};

class VertexBufferLayout {
private:
  std::vector<VertexBufferElement> m_Elements;
  unsigned int m_Stride;

public:
  VertexBufferLayout() : m_Stride(0){}
  ~VertexBufferLayout(){}

  /* VC++ is not standards compliant; GCC doesn't allow in-class specialization
   * Workaround is to have template specializations outside class scope (bottom
   * of this header)*/
  template<typename T> 
  void Push(unsigned int count) {
    std::cout << "Error: unsupported type " << typeid(T).name() << std::endl;
    DEBUG_BREAK;
  }

  inline const std::vector<VertexBufferElement> &GetElements() const {
    return m_Elements;
  }
  inline unsigned int GetStride() const { return m_Stride; };
};

  template<> 
  inline void VertexBufferLayout::Push<float>(unsigned int count) {
    m_Elements.push_back({GL_FLOAT, count, GL_FALSE});
    m_Stride += count * VertexBufferElement::GetSizeOfType(GL_FLOAT);
  }

  template<> 
  inline void VertexBufferLayout::Push<unsigned int>(unsigned int count) {
    m_Elements.push_back({GL_UNSIGNED_INT, count, GL_FALSE});
    m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT);
  }

  template<>
  inline void VertexBufferLayout::Push<unsigned char>(unsigned int count) {
    m_Elements.push_back({GL_UNSIGNED_BYTE, count, GL_TRUE});
    m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE);
  }
  