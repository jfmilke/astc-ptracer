#ifndef JAY_TYPES_IMAGE_HPP
#define JAY_TYPES_IMAGE_HPP

#include <cstdint>
#include <vector>

#include <glm/vec2.hpp>

typedef std::uint8_t astc_datatype;

// Compressed ASTC image
struct astc_compressed_image
{
  unsigned int block_x;
  unsigned int block_y;
  unsigned int block_z;
  unsigned int dim_x;
  unsigned int dim_y;
  unsigned int dim_z;
  std::uint8_t* data;
  std::size_t data_len;
};

// RGB image
struct image
{
  std::vector<uint8_t> data;
  glm::vec<2, int>     size;
};

#endif
