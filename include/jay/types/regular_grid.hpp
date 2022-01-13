#ifndef JAY_TYPES_REGULAR_GRID_HPP
#define JAY_TYPES_REGULAR_GRID_HPP

#include <cmath>
#include <cstddef>
#include <vector>

#include <boost/multi_array.hpp>
#include <glm/glm.hpp>

#include <jay/utility/permute_for.hpp>

namespace jay
{
template <typename _element_type, std::size_t _dimensions>
struct regular_grid
{
  using element_type   = _element_type;
  using domain_type    = glm::vec<_dimensions, float>;
  using index_type     = glm::vec<_dimensions, std::size_t>;
  using container_type = boost::multi_array<element_type, _dimensions>;
  
  static constexpr std::size_t dimensions = _dimensions;

  /* Initialize default */
  regular_grid() = default;
  /* Initialize with dataset */
  regular_grid(container_type grid_data) :
    data{ grid_data }
  {
  }
  /* Initialize with dimensions of dataset */
  regular_grid(std::vector<std::size_t> extents) :
    data{ extents }
  {
    domain_type offset_t;
    domain_type spacing_t;
    for (size_t i = 0; i < _dimensions; i++)
    {
      offset_t[i] = 0.f;
      spacing_t[i] = 1.f;
    }
    this->set_offset(offset_t);
    this->set_spacing(spacing_t);
  }

  /* Return if a certain position is within the defined grid. */
  // BUT: Why does it return false if position is on the outer boundary of data?
  bool          contains   (const domain_type& position) const
  {
    for (std::size_t i = 0; i < dimensions; ++i)
    {
      const auto subscript = std::floor((position[i] - offset[i]) / spacing[i]);
      if (std::int64_t(0) > std::int64_t(subscript) || std::size_t(subscript) >= data.shape()[i] - 1)
        return false;
    }
    return true;
  }
  /* Returns a number that is interpolated between the nearest prior node and the explicit successor within the defined grid. */
  // BUT: You can't use interpolate on a position on the outer boundary
  //      Only works for scalarwise arranged data, element_type must not be a vector => a grid per vectorcomponent?
  element_type  interpolate(const domain_type& position) const
  {
    domain_type weights    ;
    index_type  start_index;
    index_type  end_index  ;
    index_type  increment  ;

    for (std::size_t i = 0; i < dimensions; ++i)
    {
      weights    [i] = std::fmod ((position[i] - offset[i]) , spacing[i]) / spacing[i];
      start_index[i] = std::floor((position[i] - offset[i]) / spacing[i]);
      end_index  [i] = start_index[i] + 2;
      increment  [i] = 1;
    }

    std::vector<element_type> intermediates;
    intermediates.reserve(std::pow(2, dimensions));
    permute_for<index_type>([&] (const index_type& index)
    {
      intermediates.push_back(data(*reinterpret_cast<const std::array<std::size_t, 3>*>(&index)));
    }, start_index, end_index, increment);

    for (std::int64_t i = dimensions - 1; i >= 0; --i)
      for (std::size_t j = 0; j < std::pow(2, i); ++j)
        intermediates[j] = (1.f - weights[i]) * intermediates[2 * j] + weights[i] * intermediates[2 * j + 1];
    return intermediates[0];
  }
  /* Sets the spacing and adjusts the size of the grid relative to spacing and data */
  void set_spacing(const domain_type& spacing)
  {
    this->spacing = spacing;

    const size_t * shape = this->data.shape();
    for (size_t l = 0; l < _dimensions; l++)
      this->size[l] = shape[l] * this->spacing[l];
  }
  /* Simple setter & getter (if needed at all) */
  void set_offset(const domain_type& offset)
  {
    this->offset = offset;
  }
  domain_type get_size()
  {
    return this->size;
  }

  /*
   * data:    Holds the original data
   * offset:  Offset to camera? Don't know
   * size:    Shape of the original data including the spacing? No reference yet
   * spacing: How far away each node of original data is from its neighbours
   */
  container_type data    {};
  domain_type    offset  {};
  domain_type    size    {};
  domain_type    spacing {};
};
}

#endif