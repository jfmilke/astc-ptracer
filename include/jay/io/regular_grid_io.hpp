#ifndef JAY_GRID_IO_HPP
#define JAY_GRID_IO_HPP

#include <jay/types/regular_grid.hpp>
#include <jay/io/data_io.hpp>

#include <highfive/H5File.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>

#include <jay/export.hpp>

namespace jay
{
class JAY_EXPORT grid_io
{
public:
  /* Just for testing stuff */
  static void test();

  /* Reads a scalarfield or single vectorcomponent of a vectorfield of a hdf5 compliant file a grid-struct */
  /* Grid must be 1D - 4D and vector components must be given as seperate datasets */
  template <typename _element_type, std::size_t _dimensions>
  static jay::regular_grid<_element_type, _dimensions> loadGrid(
    const std::string filepath,
    const char* dataset_name
  )
  {
    HighFive::File hdf5_file(filepath, HighFive::File::ReadOnly);

    HighFive::DataSet dataset      = hdf5_file.getDataSet(dataset_name);
    std::vector<size_t> grid_shape = dataset.getDimensions();
    unsigned int grid_dim          = grid_shape.size();

    /* Only accept 1D - 4D grids */
    if (grid_dim > 4 || _dimensions > 4)
    { 
      throw GridException();
    }

    /* Every possible dimension has at least magnitude 1 */
    size_t grid_t = (grid_dim >= 4) ? grid_shape[grid_dim - 4] : 1;
    size_t grid_z = (grid_dim >= 3) ? grid_shape[grid_dim - 3] : 1;
    size_t grid_y = (grid_dim >= 2) ? grid_shape[grid_dim - 2] : 1;
    size_t grid_x = (grid_dim >= 1) ? grid_shape[grid_dim - 1] : 1;

    jay::regular_grid<_element_type, _dimensions> grid(grid_shape);

    //boost::multi_array<_element_type, _dimensions> data(grid_shape);
    dataset.read(grid.data);

    /* Create and initialize the grid */
    //jay::regular_grid<_element_type, _dimensions> grid = { data };
    grid.set_offset ({ 0.f, 0.f, 0.f });
    grid.set_spacing({ 1.f, 1.f, 1.f });

    return grid;
  }
  

protected:
  struct GridException : public std::exception {
    const char* what() const throw () {
      return "You can only select datasets with grid-dimension of 1 to 4.";
    }
  };
  struct EncodingException : public std::exception {
    const char* what() const throw () {
      return "You selected an encoding that doesn't exactly meet the number of colorchannels. Only L+A supports for 1 or 2 components per pixel.";
    }
  };
  struct VectorEncodingException : public std::exception {
    const char* what() const throw () {
      return "You cannot choose vectorlike encoding when your dataset is scalar and cannot addres more than 1 vectorcomponent.";
    }
  };
  struct BadCodeForFloatException : public std::exception {
    const char* what() const throw () {
      return "Well, I assumed float to be 32bits long. Seems it isn't and I need to update code :(";
    }
  };
};
}
#endif