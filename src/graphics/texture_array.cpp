#include <graphics/texture_array.hpp>

#include <stb_image.h>

#include <assert.h>

TextureArray::TextureArray(const std::vector<std::string>& filenames)
{
  glGenTextures(1, &m_id);
  glBindTexture(GL_TEXTURE_2D_ARRAY, m_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  std::vector<stbi_uc> bytes;
  int width  = 0;
  int height = 0;
  int depth  = filenames.size();

  stbi_set_flip_vertically_on_load(true);
  for(const std::string& filename : filenames)
  {
    int _width, _height, channels_in_file;
    stbi_uc *_bytes = stbi_load(filename.c_str(), &_width, &_height, &channels_in_file, STBI_rgb_alpha);
    assert(_bytes);
    if(width  == 0) width  = _width;  else assert(width  == _width);
    if(height == 0) height = _height; else assert(height == _height);
    bytes.insert(bytes.end(), &_bytes[0], &_bytes[width * height * 4]);
    stbi_image_free(_bytes);
  }
  assert(width  != 0);
  assert(height != 0);

  assert(bytes.size() == width * height * depth * 4);

  glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes.data());
  glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

TextureArray::~TextureArray()
{
  glDeleteTextures(1, &m_id);
}
