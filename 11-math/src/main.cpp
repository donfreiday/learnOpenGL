/* g++ -g -lglfw -lGLEW -lGLU -lGL src/*.cpp -o bin/a.out && bin/a.out */

#include <GL/glew.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>

#include "Renderer.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void error_callback(int error, const char *description);
static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods);

int main(int argc, char **argv) {
  glfwSetErrorCallback(error_callback);

  if (!glfwInit()) {
    std::cout << "Error: Failed to initialize GLFW.\n";
    return -1;
  }
  std::cout << "Status: Using GLFW version " << glfwGetVersionString() << std::endl;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(960, 540, "Learn OpenGL", NULL, NULL);
  if (!window) {
    std::cout << "Error: Failed to create Window or OpenGL context.\n";
    glfwTerminate();
  }

  glfwSetKeyCallback(window, key_callback);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Vsync

  // A valid OpenGL rendering context must be created BEFORE calling glewInit()!
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    std::cout << "Error: glewInit failed: " << glewGetErrorString(err) << std::endl;
    glfwTerminate();
    return -1;
  }

  std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  GLCall(std::cout << "Status: Using OpenGL version " << glGetString(GL_VERSION) << std::endl);

  // Alpha transparency blending
  GLCall(glEnable(GL_BLEND));
  GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  
  { /* Vertex and index buffers will go out of scope so their destructors are
     called before OpenGL context is lost, prevents infinite loop w/glGetError
   */

  // Vertex buffer: posX, posy, textureX, textureY
  float positions[] = {
       100.0f,  100.0f, 0.0f, 0.0f, // bottom left
       200.0f,  100.0f, 1.0f, 0.0f, // bottom right
       200.0f,  200.0f, 1.0f, 1.0f, // top right
       100.0f,  200.0f, 0.0f, 1.0f  // top left
  };
  VertexBuffer vb(positions, 4 * 4 * sizeof(float));

  VertexBufferLayout layout;
  layout.Push<float>(2); // position
  layout.Push<float>(2); // texture coordinates

  VertexArray va;
  va.AddBuffer(vb, layout);

  // Index buffer: each int refers the index of a vertice in a vertex buffer
  unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0
  };
  IndexBuffer ib(indices, 6);

  /* Create a 4x4 orthographic projection matrix. 
     The left, right, top, bottom must match the aspect ratio of our target surface, 4:3;
     they specify the boundaries of our window.
     Near and far aren't strictly necessary for our purposes.
  */
  glm::mat4 proj = glm::ortho(0.0f, 960.0f, 0.0f, 540.0f, -1.0f, 1.0f);

  // Move camera to the right by translating the scene to the left
  glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-100.0f, 0.0f, 0.0f));
  
  // Move the model up and right
  glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(200, 200, 0));
  
  // Combine MVP (Model, View, and Projection) matrices
  glm::mat4 mvp = proj * view * model;
  
  Shader shader("res/shaders/Basic.shader");
  shader.Bind();
  shader.SetUniform4f("u_Color",0.2f, 0.3f, 0.8f, 1.0f);
  shader.SetUniformMat4f("u_MVP", mvp);

  Texture texture("res/textures/bowser.png");
  texture.Bind();
  
  // Texture slot is passed to shader by integer uniform
  shader.SetUniform1i("u_Texture", 0);

  va.Unbind();
  vb.Unbind();
  ib.Unbind();
  shader.Unbind();

  Renderer renderer;

  // For animating color
  float r = 0.0f;
  float increment = 0.05f;

  while (!glfwWindowShouldClose(window)) {
    renderer.Clear();

    // We don't have materials yet so
    shader.Bind();
    shader.SetUniform4f("u_Color", r, 0.3f, 0.8f, 1.0f);

    renderer.Draw(va, ib, shader);

    if (r > 1.0f)
      increment = -0.05f;
    else if (r < 0.0f)
      increment = 0.05f;
    r += increment;

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

} /* Vertex and index buffers will go out of scope so their destructors are
     called before OpenGL context is lost, prevents infinite loop w/glGetError
   */
  
glfwDestroyWindow(window);
glfwTerminate();

return 0;
}

void error_callback(int error, const char *description) {
  std::cout<< "Error: GLFW error: " << description << std::endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

