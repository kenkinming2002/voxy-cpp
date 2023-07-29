#include <texture_array.hpp>

#include <stb_image.h>
#include <assert.h>

TextureArray::TextureArray(const std::vector<std::string>& filenames)
{
  glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_id);

  std::vector<stbi_uc> bytes;
  int width  = 0;
  int height = 0;
  int depth  = filenames.size();

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

  glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes.data());
  glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

TextureArray::~TextureArray()
{
  glDeleteTextures(1, &m_id);
}
