#include <jay/analysis/distance_measure.hpp>

namespace jay
{
  std::size_t get_count_quads_per_seed(std::size_t elements_per_seed)
  {
    return elements_per_seed - 1;
  }

  std::vector<double> calculate_velocity_magnitude_difference(std::vector<float>& b_velo0, std::vector<float>& b_velo1, std::size_t seeds)
  {
    auto b_size = b_velo0.size();
    auto vectors = b_size / 4;

    std::vector<double> vector_mag_diffs(vectors);

    for (auto v = 0; v < vectors; v++)
    {
      glm::dvec4 vel0 = (glm::dvec4) *reinterpret_cast<glm::vec4*>(&b_velo0[4 * v]);
      glm::dvec4 vel1 = (glm::dvec4) *reinterpret_cast<glm::vec4*>(&b_velo1[4 * v]);

      vector_mag_diffs[v] = glm::length(vel0) - glm::length(vel1);
    }

    return vector_mag_diffs;
  }

  std::vector<double> calculate_seed_velocity_magnitude_difference(std::vector<float>& b_velo0, std::vector<float>& b_velo1, std::size_t seeds)
  {
    auto b_size = b_velo0.size();
    auto vectors = b_size / 4;
    auto vectors_per_seed = vectors / seeds;

    std::vector<double> vector_mag_diffs(seeds);
    std::vector<double> s_diff(vectors_per_seed);

    for (auto s = 0; s < seeds; s++)
    {
      for (auto v = 0; v < vectors_per_seed; v++)
      {
        glm::dvec4 vel0 = (glm::dvec4) *reinterpret_cast<glm::vec4*>(&b_velo0[4 * v]);
        glm::dvec4 vel1 = (glm::dvec4) *reinterpret_cast<glm::vec4*>(&b_velo1[4 * v]);

        s_diff[v] = glm::length(vel0) - glm::length(vel1) / vectors_per_seed;
      }
      vector_mag_diffs[s] = std::accumulate(s_diff.begin(), s_diff.end(), 0.0);
    }

    return vector_mag_diffs;
  }

  std::vector<double> calculate_seed_velocity_angle_difference(std::vector<float>& b_velo0, std::vector<float>& b_velo1, std::size_t seeds, bool degrees)
  {
    auto b_size = b_velo0.size();
    auto vectors = b_size / 4;
    auto vectors_per_seed = vectors / seeds;

    std::vector<double> vector_angle_diffs(vectors);
    std::vector<double> s_diff(vectors_per_seed);

    for (auto s = 0; s < seeds; s++)
    {
      for (auto v = 0; v < vectors_per_seed; v++)
      {
        glm::dvec4 vel0 = (glm::dvec4) * reinterpret_cast<glm::vec4*>(&b_velo0[4 * v]);
        glm::dvec4 vel1 = (glm::dvec4) * reinterpret_cast<glm::vec4*>(&b_velo1[4 * v]);

        double dot = glm::dot(vel0, vel1);
        double alpha = dot / (glm::length(vel0) * glm::length(vel1));
        s_diff[v] = (degrees) ? acos(alpha) / vectors_per_seed : alpha / vectors_per_seed;
      }
      vector_angle_diffs[s] = std::accumulate(s_diff.begin(), s_diff.end(), 0.0);
    }

    return vector_angle_diffs;
  }

  std::vector<double> calculate_velocity_angle_difference(std::vector<float>& b_velo0, std::vector<float>& b_velo1, std::size_t seeds, bool degrees)
  {
    auto b_size = b_velo0.size();
    auto vectors = b_size / 4;

    std::vector<double> vector_angle_diffs(vectors);

    for (auto v = 0; v < vectors; v++)
    {
      glm::dvec4 vel0 = (glm::dvec4) * reinterpret_cast<glm::vec4*>(&b_velo0[4 * v]);
      glm::dvec4 vel1 = (glm::dvec4) * reinterpret_cast<glm::vec4*>(&b_velo1[4 * v]);

      double dot = glm::dot(vel0, vel1);
      double alpha = dot / (glm::length(vel0) * glm::length(vel1));
      vector_angle_diffs[v] = (degrees) ? acos(alpha) : alpha;
    }

    return vector_angle_diffs;
  }
  
  std::vector<double> calculate_vertex_distance(std::vector<float>& b_pos0, std::vector<float>& b_pos1, std::size_t seeds)
  {
    auto b_size = b_pos0.size();
    auto vertices = b_size / 4;

    std::vector<double> vertex_distances(vertices);

    for (auto v = 0; v < vertices; v++)
    {
      glm::dvec4 pos0 = (glm::dvec4) * reinterpret_cast<glm::vec4*>(&b_pos0[4 * v]);
      glm::dvec4 pos1 = (glm::dvec4) *reinterpret_cast<glm::vec4*>(&b_pos1[4 * v]);

      vertex_distances[v] = glm::distance(pos0, pos1);
    }

    return vertex_distances;
  }

  std::vector<double> calculate_seed_distance(std::vector<float>& b_pos0, std::vector<float>& b_pos1, std::size_t seeds)
  {
    auto b_size = b_pos0.size();
    auto vertices = b_size / 4;
    auto vertices_per_seed = vertices / seeds;

    std::vector<double> seed_distances(seeds);
    std::vector<double> s_dist(vertices_per_seed);

    for (auto s = 0; s < seeds; s++)
    {
      for (auto v = 0; v < vertices_per_seed; v++)
      {
        auto index = s * vertices_per_seed + v;
        glm::dvec4 pos0 = (glm::dvec4) * reinterpret_cast<glm::vec4*>(&b_pos0[4 * index]);
        glm::dvec4 pos1 = (glm::dvec4) *reinterpret_cast<glm::vec4*>(&b_pos1[4 * index]);

        s_dist[v] = glm::distance(pos0, pos1) / vertices_per_seed;
      }
      seed_distances[s] = std::accumulate(s_dist.begin(), s_dist.end(), 0.0);
    }

    return seed_distances;
  }

  std::vector<double> calculate_quad_area(std::vector<float>& b_pos0, std::vector<float>& b_pos1, std::size_t seeds)
  {
    auto b_size = b_pos0.size();
    auto vertices = b_size / 4;
    auto vertices_per_seed = vertices / seeds;
    auto quads_per_seed = get_count_quads_per_seed(vertices_per_seed); // vertices_per_seed - 1

    std::vector<double> quad_areas(quads_per_seed * seeds);

    // v3__v2
    //  |__|
    // v1  v0
    for (auto s = 0; s < seeds; s++)
      for (auto q = 0; q < quads_per_seed; q++)
      {
        auto index = s * quads_per_seed + q;

        auto v0_id = s * vertices_per_seed + q;
        auto v1_id = s * vertices_per_seed + q + vertices;
        auto v2_id = s * vertices_per_seed + q + 1;
        auto v3_id = s * vertices_per_seed + q + vertices + 1;

        glm::dvec4 v0 = (glm::dvec4) * reinterpret_cast<glm::vec4*>(&b_pos0[4 * v0_id]);
        glm::dvec4 v1 = (glm::dvec4) *reinterpret_cast<glm::vec4*>(&b_pos1[4 * v1_id]);
        glm::dvec4 v2 = (glm::dvec4) * reinterpret_cast<glm::vec4*>(&b_pos0[4 * v2_id]);
        glm::dvec4 v3 = (glm::dvec4) *reinterpret_cast<glm::vec4*>(&b_pos1[4 * v3_id]);

        glm::dvec3 V0V3 = glm::dvec3(v3 - v0);
        glm::dvec3 V2V1 = glm::dvec3(v1 - v2);
        glm::dvec3 cross = glm::cross(V0V3, V2V1);

        double local_area = 0.5 * glm::length(cross);

        quad_areas[index] = local_area;
      }

    return quad_areas;
  }

  std::vector<double> calculate_seed_area(std::vector<float>& b_pos0, std::vector<float>& b_pos1, std::size_t seeds)
  {
    auto b_size = b_pos0.size();
    auto vertices = b_size / 4;
    auto vertices_per_seed = vertices / seeds;
    auto quads_per_seed = get_count_quads_per_seed(vertices_per_seed); // vertices_per_seed - 1

    std::vector<double> seed_areas(seeds);
    std::vector<double> s_area(quads_per_seed);

    // v3__v2
    //  |__|
    // v1  v0

    for (auto s = 0; s < seeds; s++)
    {
      for (auto q = 0; q < quads_per_seed; q++)
      {
        auto v0_id= s * vertices_per_seed + q;
        auto v1_id= s * vertices_per_seed + q;
        auto v2_id= s * vertices_per_seed + q + 1;
        auto v3_id= s * vertices_per_seed + q + 1;

        glm::dvec4 v0 = (glm::dvec4) * reinterpret_cast<glm::vec4*>(&b_pos0[4 * v0_id]);
        glm::dvec4 v1 = (glm::dvec4) *reinterpret_cast<glm::vec4*>(&b_pos1[4 * v1_id]);
        glm::dvec4 v2 = (glm::dvec4) * reinterpret_cast<glm::vec4*>(&b_pos0[4 * v2_id]);
        glm::dvec4 v3 = (glm::dvec4) *reinterpret_cast<glm::vec4*>(&b_pos1[4 * v3_id]);

        glm::dvec3 V0V3 = glm::dvec3(v3 - v0);
        glm::dvec3 V2V1 = glm::dvec3(v1 - v2);
        glm::dvec3 cross = glm::cross(V0V3, V2V1);

        double local_area = 0.5 * glm::length(cross);

        s_area[q] = local_area;
      }
      seed_areas[s] = std::accumulate(s_area.begin(), s_area.end(), 0.0);
    }

    return seed_areas;
  }

  std::size_t get_strided_element_count(std::size_t elements, long int stride, std::size_t offset)
  {
    return floor((elements - 1 - offset + stride) / stride);
  }
}