#ifndef JAY_ASTC_IO_HPP
#define JAY_ASTC_IO_HPP

#include <vector>
#include <jay/types/image.hpp>
#include <jay/types/jaydata.hpp>

namespace jay
{

struct astc_header
{
  uint8_t magic[4];
  uint8_t block_x;
  uint8_t block_y;
  uint8_t block_z;
  uint8_t dim_x[3];			// dims = dim[0] + (dim[1] << 8) + (dim[2] << 16)
  uint8_t dim_y[3];			// Sizes are given in texels;
  uint8_t dim_z[3];			// block count is inferred
};

struct astc_io
{
  /* =============================================================

                          ASTC I/O Operations

     ============================================================= */

  static unsigned int unpack_bytes(
    std::uint8_t a,
    std::uint8_t b,
    std::uint8_t c,
    std::uint8_t d
  );

  // Stores an astc compressed file and its corresponding header.
  // newFile=true replaces existing files, newFile=false will concatenate. 
  static void astc_store(const jayComp<astc_datatype>& comp_imgs, std::string filepath, bool newFile);
  static void astc_store(const jayComp<astc_datatype>& comp_imgs, std::size_t img_offset, std::string filepath, bool newFile);
  static void astc_store(const jayComp<astc_datatype>& comp_imgs, std::size_t img_offset, std::size_t img_count, std::string filepath, bool newFile);

  // Reads an astc compressed file and its preceding header.
  // Returns a vector of astc compressed images (without the fileheader).
  static jayComp<astc_datatype> astc_read(std::string filename);

  // Reads multiple astc compressed files and its preceding headers.
  // Returns a vector of astc compressed images (without the fileheaders).
  static std::vector<jayComp<astc_datatype>> astc_read_multiple(const std::vector<std::string>& filenames);

};

}
#endif