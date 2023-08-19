#include <player_controller.hpp>

#include <directions.hpp>
#include <ray_cast.hpp>

#include <glm/gtc/type_ptr.hpp>

PlayerController::PlayerController()
{
  m_first    = true;
  m_cooldown = 0.0f;

  m_cube_shader_program = std::make_unique<graphics::ShaderProgram>("assets/ui_selection.vert", "assets/ui_selection.frag");

  std::vector<uint8_t>  indices;
  std::vector<glm::vec3> vertices;

  indices.push_back(0); indices.push_back(1);
  indices.push_back(2); indices.push_back(3);
  indices.push_back(4); indices.push_back(5);
  indices.push_back(6); indices.push_back(7);

  indices.push_back(0); indices.push_back(2);
  indices.push_back(4); indices.push_back(6);
  indices.push_back(1); indices.push_back(3);
  indices.push_back(5); indices.push_back(7);

  indices.push_back(0); indices.push_back(4);
  indices.push_back(1); indices.push_back(5);
  indices.push_back(2); indices.push_back(6);
  indices.push_back(3); indices.push_back(7);

  for(int z : {0, 1})
    for(int y : {0, 1})
      for(int x : {0, 1})
        vertices.push_back(glm::vec3(x, y, z));

  graphics::MeshLayout layout{
    .index_type = graphics::IndexType::UNSIGNED_BYTE,
      .stride = sizeof(glm::vec3),
      .attributes = {
        { .type = graphics::AttributeType::FLOAT3, .offset = 0, },
      },
  };

  m_cube_mesh = std::make_unique<graphics::Mesh>(
    std::move(layout),
    graphics::as_bytes(indices),
    graphics::as_bytes(vertices)
  );
}

static bool aabb_collide(glm::vec3 position1, glm::vec3 dimension1, glm::vec3 position2, glm::vec3 dimension2)
{
  glm::vec3 min = glm::max(position1,            position2);
  glm::vec3 max = glm::min(position1+dimension1, position2+dimension2);
  for(int i=0; i<3; ++i)
    if(min[i] >= max[i])
      return false;
  return true;
}

void PlayerController::update(Application& application, World& world, float dt)
{
  // 1: Jump
  if(application.glfw_get_key(GLFW_KEY_SPACE) == GLFW_PRESS)
    if(world.player.grounded)
    {
      world.player.grounded = false;
      entity_apply_impulse(world.player, JUMP_STRENGTH * glm::vec3(0.0f, 0.0f, 1.0f));
    }

  // 2: Movement
  glm::vec3 translation = glm::vec3(0.0f);
  if(application.glfw_get_key(GLFW_KEY_D) == GLFW_PRESS) translation += world.player.transform.local_right();
  if(application.glfw_get_key(GLFW_KEY_A) == GLFW_PRESS) translation -= world.player.transform.local_right();
  if(application.glfw_get_key(GLFW_KEY_W) == GLFW_PRESS) translation += world.player.transform.local_forward();
  if(application.glfw_get_key(GLFW_KEY_S) == GLFW_PRESS) translation -= world.player.transform.local_forward();

  if(glm::vec3 direction = translation; direction.z = 0.0f, glm::length(direction) != 0.0f)
    entity_apply_force(world.player, MOVEMENT_SPEED * glm::normalize(direction), dt);
  else if(glm::vec3 direction = -world.player.velocity; direction.z = 0.0f, glm::length(direction) != 0.0f)
    entity_apply_force(world.player, MOVEMENT_SPEED * glm::normalize(direction), dt, glm::length(direction));

  // 3: Rotation
  double new_cursor_xpos;
  double new_cursor_ypos;
  application.glfw_get_cursor_pos(new_cursor_xpos, new_cursor_ypos);
  if(!m_first)
  {
    double xrel = new_cursor_xpos - m_cursor_xpos;
    double yrel = new_cursor_ypos - m_cursor_ypos;
    world.player.transform = world.player.transform.rotate(glm::vec3(0.0f,
      -yrel * ROTATION_SPEED,
      -xrel * ROTATION_SPEED
    ));
  }
  m_first = false;
  m_cursor_xpos = new_cursor_xpos;
  m_cursor_ypos = new_cursor_ypos;

  // 4: Block placement/destruction
  m_cooldown = std::max(m_cooldown - dt, 0.0f);

  world.selection.reset();
  world.placement.reset();
  ray_cast(world.player.transform.position + world.player.eye_offset, world.player.transform.local_forward(), RAY_CAST_LENGTH, [&](glm::ivec3 block_position) -> bool {
      const Block *block = get_block(world, block_position);
      if(block && block->id != BLOCK_ID_NONE)
        world.selection = block_position;
      else
        world.placement = block_position;
      return block && block->id != BLOCK_ID_NONE;
  });

  // Can only place against a selected block
  if(!world.selection)
    world.placement.reset();

  if(m_cooldown == 0.0f)
    if(application.glfw_get_mouse_button(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
      if(world.selection)
        if(Block *block = get_block(world, *world.selection))
          if(block->id != BLOCK_ID_NONE)
          {
            if(block->destroy_level != 15)
              ++block->destroy_level;
            else
              block->id = BLOCK_ID_NONE;

            invalidate_mesh(world, *world.selection);
            invalidate_light     (world, *world.selection);
            for(glm::ivec3 direction : DIRECTIONS)
            {
              glm::ivec3 neighbour_position = *world.selection + direction;
              invalidate_mesh(world, neighbour_position);
            }
            m_cooldown = ACTION_COOLDOWN;
          }

  if(m_cooldown == 0.0f)
    if(application.glfw_get_mouse_button(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
      if(world.placement)
        if(Block *block = get_block(world, *world.placement))
          if(block->id == BLOCK_ID_NONE)
            if(!aabb_collide(world.player.transform.position, world.player.bounding_box, *world.placement, glm::vec3(1.0f, 1.0f, 1.0f))) // Cannot place a block that collide with the player
            {
              block->id = BLOCK_ID_STONE;
              invalidate_mesh(world, *world.placement);
              invalidate_light     (world, *world.placement);
              for(glm::ivec3 direction : DIRECTIONS)
              {
                glm::ivec3 neighbour_position = *world.placement + direction;
                invalidate_mesh(world, neighbour_position);
              }
              m_cooldown = ACTION_COOLDOWN;
            }
}

void PlayerController::render(const Camera& camera, const World& world)
{
  // 1: Selection
  if(world.selection)
  {
    glm::vec3 position = *world.selection;

    glm::mat4 view       = camera.view();
    glm::mat4 projection = camera.projection();
    glm::mat4 model      = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 MVP        = projection * view * model;

    glUseProgram(m_cube_shader_program->id());
    glUniformMatrix4fv(glGetUniformLocation(m_cube_shader_program->id(), "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));

    glLineWidth(UI_SELECTION_THICKNESS);
    m_cube_mesh->draw_lines();
  }

  // 1: Selection
  if(world.placement)
  {
    glm::vec3 position = *world.placement;

    glm::mat4 view       = camera.view();
    glm::mat4 projection = camera.projection();
    glm::mat4 model      = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 MVP        = projection * view * model;

    glUseProgram(m_cube_shader_program->id());
    glUniformMatrix4fv(glGetUniformLocation(m_cube_shader_program->id(), "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));

    glLineWidth(UI_SELECTION_THICKNESS);
    m_cube_mesh->draw_lines();
  }
}
