#ifndef TEXTURE_ARRAY_HPP
#define TEXTURE_ARRAY_HPP

#include <glad/glad.h>

#include <vector>
#include <string>

struct TextureArray
{
public:
  TextureArray(const std::vector<std::string>& filenames);
  ~TextureArray();

private:
  GLuint m_id;
};

#endif // TEXTURE_ARRAY_HPP
