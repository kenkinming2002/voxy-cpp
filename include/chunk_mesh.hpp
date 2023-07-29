#ifndef CHUNK_MESH_HPP
#define CHUNK_MESH_HPP

#include <chunk_defs.hpp>

#include <mesh.hpp>

class ChunkManager;
Mesh generate_chunk_mesh(glm::ivec2 chunk_position, const ChunkManager& chunk_manager);

#endif // CHUNK_MESH_HPP
