#include <text_renderer.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>

Font::Font(const char *font, unsigned height)
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

    m_glyphs[c].texture = std::make_unique<graphics::Texture>(face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows, 1);
  }

  FT_Done_Face(face);
  FT_Done_FreeType(library);
}

TextRenderer::TextRenderer()
{
  struct Vertex
  {
    glm::vec2 position;
    glm::vec2 tex_coords;
  };

  m_shader_program = std::make_unique<graphics::ShaderProgram>("./assets/ui.vert", "./assets/ui.frag");
  m_quad_mesh = std::make_unique<graphics::Mesh>(
    graphics::MeshLayout{
      .index_type = graphics::IndexType::UNSIGNED_BYTE,
      .stride     = sizeof(Vertex),
      .attributes = {
        { .type = graphics::AttributeType::FLOAT2, .offset = offsetof(Vertex, position),   },
        { .type = graphics::AttributeType::FLOAT2, .offset = offsetof(Vertex, tex_coords), },
      }
    },
    graphics::as_bytes(std::vector<uint8_t>{0, 1, 2, 2, 1, 3}),
    graphics::as_bytes(std::vector<Vertex>{
      { .position = glm::vec2(0.0f, 0.0f), .tex_coords = glm::vec2(0.0f, 1.0f), },
      { .position = glm::vec2(1.0f, 0.0f), .tex_coords = glm::vec2(1.0f, 1.0f), },
      { .position = glm::vec2(0.0f, 1.0f), .tex_coords = glm::vec2(0.0f, 0.0f), },
      { .position = glm::vec2(1.0f, 1.0f), .tex_coords = glm::vec2(1.0f, 0.0f), },
    })
  );
}

void TextRenderer::render(glm::vec2 dimension, glm::vec2 position, const Font& font, const char* str)
{
  glDisable(GL_DEPTH_TEST);

  glUseProgram(m_shader_program->id());

  for(const char *it = str; *it; ++it)
  {
    int c = *it;
    assert(c >= 0 && c < 128);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font.m_glyphs[c].texture->id());
    glUniform1i(glGetUniformLocation(m_shader_program->id(), "ourTexture"), 0);

    glm::vec2 glyph_position  = position + font.m_glyphs[c].bearing;
    glm::vec2 glyph_dimension = font.m_glyphs[c].dimenson;

    glm::mat4 projection = glm::ortho(0.0f, (float)dimension.x, 0.0f, (float)dimension.y);
    glm::mat4 view       = glm::mat4(1.0f);

    glm::mat4 model  = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(glyph_position .x, glyph_position .y, 0.0f));
    model = glm::scale    (model, glm::vec3(glyph_dimension.x, glyph_dimension.y, 0.0f));

    glm::mat4 MVP = projection * view * model;
    glUniformMatrix4fv(glGetUniformLocation(m_shader_program->id(), "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));

    m_quad_mesh->draw_triangles();

    position += font.m_glyphs[c].advance;
  }

  glEnable(GL_DEPTH_TEST);
}
