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

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

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

  int windowX = 960, windowY = 540;
  GLFWwindow *window = glfwCreateWindow(windowX, windowY, "Learn OpenGL", NULL, NULL);
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
      -50.0f, -50.0f, 0.0f, 0.0f, // bottom left
       50.0f, -50.0f, 1.0f, 0.0f, // bottom right
       50.0f,  50.0f, 1.0f, 1.0f, // top right
      -50.0f,  50.0f, 0.0f, 1.0f  // top left
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
  glm::mat4 view = glm::mat4(1.0f);

  Texture texture("res/textures/bowser.png");
  texture.Bind();
  
  Shader shader("res/shaders/Basic.shader");
  shader.Bind();
  shader.SetUniform1i("u_Texture", 0); // Texture slot is passed to shader by integer uniform
  shader.Unbind();

  Renderer renderer;

  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  const char* glsl_version = "#version 130";
  ImGui_ImplOpenGL3_Init(glsl_version);

  glm::vec3 translationA(50, 50, 0);
  glm::vec3 translationB(600, 50, 0);

  int directionAx = 1;
  int directionAy = 1;
  int directionBx = 1;
  int directionBy = 1;
  int speedA[2] = {1, 1};
  int speedB[2] = {1, 1};
  while (!glfwWindowShouldClose(window)) {
    renderer.Clear();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Model translation");
      ImGui::SliderFloat3("TranslationA", &translationA.x, 0.0f, 960.0f); 
      ImGui::SliderFloat3("TranslationB", &translationB.x, 0.0f, 960.0f);
      ImGui::SliderInt2("SpeedA", &speedA[0], -20, 20); 
      ImGui::SliderInt2("SpeedB", &speedB[0], -20, 20); 
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::End();
    }

    shader.Bind();

    {
      if(translationA.x >= windowX || translationA.x <= 0)
        directionAx *= -1;
      if(translationA.y >= windowY || translationA.y <= 0)
        directionAy *= -1;
      translationA.x += directionAx * speedA[0];
      translationA.y += directionAy * speedA[1];
      glm::mat4 model = glm::translate(glm::mat4(1.0f), translationA);
      glm::mat4 mvp = proj * view * model;
      shader.SetUniformMat4f("u_MVP", mvp);
      renderer.Draw(va, ib, shader);
    }
    {
      if (translationB.x >= windowX || translationB.x <= 0)
        directionBx *= -1;
      if (translationB.y >= windowY || translationB.y <= 0)
        directionBy *= -1;
      translationB.x += directionBx * speedB[0];
      translationB.y += directionBy * speedB[1];
      glm::mat4 model = glm::translate(glm::mat4(1.0f), translationB);
      glm::mat4 mvp = proj * view * model;
      shader.SetUniformMat4f("u_MVP", mvp);
      renderer.Draw(va, ib, shader);
    }    

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

} /* Vertex and index buffers will go out of scope so their destructors are
     called before OpenGL context is lost, prevents infinite loop w/glGetError
   */

// Cleanup
ImGui_ImplOpenGL3_Shutdown();
ImGui_ImplGlfw_Shutdown();
ImGui::DestroyContext();

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

