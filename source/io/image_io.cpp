#include <jay/io/image_io.hpp>
#include <stb_image_write.h>
#include <stb_image.h>

#include <fstream>
#include <string>
#include <vector>

namespace jay
{
  int image_io::export_bmp(std::string filepath, image* img, int channels, bool yflip)
  {
    stbi_flip_vertically_on_write(yflip);
    auto status = stbi_write_bmp(filepath.data(), img->size.x, img->size.y, channels, img->data.data());
    if (status == 0)
      return -1;
    return 1;
  }

  int image_io::export_png(std::string filepath, image* img, int channels, std::size_t row_stride_in_bytes)
  {
    stbi_flip_vertically_on_write(true);
    auto status = stbi_write_png(filepath.data(), img->size.x, img->size.y, channels, img->data.data(), row_stride_in_bytes);
    if (status == 0)
      return -1;
    return 1;
  }

  image image_io::import_image(std::string filepath)
  {
    int x, y, n;
    image img;
    std::uint8_t* data = stbi_load(filepath.data(), &x, &y, &n, 3);
    
    auto imgsize = x * y * 3 * sizeof(std::uint8_t);
    
    for (auto i = 0; i < x * y * 3; i++)
    {
      img.data.push_back(*(data + i));
    }

    img.size = glm::ivec2(x, y);

    return img;
  }
}