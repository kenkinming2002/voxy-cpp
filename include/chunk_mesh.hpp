#ifndef CHUNK_MESH_HPP
#define CHUNK_MESH_HPP

#include <chunk_defs.hpp>
#include <chunk_data.hpp>

#include <mesh.hpp>

Mesh generate_chunk_mesh(glm::ivec2 chunk_position, const ChunkData& chunk_data, const std::vector<BlockData>& block_datas);

#endif // CHUNK_MESH_HPP
