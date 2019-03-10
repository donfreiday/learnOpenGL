#include "Texture.h"

#include "../vendor/stb_image/stb_image.h"

Texture::Texture(const std::string &path)
    : m_RendererID(0), m_FilePath(path), m_LocalBuffer(nullptr), m_Width(0),
      m_Height(0), m_BPP(0) {
  // OpenGL expects texture pixels to start at the bottom left, so we flip the
  // PNG upside down
  stbi_set_flip_vertically_on_load(1);
  m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4); // RGBA so 4 channels

  GLCall(glGenTextures(1, &m_RendererID));
  GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));

  // These four texture parameters are REQUIRED, no default values are provided
  // Minification is used if area to texture is smaller than the texture,
  // magnification if larger
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)); // Minification filter
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)); // Magnification filter

  // Wrap mode, clamp means not to extend the area (vs tiling)
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)); // S = X = Horizontal
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)); // T = Y = Vertical

  GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0,
                          GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer));

  GLCall(glBindTexture(GL_TEXTURE_2D, 0)); // Unbind

  if (m_LocalBuffer) {
    stbi_image_free(m_LocalBuffer);
  }
}

Texture::~Texture() { GLCall(glDeleteTextures(1, &m_RendererID)); }

void Texture::Bind(unsigned int slot) const {
  GLCall(glActiveTexture(GL_TEXTURE0 + slot)); // Bind texture slot
  GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
}

void Texture::Unbind() const { GLCall(glBindTexture(GL_TEXTURE_2D, 0)); }
