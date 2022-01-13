#include <jay/io/io.hpp>

namespace jay
{
  io::io()
    : hdf5_handler(nullptr)
    , astc_handler(std::make_unique<astc_io>(astc_io()))
  {
    // no-op
  }


  /* =============================================================

                        General I/O Operations

   ============================================================= */

  void io::store_buffer(char* buffer, std::size_t buffer_length, std::string filepath, bool new_file)
  {
    data_io::store_binary(buffer, buffer_length, filepath, new_file);
  }

  
  char* io::read_buffer(std::string filepath)
  {
    return data_io::read_binary(filepath);
  }


  /* =============================================================

                      HDF5 I/O Operations

     ============================================================= */

  void io::hdf5_open(std::string filepath, std::vector<std::string> datasets)
  {
    hdf5_handler = std::make_unique<hdf5_io>(hdf5_io(filepath, datasets));
  }


  /* =============================================================

                    ASTC I/O Operations

     ============================================================= */

  void io::astc_store(const jayComp<astc_datatype>& comp_imgs, std::string filepath, bool newFile)
  {
    astc_handler->astc_store(comp_imgs, filepath, newFile);
  }


  void io::astc_store(const jayComp<astc_datatype>& comp_imgs, std::size_t img_offset, std::string filepath, bool newFile)
  {
    astc_handler->astc_store(comp_imgs, img_offset, filepath, newFile);
  }


  void io::astc_store(const jayComp<astc_datatype>& comp_imgs, std::size_t img_offset, std::size_t img_count, std::string filepath, bool newFile)
  {
    astc_handler->astc_store(comp_imgs, img_offset, img_count, filepath, newFile);
  }


  jayComp<astc_datatype> io::astc_read(std::string filename)
  {
    return astc_handler->astc_read(filename);
  }


  std::vector<jayComp<astc_datatype>> io::astc_read_multiple(const std::vector<std::string>& filenames)
  {
    return astc_handler->astc_read_multiple(filenames);
  }
}