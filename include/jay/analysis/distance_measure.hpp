#ifndef JAY_ANALYSIS_DISTANCE_HPP
#define JAY_ANALYSIS_DISTANCE_HPP
#include <vector>
#include <algorithm>
#include <numeric>
#include <glm/glm.hpp>
#include <jay/export.hpp>

namespace jay
{
  std::vector<double> calculate_velocity_magnitude_difference(std::vector<float>& b_velo0, std::vector<float>& b_velo1, std::size_t seeds);
  std::vector<double> calculate_seed_velocity_magnitude_difference(std::vector<float>& b_velo0, std::vector<float>& b_velo1, std::size_t seeds);

  // Returns vector of size b_pos0.size()/4, assuming all elements are vec4.
  // Each element in vector denominates the distance between the corresponding vertex of both input vectors
  std::vector<double> calculate_vertex_distance(std::vector<float>& b_pos0, std::vector<float>& b_pos1, std::size_t seeds);

  // Returns vector of size seeds, assuming all elements are vec4.
  // Each element in vector denominates the averaged distance between the corresponding seed of both input vectors
  std::vector<double> calculate_seed_distance(std::vector<float>& b_pos0, std::vector<float>& b_pos1, std::size_t seeds);

  // Returns a vector of size b_pos0.size/4 - seeds, assuming all elements are vec4.
  // For construction of a quad 2 elements of each seed are used, reducing the size of quads by one for each seed.
  // Each element in vector denominates the area of a quad. The quads are ordered by seed, having vertices_per_seed-1 quads for each seed.
  std::vector<double> calculate_quad_area(std::vector<float>& b_pos0, std::vector<float>& b_pos1, std::size_t seeds);

  // Returns a vector of size seeds, assuming all elements are vec4.
  // Each element in vector denominates the area of a quad. The quads are ordered by seed, having vertices_per_seed-1 quads for each seed.
  std::vector<double> calculate_seed_area(std::vector<float>& b_pos0, std::vector<float>& b_pos1, std::size_t seeds);

  std::size_t get_count_quads_per_seed(std::size_t elements_per_seed);

  // Calculates arithmetic mean for a single dimension.
  // element_count must match the number of distinct elements from which the mean should be calculated.
  // stride & offset allow to calculate the mean of a higher dimensional datatype seperately for each component.
  // mean = 1/n * sum(x_i)
  std::size_t get_strided_element_count(std::size_t elements, long int stride, std::size_t offset);
}

#endif