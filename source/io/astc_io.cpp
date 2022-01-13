#include <fstream>

#include <jay/io/astc_io.hpp>
#include <jay/io/data_io.hpp>

// ASTC Header
static const uint32_t ASTC_MAGIC_ID = 0x5CA1AB13;

namespace jay
{
  /* =============================================================

                        ASTC I/O Operations

     ============================================================= */

  unsigned int astc_io::unpack_bytes(
    std::uint8_t a,
    std::uint8_t b,
    std::uint8_t c,
    std::uint8_t d
  ) {
    return ((unsigned int)(a)) +
      ((unsigned int)(b) << 8) +
      ((unsigned int)(c) << 16) +
      ((unsigned int)(d) << 24);
  }


  void astc_io::astc_store(const jayComp<astc_datatype>& comp_imgs, std::size_t img_offset, std::size_t img_count, std::string filepath, bool newFile)
  {
    std::size_t vector_offset = img_offset * comp_imgs.img_len;
    std::size_t data_size     = img_count * comp_imgs.img_len;

    if (newFile)
    {
      astc_header hdr;
      hdr.magic[0] = ASTC_MAGIC_ID & 0xFF;
      hdr.magic[1] = (ASTC_MAGIC_ID >> 8) & 0xFF;
      hdr.magic[2] = (ASTC_MAGIC_ID >> 16) & 0xFF;
      hdr.magic[3] = (ASTC_MAGIC_ID >> 24) & 0xFF;

      hdr.block_x = comp_imgs.block_x;
      hdr.block_y = comp_imgs.block_y;
      hdr.block_z = comp_imgs.block_z;

      hdr.dim_x[0] =  comp_imgs.dim_x & 0xFF;
      hdr.dim_x[1] = (comp_imgs.dim_x >> 8) & 0xFF;
      hdr.dim_x[2] = (comp_imgs.dim_x >> 16) & 0xFF;

      hdr.dim_y[0] =  comp_imgs.dim_y & 0xFF;
      hdr.dim_y[1] = (comp_imgs.dim_y >> 8) & 0xFF;
      hdr.dim_y[2] = (comp_imgs.dim_y >> 16) & 0xFF;

      hdr.dim_z[0] =  comp_imgs.dim_z & 0xFF;
      hdr.dim_z[1] = (comp_imgs.dim_z >> 8) & 0xFF;
      hdr.dim_z[2] = (comp_imgs.dim_z >> 16) & 0xFF;

      data_io::store_binary((char*)&hdr, sizeof(astc_header), (char*) &comp_imgs.data[vector_offset], data_size, filepath, true);
    }
    else
    {
      data_io::store_binary((char*)&comp_imgs.data[vector_offset], data_size, filepath, true);
    }
  }


  void astc_io::astc_store(const jayComp<astc_datatype>& comp_imgs, std::size_t img_offset, std::string filepath, bool newFile)
  {
    std::size_t number_of_images = comp_imgs.data_len / comp_imgs.img_len - img_offset;

    astc_store(comp_imgs, img_offset, number_of_images, filepath, newFile);
  }


  void astc_io::astc_store(const jayComp<astc_datatype>& comp_imgs, std::string filepath, bool newFile)
  {
    std::size_t number_of_images = comp_imgs.data_len / comp_imgs.img_len;

    astc_store(comp_imgs, 0, number_of_images, filepath, newFile);
  }


  jayComp<astc_datatype> astc_io::astc_read(
    std::string filepath
  )
  {
    // Open the astc-compressed file on disk
    std::ifstream astc_file(filepath, std::ios::in | std::ios::binary | std::ios::ate);

    if (!astc_file)
    {
      printf("Error: File open failed '%s'\n", filepath);
      return {};
    }

    // Read complete filesize
    std::size_t filesize = astc_file.tellg();
    astc_file.seekg(0, std::ios::beg);


    // Read the header (once)..
    astc_header hdr;
    astc_file.read(reinterpret_cast<char*>(&hdr), sizeof(astc_header));

    unsigned int magicval = unpack_bytes(hdr.magic[0], hdr.magic[1], hdr.magic[2], hdr.magic[3]);
    if (magicval != ASTC_MAGIC_ID)
    {
      printf("Error: File not recognized as ASTC: '%s'\n", filepath);
      return {};
    }


    // .. -> evaluate block sizes
    unsigned int block_x = (hdr.block_x > 1) ? hdr.block_x : 1;
    unsigned int block_y = (hdr.block_y > 1) ? hdr.block_y : 1;
    unsigned int block_z = (hdr.block_z > 1) ? hdr.block_z : 1;

    // .. -> evaluate image dimensions
    unsigned int dim_x = unpack_bytes(hdr.dim_x[0], hdr.dim_x[1], hdr.dim_x[2], 0);
    unsigned int dim_y = unpack_bytes(hdr.dim_y[0], hdr.dim_y[1], hdr.dim_y[2], 0);
    unsigned int dim_z = unpack_bytes(hdr.dim_z[0], hdr.dim_z[1], hdr.dim_z[2], 0);

    if (dim_x == 0 || dim_y == 0 || dim_z == 0)
    {
      printf("Error: File corrupt: '%s'\n", filepath);
      return {};
    }

    // .. -> evaluate block count
    unsigned int blocks_x = (dim_x + block_x - 1) / block_x;
    unsigned int blocks_y = (dim_y + block_y - 1) / block_y;
    unsigned int blocks_z = (dim_z + block_z - 1) / block_z;

    // .. -> evaluate (single) image size
    unsigned int img_size = (blocks_x * blocks_y * blocks_z) << 4;

    // .. -> evaluate (complete) data size
    unsigned int data_size = filesize - sizeof(astc_header);

    if (data_size % img_size > 0)
    {
      printf("Error: File corrupt: '%s'\n", filepath);
      return {};
    }

    jayComp<astc_datatype> compressed_images{};
    compressed_images.block_x = block_x;
    compressed_images.block_y = block_y;
    compressed_images.block_z = block_z;
    compressed_images.dim_x = dim_x;
    compressed_images.dim_y = dim_y;
    compressed_images.dim_z = dim_z;
    compressed_images.img_len = img_size;
    compressed_images.data_len = data_size;

    // Read the data (each slice)
    compressed_images.data.resize(data_size);

    astc_file.read((char*)compressed_images.data.data(), data_size);

    if (!astc_file)
    {
      printf("Error: File read failed: '%s'\n", filepath);
      return {};
    }

    astc_file.close();

    return compressed_images;
  }


  std::vector<jayComp<astc_datatype>> astc_io::astc_read_multiple(const std::vector<std::string>& filenames)
  {
    std::vector<jayComp<astc_datatype>> compressed_images;
    compressed_images.reserve(filenames.size());

    for (std::uint16_t i = 0; i < filenames.size(); i++)
    {
      compressed_images.push_back(astc_read(filenames[i]));
    }

    return compressed_images;
  }
}