#include <text_renderer.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>

struct Vertex
{
  glm::vec2 position;
  glm::vec2 tex_coords;
};

TextRenderer::TextRenderer(const char *font, unsigned height) : m_shader_program("./assets/ui.vert", "./assets/ui.frag")
{
  FT_Library library;
  FT_Face    face;

  if(FT_Init_FreeType(&library) != 0)
    throw std::runtime_error("Failed to initialize freetype");

  if(FT_New_Face(library, font, 0, &face) != 0)
    throw std::runtime_error("Failed to load font");

  if(FT_Set_Pixel_Sizes(face, 0, height) != 0)
    throw std::runtime_error("Failed to set pixel sizes");

  for(int c = 0; c<128; ++c)
  {
    if(FT_Load_Char(face, c, FT_LOAD_RENDER) != 0)
      throw std::runtime_error("Failed to load character");

    m_glyphs[c].dimenson.x = face->glyph->bitmap.width;
    m_glyphs[c].dimenson.y = face->glyph->bitmap.rows;

    m_glyphs[c].bearing.x = face->glyph->bitmap_left;
    m_glyphs[c].bearing.y = -(int)face->glyph->bitmap.rows+(int)face->glyph->bitmap_top;

    m_glyphs[c].advance.x = face->glyph->advance.x / 64.0f;
    m_glyphs[c].advance.y = face->glyph->advance.y / 64.0f;

    glGenTextures(1, &m_glyphs[c].texture);
    glBindTexture(GL_TEXTURE_2D, m_glyphs[c].texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
  }

  FT_Done_Face(face);
  FT_Done_FreeType(library);

  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);

  glGenBuffers(1, &m_veo);

  uint8_t indices[] = { 0, 1, 2, 2, 1, 3 };
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_veo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);

  glGenBuffers(1, &m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
}

void TextRenderer::render(glm::vec2& cursor, const char *str)
{
  glDisable(GL_DEPTH_TEST);

  glUseProgram(m_shader_program.id());
  glBindVertexArray(m_vao);

  glm::mat4 projection = glm::ortho(0.0f, 1024.0f, 0.0f, 720.0f);
  glUniformMatrix4fv(glGetUniformLocation(m_shader_program.id(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

  for(const char *it = str; *it; ++it)
  {
    int c = *it;
    assert(c >= 0 && c < 128);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_glyphs[c].texture);
    glUniform1i(glGetUniformLocation(m_shader_program.id(), "ourTexture"), 0);

    glm::vec2 position  = cursor + m_glyphs[c].bearing;
    glm::vec2 dimension = m_glyphs[c].dimenson;
    struct Vertex vertices[] = {
      { .position = position,                                .tex_coords = glm::vec2(0.0f, 1.0f), },
      { .position = position + glm::vec2(dimension.x, 0.0f), .tex_coords = glm::vec2(1.0f, 1.0f), },
      { .position = position + glm::vec2(0.0f, dimension.y), .tex_coords = glm::vec2(0.0f, 0.0f), },
      { .position = position + dimension,                    .tex_coords = glm::vec2(1.0f, 0.0f), },
    };
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof vertices, nullptr, GL_DYNAMIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, (void*)0);

    cursor += m_glyphs[c].advance;
  }

  glEnable(GL_DEPTH_TEST);
}

TextRenderer::~TextRenderer()
{
  for(int c = 0; c<128; ++c)
    glDeleteTextures(1, &m_glyphs[c].texture);
}

