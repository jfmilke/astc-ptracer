#ifndef JAY_ENUM_IO_HPP
#define JAY_ENUM_IO_HPP

#include <jay/export.hpp>

namespace jay {
enum class FileType
{
  None = 0,
  HDF5
};

enum class Order
{
  None = 0,
  VectorFirst,
  ComponentFirst
};
}

#endif