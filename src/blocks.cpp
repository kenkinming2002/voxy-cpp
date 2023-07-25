#include <blocks.hpp>

#include <perlin.hpp>

#include <random>

Blocks::Blocks(size_t seed, glm::ivec2 cpos)
{
  std::mt19937 prng(seed);
  size_t stone_seed = prng();
  size_t grass_seed = prng();
  size_t cave_seed  = prng();

  int stone_heights[Blocks::WIDTH][Blocks::WIDTH];
  for(int cy=0; cy<Blocks::WIDTH; ++cy)
    for(int cx=0; cx<Blocks::WIDTH; ++cx)
    {
      glm::vec2 pos = glm::vec2(Blocks::WIDTH * cpos) + glm::vec2(cx, cy);
      stone_heights[cy][cx] = perlin(stone_seed, pos, 0.03f, 40.0f, 2.0f, 0.5f, 4);
    }

  int grass_heights[Blocks::WIDTH][Blocks::WIDTH];
  for(int cy=0; cy<Blocks::WIDTH; ++cy)
    for(int cx=0; cx<Blocks::WIDTH; ++cx)
    {
      glm::vec2 pos = glm::vec2(Blocks::WIDTH * cpos) + glm::vec2(cx, cy);
      grass_heights[cy][cx] = perlin(grass_seed, pos, 0.01f, 5.0f, 2.0f, 0.5f, 2);
    }

  int max_height = 0;
  for(int cy=0; cy<Blocks::WIDTH; ++cy)
    for(int cx=0; cx<Blocks::WIDTH; ++cx)
      if(max_height < stone_heights[cy][cx]+grass_heights[cy][cx])
        max_height = stone_heights[cy][cx]+grass_heights[cy][cx];

  for(int cz=0; cz<max_height; ++cz)
  {
    Blocks::Layer layer;
    for(int cy=0; cy<Blocks::WIDTH; ++cy)
      for(int cx=0; cx<Blocks::WIDTH; ++cx)
        if(cz <= stone_heights[cy][cx])
          layer.blocks[cy][cx] = Block {
            .presence = true,
            .color    = glm::vec3(0.7, 0.7, 0.7),
          };
        else if(cz <= stone_heights[cy][cx] + grass_heights[cy][cx])
          layer.blocks[cy][cx] = Block {
            .presence = true,
            .color    = glm::vec3(0.2, 1.0, 0.2),
          };
        else
          layer.blocks[cy][cx] = Block {
            .presence = false,
          };

    m_layers.push_back(layer);
  }

  // Hopefully carve some caves
  for(int lz=0; lz<height(); ++lz)
    for(int ly=0; ly<width(); ++ly)
      for(int lx=0; lx<width(); ++lx)
      {
        glm::ivec3 lpos  = { lx, ly, lz };
        glm::ivec3 gpos  = glm::ivec3(cpos.x * Blocks::WIDTH, cpos.y * Blocks::WIDTH, 0.0f) + lpos;

        float value = perlin(cave_seed, glm::vec3(gpos), 0.05f, 0.5f, 2.0f, 0.5f, 4);
        if(value<=0.4f)
          set(lpos, Block{ .presence = false });
      }
}

