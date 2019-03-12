#include "Shader.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>

#include "Renderer.h"

Shader::Shader(const std::string &filepath) : m_Filepath(filepath), m_RendererID(0){
      ShaderProgramSource source = ParseShader(filepath);
  m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::~Shader() {
    GLCall(glDeleteProgram(m_RendererID));
}

// Parse shader from file
ShaderProgramSource Shader::ParseShader(const std::string& filepath) {
  std::ifstream stream(filepath);

  enum class ShaderType {
    NONE = -1, VERTEX = 0, FRAGMENT = 1
  };

  std::string line;
  std::stringstream ss[2];
  ShaderType type = ShaderType::NONE;
  while (getline(stream, line)) {
    if (line.find("#shader") != std::string::npos) {
      if (line.find("vertex") != std::string::npos) {
        type = ShaderType::VERTEX;
      } 
      else if (line.find("fragment") != std::string::npos) {
        type = ShaderType::FRAGMENT;
      }
    }
    else {
      ss[(int)type] << line << "\n";
    }
  }
  return { ss[0].str(), ss[1].str() };
}

// Compile shader and return its ID
unsigned int Shader::CompileShader(unsigned int type, const std::string& source) {
    GLCall(unsigned int id = glCreateShader(type));
    const char* src = source.c_str(); // equivalently &source[0]; source must not be out of scope!
    
    // Give shader the address of our source code string
    GLCall(glShaderSource(id, 1, &src, nullptr));

    // Compile the source
    GLCall(glCompileShader(id));

    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    
    // Handle errors
    if (result == GL_FALSE) {
     int length;
     GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));

     // alloca dynamically allocates stack memory
     char *message = (char *)alloca(length * sizeof(char));
     GLCall(glGetShaderInfoLog(id, length, &length, message));
     std::cout << "Error: "
               << ((type == GL_VERTEX_SHADER) ? "Vertex" : "Fragment")
               << " shader compilation failed: " << message << std::endl;
     GLCall(glDeleteShader(id));
     return 0;
   }
   return id;
}

// Combine vertex and fragment shaders into a shader program, returns program ID
unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
    // Compile our vertex and fragment shaders
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GLCall(unsigned int program = glCreateProgram());

    // Tell OpenGL we will be linking our shaders to our new shader program
    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));

    // Perform linking
    GLCall(glLinkProgram(program));

    // Validation will be stored as part of the program state.
    GLCall(glValidateProgram(program));

    /* Delete intermediate shader files since they've now been linked into a
     * program. glDetachShader would delete the shader source code. This makes
     * debugging harder, though it technically should be done. */
    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));

    return program;
}

void Shader::Bind() const {
    GLCall(glUseProgram(m_RendererID));
}

void Shader::Unbind() const {
    GLCall(glUseProgram(0));
}

void Shader::SetUniform1i(const std::string &name, int v0) {
  GLCall(glUniform1i(GetUniformLocation(name), v0));
}

void Shader::SetUniform1f(const std::string &name, float v0) {
  GLCall(glUniform1f(GetUniformLocation(name), v0));
}

void Shader::SetUniform4f(const std::string &name, float v0, float v1, float v2, float v3) {
  GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

void Shader::SetUniformMat4f(const std::string &name, const glm::mat4& matrix) {
  GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}

int Shader::GetUniformLocation(const std::string &name) {
    if(m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) {
      return m_UniformLocationCache[name];
    }

    GLCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));
    if(location==-1) {
        std::cout<<"Warning: uniform " << name << " doesn't exist!" << std::endl;
    }

    m_UniformLocationCache[name] = location;
    return location;
}
