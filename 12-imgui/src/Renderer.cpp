#include "Renderer.h"

#include <iomanip>
#include <iostream>

// Clear OpenGL errors
void GLClearError() {
  while (glGetError() != GL_NO_ERROR); // GL_NO_ERROR = 0, so could be while(!glGetError())
}

/******************************************************************************/
// Return false if there are any OpenGl errors.
bool GLLogCall(const char* function, const char* file, int line) {
  while (GLenum error = glGetError()) {
    std::cout << "OpenGL Error 0x" << std::setfill('0') << std::setw(4)
              << std::hex << error << " : " << std::dec << function << " in "
              << file << ":" << line << std::endl;
    return false;
  }
  return true;
}

void Renderer::Clear() const {
  GLCall(glClear(GL_COLOR_BUFFER_BIT));
}

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const {
    shader.Bind();
    va.Bind();
    ib.Bind();
    GLCall(glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
}