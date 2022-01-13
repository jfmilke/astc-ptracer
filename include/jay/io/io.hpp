#ifndef JAY_IO_HPP
#define JAY_IO_HPP

#include <glm/glm.hpp>

#include <jay/io/data_io.hpp>
#include <jay/io/astc_io.hpp>
#include <jay/io/hdf5_io.hpp>
#include <jay/io/image_io.hpp>
#include <jay/io/io_enums.hpp>
#include <jay/types/image.hpp>
#include <jay/types/jaydata.hpp>


#include <jay/export.hpp>

namespace jay
{
struct JAY_EXPORT io
{
  std::unique_ptr<hdf5_io> hdf5_handler;
  std::unique_ptr<astc_io> astc_handler;

  io();

  /* =============================================================

                        General I/O Operations

   ============================================================= */

  void store_buffer(char* buffer, std::size_t buffer_length, std::string filepath, bool new_file);


  char* read_buffer(std::string filepath);


  template <typename T>
  void store_vector(std::vector<T> vector, std::string filepath, bool new_file)
  {
    data_io::store_binary(reinterpret_cast<char*>(vector.data()), vector.size() * sizeof(T), filepath, new_file);
  }


  template <typename T>
  std::vector<T> read_vector(std::string filepath)
  {
    auto filesize = data_io::get_filesize(filepath);

    if ((filesize % sizeof(T)) > 0)
    {
      std::cout << "Error: Could not read file into a vector of the depicted type. Wrong format?" << std::endl;
      return {};
    }

    std::vector<T> filecontent(filesize / sizeof(T));
    data_io::read_binary_into(filecontent.data(), filepath);
    //filecontent.data() = reinterpret_cast<T*>(data_io::read_binary(filepath));

    return filecontent;
  }


  /* =============================================================

                        HDF5 I/O Operations

   ============================================================= */

  // Invoke for a single file you want to read at the moment.
  void hdf5_open(std::string filepath, std::vector<std::string> datasets);

  // Reads the opened file into a container with metainfo (preferred).
  template <typename T>
  jaySrc<T> hdf5_read(
    Order ordering = Order::VectorFirst
  )
  {
    const auto grid                 = hdf5_handler->get_grid_fixsize(false, 1);
    const auto grid_dim             = hdf5_handler->get_grid_dim();
    const auto vec_size             = hdf5_handler->get_vec_len();
    const auto grid_elements_scalar = grid[0] * grid[1] * grid[2] * grid[3];

    jaySrc<T> data_container{ std::vector<T>(grid_elements_scalar * vec_size), grid, grid_dim, vec_size, ordering };

    // Vectorlike ordering (stride = vec_size, offset = component_id)
    if (ordering == Order::VectorFirst)
      for (auto c = 0; c < vec_size; c++)
        hdf5_handler->read_hdf5(data_container.data.data(), c, vec_size, c);
    // Componentwise ordering (stride = 1, offset = blocksize)
    else
      for (auto c = 0; c < vec_size; c++)
        hdf5_handler->read_hdf5(data_container.data.data(), c * grid_elements_scalar, 1, c);

    return data_container;
  }

  // Opens a new file and reads its content into a container with metainfo (preferred).
  template <typename T>
  jaySrc<T> hdf5_read(
    std::string              filepath,
    std::vector<std::string> datasets,
    Order                    ordering = Order::VectorFirst
  )
  {
    hdf5_open(filepath, datasets);
    return hdf5_read<T>(ordering);
  }
  
  // Reads only a range of the data. The range must be explicitly given for each dimension.
  template <typename T>
  jaySrc<T> hdf5_read_subset(
    std::vector<std::size_t> ds_ranges,
    std::vector<std::size_t> ds_offsets,
    Order ordering = Order::VectorFirst
  )
  {
    const auto grid     = hdf5_handler->get_grid_fixsize();
    const auto grid_dim = hdf5_handler->get_grid_dim();
    const auto vec_size = hdf5_handler->get_vec_len();

    const auto  dim    = hdf5_handler->get_grid_dim();
    const auto range_x = (dim >= 1) ? ds_ranges[0] : 1;
    const auto range_y = (dim >= 2) ? ds_ranges[1] : 1;
    const auto range_z = (dim >= 3) ? ds_ranges[2] : 1;
    const auto range_t = (dim >= 4) ? ds_ranges[3] : 1;
    // Grid elements per component
    const auto  range_elements_scalar = range_t * range_z * range_y * range_x;

    jaySrc<T> data_container{ std::vector<T>(range_elements_scalar * vec_size), ds_ranges, grid_dim, vec_size, ordering };

    // Vectorlike ordering (stride = vec_size, offset = component_id)
    if (ordering == Order::VectorFirst)
      for (auto c = 0; c < vec_size; c++)
        hdf5_handler->read_hdf5_subset(data_container.data.data(), c, vec_size, c, ds_ranges, ds_offsets);
    // Componentwise ordering (stride = 1, offset = blocksize)
    else
      for (auto c = 0; c < vec_size; c++)
        hdf5_handler->read_hdf5_subset(data_container.data.data(), c * range_elements_scalar, 1, c, ds_ranges, ds_offsets);

    return data_container;
  }
  
  // Reads file into the provided pointer (no metadata, not preferred)
  template <typename T>
  void hdf5_read_into(
    T* data_addr,
    Order ordering = Order::VectorFirst
  )
  {
    const auto grid = hdf5_handler->get_grid_fixsize();
    const auto vec_size = hdf5_handler->get_vec_len();
    const auto grid_elements_scalar = grid[0] * grid[1] * grid[2] * grid[3];

    // Vectorlike ordering (stride = vec_size, offset = component_id)
    if (ordering == Order::VectorFirst)
      for (auto c = 0; c < vec_size; c++)
        hdf5_handler->read_hdf5(data_addr, c, vec_size, c);
    // Componentwise ordering (stride = 1, offset = blocksize)
    else
      for (auto c = 0; c < vec_size; c++)
        hdf5_handler->read_hdf5(data_addr, c, 1, c * grid_elements_scalar);
  }

  // Reads file into the provided glm::vec-pointer (not preferred, no metadata)
  template <typename T, int i>
  void hdf5_read_into(
    glm::vec<i, T>* data_addr,
    Order           ordering
  )
  {
    // Only Vectorlike ordering for glm::vec
    hdf5_handler->read_hdf5(data_addr)
  }

  std::vector<std::size_t> hdf5_get_grid(bool desc_order = false)
  {
    return hdf5_handler->get_grid(desc_order);
  }

  std::vector<std::size_t> hdf5_get_grid_fixsize(bool desc_order = false, std::size_t fillvalue = 1)
  {
    return hdf5_handler->get_grid_fixsize(desc_order, fillvalue);
  }

  std::size_t hdf5_get_vec_len()
  {
    return hdf5_handler->get_vec_len();
  }

  template <typename T>
  jaySrc<T> hdf5_read_into(
    std::string              filepath,
    std::vector<std::string> datasets,
    T*                       data_addr,
    Order                    ordering = Order::VectorFirst
  )
  {
    hdf5_open(filepath, datasets);
    return hdf5_read_into<T>(data_addr, ordering);
  }

  template <typename T, int i>
  jaySrc<T> hdf5_read_into(
    std::string              filepath,
    std::vector<std::string> datasets,
    glm::vec<i, T>*          data_addr,
    Order                    ordering = Order::VectorFirst
  )
  {
    hdf5_open(filepath, datasets);
    return hdf5_read_into<T, i>(data_addr, ordering);
  }

  /* =============================================================

                      ASTC I/O Operations

    ============================================================= */

  void astc_store(const jayComp<astc_datatype>& comp_imgs, std::string filepath, bool newFile);
  void astc_store(const jayComp<astc_datatype>& comp_imgs, std::size_t img_offset, std::string filepath, bool newFile);
  void astc_store(const jayComp<astc_datatype>& comp_imgs, std::size_t img_offset, std::size_t img_count, std::string filepath, bool newFile);

  jayComp<astc_datatype> astc_read(std::string filename);

  std::vector<jayComp<astc_datatype>> astc_read_multiple(const std::vector<std::string>& filenames);


  /* =============================================================

                      Image I/O Operations

    ============================================================= */
};

}
#endif