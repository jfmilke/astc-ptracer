#ifndef JAY_TYPES_JAYDATA_HPP
#define JAY_TYPES_JAYDATA_HPP

#include <cstdint>
#include <vector>

#include <jay/io/io_enums.hpp>

template <typename T>
struct jaySrc
{
  std::vector<T>             data;
  std::vector<std::size_t>   grid;
  std::size_t                grid_dim;
  std::size_t                vec_len;
  jay::Order                 ordering;
};

template <typename T>
struct jayComp
{
  std::vector<T>           data;
  std::size_t              dim_x;
  std::size_t              dim_y;
  std::size_t              dim_z;
  std::size_t              block_x;
  std::size_t              block_y;
  std::size_t              block_z;
  std::size_t              img_len;
  std::size_t              data_len;
};

#endif
