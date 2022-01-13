#ifndef JAY_IMG_IO_HPP
#define JAY_IMG_IO_HPP

#include <string>
#include<jay/types/image.hpp>
#include <jay/types/image.hpp>
#include <jay/export.hpp>

namespace jay
{
struct JAY_EXPORT image_io
{
  static int export_bmp(std::string filepath, image* img, int channels, bool yflip = true);
  static int export_png(std::string filepath, image* img, int channels, std::size_t row_stride_in_bytes);
  static image import_image(std::string filepath);
};
}

#endif