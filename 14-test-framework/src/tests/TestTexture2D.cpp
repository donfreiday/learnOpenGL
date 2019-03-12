#include "TestTexture2D.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer.h"

#include "imgui/imgui.h"

namespace test {
TestTexture2D::TestTexture2D()
    : m_TranslationA(50, 50, 0), m_TranslationB(600, 50, 0),
      m_Proj(glm::ortho(0.0f, 960.0f, 0.0f, 540.0f, -1.0f, 1.0f)),
      m_View(glm::mat4(1.0f)), m_DirectionAx(1), m_DirectionAy(1),
      m_DirectionBx(1), m_DirectionBy(1), m_SpeedA({2, 2}), m_SpeedB({2, 2}) {
  float positions[] = {
      -50.0f, -50.0f, 0.0f, 0.0f, // bottom left
       50.0f, -50.0f, 1.0f, 0.0f, // bottom right
       50.0f,  50.0f, 1.0f, 1.0f, // top right
      -50.0f,  50.0f, 0.0f, 1.0f  // top left
  };
  unsigned int indices[] = {0, 1, 2, 2, 3, 0};

  // Alpha transparency blending
  GLCall(glEnable(GL_BLEND));
  GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

  m_VAO = std::make_unique<VertexArray>();
  m_VertexBuffer = std::make_unique<VertexBuffer>(positions, 4 * 4 * sizeof(float));
  VertexBufferLayout layout;
  layout.Push<float>(2); // position
  layout.Push<float>(2); // texture coordinates
  
  m_VAO->AddBuffer(*m_VertexBuffer, layout);
  m_IndexBuffer = std::make_unique<IndexBuffer>(indices, 6);

  m_Shader = std::make_unique<Shader>("res/shaders/Basic.shader");
  m_Shader->Bind();
  m_Texture = std::make_unique<Texture>("res/textures/bowser.png");
  m_Shader->SetUniform1i("u_Texture", 0);
}

TestTexture2D::~TestTexture2D() {}

void TestTexture2D::OnUpdate(float deltaTime) {}

void TestTexture2D::OnRender() {
  GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
  GLCall(glClear(GL_COLOR_BUFFER_BIT));
  Renderer renderer;
  m_Texture->Bind();

  {
    if (m_TranslationA.x >= 960 || m_TranslationA.x <= 0)
      m_DirectionAx *= -1;
    if (m_TranslationA.y >= 540 || m_TranslationA.y <= 0)
      m_DirectionAy *= -1;
    m_TranslationA.x += m_DirectionAx * m_SpeedA[0];
    m_TranslationA.y += m_DirectionAy * m_SpeedA[1];
    glm::mat4 model = glm::translate(glm::mat4(1.0f), m_TranslationA);
    glm::mat4 mvp = m_Proj * m_View * model;
    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_MVP", mvp);
    renderer.Draw(*m_VAO, *m_IndexBuffer, *m_Shader);
  }

  {
    if (m_TranslationB.x >= 960 || m_TranslationB.x <= 0)
      m_DirectionBx *= -1;
    if (m_TranslationB.y >= 540 || m_TranslationB.y <= 0)
      m_DirectionBy *= -1;
    m_TranslationB.x += m_DirectionBx * m_SpeedB[0];
    m_TranslationB.y += m_DirectionBy * m_SpeedB[1];
    glm::mat4 model = glm::translate(glm::mat4(1.0f), m_TranslationB);
    glm::mat4 mvp = m_Proj * m_View * model;
    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_MVP", mvp);
    renderer.Draw(*m_VAO, *m_IndexBuffer, *m_Shader);
  }
}

void TestTexture2D::OnImGuiRender() {
  ImGui::SliderFloat3("m_TranslationA", &m_TranslationA.x, 0.0f, 960.0f);
  ImGui::SliderFloat3("m_TranslationB", &m_TranslationB.x, 0.0f, 960.0f);
  ImGui::SliderInt2("SpeedA", &m_SpeedA[0], -20, 20);
  ImGui::SliderInt2("SpeedB", &m_SpeedB[0], -20, 20);
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}
} // namespace test