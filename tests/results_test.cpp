#include <catch2/catch.hpp>

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>

#include <jay/api.hpp>

void binVector(std::vector<float>& vec, int bins, float bin_size, std::string filepath)
{
  std::vector<int> arr;
  std::vector<std::string> desc;
  arr.resize(bins + 1);

  for (const auto& el : vec)
  {
    if (el > (bins * bin_size))
      arr[bins] += 1;
    else if (el < 0)
    {
      std::cout << "Error: Negative binning" << std::endl;
      return;
    }
    else
      arr[(int)(el / bin_size)] += 1;
  }

  for (auto i = 0; i < bins; i++)
  {
    std::ostringstream de;
    de << std::setprecision(4);
    de << "[" << bin_size * i << ", "<< i * bin_size + bin_size <<  ")";
    //std::string d = "[" + std::to_string(bin_size * i) + ", " + std::to_string(2 * i + bin_size) + ")";
    desc.push_back(de.str());
    std::cout << de.str() << ":  " << arr[i] << std::endl;
  }

  std::ostringstream de;
  de << std::setprecision(2);
  de << "[>=" << bin_size * bins << "]";
  //std::string d = "[>=" + std::to_string(bin_size * bins) + "]";
  desc.push_back(de.str());
  std::cout << de.str() << ":  " << arr[bins] << std::endl << std::endl;

  std::setprecision(6);

  jay::data_io::store_descriptive_text("Seed Distance", desc, arr, filepath, 0, true);
}

void vertex_distance(std::vector<float>& a, std::vector<float>& b, std::size_t seeds, std::string filename)
{
  std::string filepath = "../files/exports/";

  auto vertices = a.size() / 4;          // Vertex Count
  auto vertices_per_seed = vertices / seeds;  // Steps per Seed

  // Distances of vertices
  std::vector<float> vertex_dists;
  // Distances of individual seeds
  std::vector<float> seed_dists;
  std::vector<std::pair<float, float>> seed_minmax;
  // Distances of the whole dataset
  double dataset_dist = 0;

  float gmin = INFINITY;
  float gmax = -INFINITY;

  // Cycle all seeds
  for (auto seed = 0; seed < seeds; seed++)
  {
    double seed_dist = 0;
    float min = INFINITY;
    float max = -INFINITY;

    // Cycle the whole trace
    for (auto vertex = 0; vertex < vertices_per_seed; vertex++)
    {
      // Squared distance
      glm::vec4 a_vertex = *reinterpret_cast<glm::vec4*>(&a[4 * (seed * vertices_per_seed + vertex)]);
      glm::vec4 b_vertex = *reinterpret_cast<glm::vec4*>(&b[4 * (seed * vertices_per_seed + vertex)]);
      auto distance = glm::distance(a_vertex, b_vertex);
      vertex_dists.push_back(distance);

      seed_dist += distance;

      if (distance < min)
        min = distance;
      if (distance > max)
        max = distance;
    }
    seed_minmax.push_back({ min, max });

    dataset_dist += seed_dist / seeds;
    seed_dists.push_back(seed_dist / vertices_per_seed);

    if (min < gmin)
      gmin = min;
    if (max > gmax)
      gmax = max;
  }

  // Account for uniform weight
  dataset_dist /= (double)seeds;

  //jay::data_io::store_text("Vertex Distance", vertex_dists, filepath + filename + "vertex_dist.txt", 5, true);
  jay::data_io::store_text("Seed Distance", seed_dists, filepath + filename + "_seed_dist.txt", 5, true);
  jay::data_io::store_text("Dataset Distance", dataset_dist, filepath + filename + "_data_dist.txt", 5, true);

  binVector(vertex_dists, 20, 1, filepath + filename + "_binned.txt");

  std::cout << "Avg Dist per seed:" << std::endl;
  std::cout << "   " << dataset_dist << std::endl;

  std::cout << "Maximum vertex distance" << std::endl;
  std::cout << "   " << gmax << std::endl;

}

void vector_difference(std::vector<float>& a, std::vector<float>& b, std::size_t seeds)
{
  std::string filepath = "../files/exports/";
  std::string filename = "velocity_diffs.txt";

  glm::vec4* data_a = reinterpret_cast<glm::vec4*>(a.data());
  glm::vec4* data_b = reinterpret_cast<glm::vec4*>(a.data());
  auto buffer_size = a.size() / 4;          // Vertex Count
  auto trace_length = buffer_size / seeds;  // Steps per Seed

  // Distances of individual seeds
  std::vector<double> seed_diffs;
  // Distances of the whole dataset
  double dataset_diff= 0;

  // Cycle all seeds
  for (auto seed = 0; seed < seeds; seed++)
  {
    double length_diff = 0;

    // Cycle the whole trace
    for (auto vertex = 0; vertex < trace_length; vertex++)
    {
      // Squared distance
      double a_length = glm::length(*reinterpret_cast<glm::vec4*>(&a[4 * (seed * trace_length + vertex)]));
      double b_length = glm::length(*reinterpret_cast<glm::vec4*>(&b[4 * (seed * trace_length + vertex)]));
      length_diff += ((a_length - b_length) * (a_length - b_length)) / trace_length;
      dataset_diff += length_diff;
    }
    // Account for uniform weight
    seed_diffs.push_back(length_diff);
  }
  // Account for uniform weight
  dataset_diff /= (double)seeds;

  jay::data_io::store_text("Velocity Difference", seed_diffs, filepath + filename, 10, true);

  std::cout << "Distance of whole dataset:" << std::endl;
  std::cout << "   " << dataset_diff << std::endl;
}


void rms(std::vector<float>& a, std::vector<float>& b, std::size_t seeds, std::string filename)
{
  std::string filepath = "../files/exports/";

  auto vertices = a.size() / 4;          // Vertex Count
  auto vertices_per_seed = vertices / seeds;  // Steps per Seed

  std::vector<float> seed_rms;
  double global_rms = 0.0;

  float gmin = INFINITY;
  float gmax = -INFINITY;

  // Cycle all seeds
  for (auto seed = 0; seed < seeds; seed++)
  {

    double rms = 0.0;
    // Cycle the whole trace
    for (auto vertex = 0; vertex < vertices_per_seed; vertex++)
    {
      // Squared distance
      glm::vec4 a_vertex = *reinterpret_cast<glm::vec4*>(&a[4 * (seed * vertices_per_seed + vertex)]);
      glm::vec4 b_vertex = *reinterpret_cast<glm::vec4*>(&b[4 * (seed * vertices_per_seed + vertex)]);
      auto distance = glm::distance(a_vertex, b_vertex);
      rms += distance * distance;
    }

    global_rms += (rms / vertices_per_seed);
    seed_rms.push_back(std::sqrt(rms / vertices_per_seed));
  }

  global_rms = std::sqrt(global_rms);

  std::cout << "Global RMS: " << global_rms << std::endl;

  binVector(seed_rms, 10, 20, filepath + filename + "_rms.txt");
}

std::vector<int> getValidRanges(std::vector<float>& vertices, std::size_t seeds, int& steps_skipped, int& seeds_skipped)
{
  auto vertex_count = vertices.size() / 4;
  auto vertices_per_seed = vertex_count / seeds;
  auto* vert_it = reinterpret_cast<glm::vec4*>(vertices.data());

  std::vector<int> valid_ranges(seeds);
  
  for (auto s = 0; s < seeds; s++)
    for (auto v = 0; v < vertices_per_seed; v++)
    {
      if (v > 0)
        if ((*vert_it == *(vert_it - 1)))
        {
          seeds_skipped++;
          steps_skipped += vertices_per_seed - v;
          vert_it += (vertices_per_seed - v);
          break;
        }

      valid_ranges[s] += 1;

      vert_it++;
    }

  return valid_ranges;
}

std::vector<int> getValidRanges(std::vector<float>& ds_A, std::vector<float>& ds_B, std::size_t seeds, int& steps_skipped, int& seeds_skipped)
{
  auto vertex_count = ds_A.size() / 4;
  auto vertices_per_seed = vertex_count / seeds;
  auto* dsA_it = reinterpret_cast<glm::vec4*>(ds_A.data());
  auto* dsB_it = reinterpret_cast<glm::vec4*>(ds_B.data());

  std::vector<int> valid_ranges(seeds);

  for (auto s = 0; s < seeds; s++)
    for (auto v = 0; v < vertices_per_seed; v++)
    {
      if (v > 0)
        if ((*dsA_it == *(dsA_it - 1)) || (*dsB_it == *(dsB_it - 1)))
        {
          dsA_it += (vertices_per_seed - v);
          dsB_it += (vertices_per_seed - v);
          seeds_skipped++;
          steps_skipped += vertices_per_seed - v;
          break;
        }

      valid_ranges[s] += 1;

      dsA_it++;
      dsB_it++;
    }

  return valid_ranges;
}

std::vector<double> AVG_distance_per_seed(std::vector<float>& vertices_dsA, std::vector<float>& vertices_dsB, std::vector<int>& valid_ranges, std::size_t seeds)
{
  auto vertex_count = vertices_dsA.size() / 4;
  auto vertices_per_seed = vertex_count / seeds;
  auto* vertA_it = reinterpret_cast<glm::vec4*>(vertices_dsA.data());
  auto* vertB_it = reinterpret_cast<glm::vec4*>(vertices_dsB.data());

  std::vector<double> vertex_dists;
  std::vector<double> avg_seed_dists;

  for (auto s = 0; s < seeds; s++)
  {
    for (auto v = 0; v < vertices_per_seed; v++)
    {
      if (v >= valid_ranges[s])
      {
        vertA_it++;
        vertB_it++;
        continue;
      }

      const auto& a_vertex = *vertA_it;
      const auto& b_vertex = *vertB_it;
      auto distance = glm::distance(a_vertex, b_vertex);
      vertex_dists.push_back(distance);

      vertA_it++;
      vertB_it++;
    }

    avg_seed_dists.push_back(boost::math::statistics::mean(vertex_dists));
    vertex_dists.clear();
  }

  return avg_seed_dists;
}

double AVG_distance_per_vertex(std::vector<float>& vertices_dsA, std::vector<float>& vertices_dsB, std::vector<int>& valid_ranges, std::size_t seeds)
{
  auto vertex_count = vertices_dsA.size() / 4;
  auto vertices_per_seed = vertex_count / seeds;
  auto* vertA_it = reinterpret_cast<glm::vec4*>(vertices_dsA.data());
  auto* vertB_it = reinterpret_cast<glm::vec4*>(vertices_dsB.data());

  std::vector<double> vertex_dists;

  for (auto s = 0; s < seeds; s++)
  {
    for (auto v = 0; v < vertices_per_seed; v++)
    {
      if (v >= valid_ranges[s])
      {
        vertA_it++;
        vertB_it++;
        continue;
      }

      const auto& a_vertex = *vertA_it;
      const auto& b_vertex = *vertB_it;
      auto distance = glm::distance(a_vertex, b_vertex);
      vertex_dists.push_back(distance);

      vertA_it++;
      vertB_it++;
    }
  }

  return boost::math::statistics::mean(vertex_dists);
}

std::vector<double> ABS_distance_per_vertex(std::vector<float>& vertices_dsA, std::vector<float>& vertices_dsB, std::vector<int>& valid_ranges, std::size_t seeds)
{
  auto vertex_count = vertices_dsA.size() / 4;
  auto vertices_per_seed = vertex_count / seeds;
  auto* vertA_it = reinterpret_cast<glm::vec4*>(vertices_dsA.data());
  auto* vertB_it = reinterpret_cast<glm::vec4*>(vertices_dsB.data());

  std::vector<double> vertex_dists;

  for (auto s = 0; s < seeds; s++)
  {
    for (auto v = 0; v < vertices_per_seed; v++)
    {
      if (v >= valid_ranges[s])
      {
        vertA_it++;
        vertB_it++;
        continue;
      }

      const auto& a_vertex = *vertA_it;
      const auto& b_vertex = *vertB_it;
      auto distance = glm::distance(a_vertex, b_vertex);
      vertex_dists.push_back(distance);

      vertA_it++;
      vertB_it++;
    }
  }

  return vertex_dists;
}

std::vector<double> MAX_distance_per_seed(std::vector<float>& vertices_dsA, std::vector<float>& vertices_dsB, std::vector<int>& valid_ranges, std::size_t seeds)
{
  auto vertex_count = vertices_dsA.size() / 4;
  auto vertices_per_seed = vertex_count / seeds;
  auto* vertA_it = reinterpret_cast<glm::vec4*>(vertices_dsA.data());
  auto* vertB_it = reinterpret_cast<glm::vec4*>(vertices_dsB.data());

  std::vector<double> max_seed_dists;

  for (auto s = 0; s < seeds; s++)
  {
    double max = -INFINITY;
    double min = INFINITY;

    for (auto v = 0; v < vertices_per_seed; v++)
    {
      if (v >= valid_ranges[s])
      {
        vertA_it++;
        vertB_it++;
        continue;
      }

      const auto& a_vertex = *vertA_it;
      const auto& b_vertex = *vertB_it;
      auto distance = glm::distance(a_vertex, b_vertex);
      
      if (distance > max)
        max = distance;

      if (distance < min)
        min = distance;

      vertA_it++;
      vertB_it++;
    }

    max_seed_dists.push_back(max);
  }

  return max_seed_dists;
}


std::vector<double> AVG_velocity_difference_per_seed(std::vector<float>& velos_dsA, std::vector<float>& velos_dsB, std::vector<int>& valid_ranges, std::size_t seeds)
{
  auto vertex_count = velos_dsA.size() / 4;
  auto vertices_per_seed = vertex_count / seeds;
  auto* veloA_it = reinterpret_cast<glm::vec4*>(velos_dsA.data());
  auto* veloB_it = reinterpret_cast<glm::vec4*>(velos_dsB.data());

  std::vector<double> vertex_velos;
  std::vector<double> avg_seed_dists;

  for (auto s = 0; s < seeds; s++)
  {
    for (auto v = 0; v < vertices_per_seed; v++)
    {
      if (v >= valid_ranges[s])
      {
        veloA_it++;
        veloB_it++;
        continue;
      }

      const auto& a_velo = *veloA_it;
      const auto& b_velo = *veloB_it;
      auto velocity_diff = glm::abs(glm::length(a_velo) - glm::length(b_velo));
      vertex_velos.push_back(velocity_diff);

      veloA_it++;
      veloB_it++;
    }

    avg_seed_dists.push_back(boost::math::statistics::mean(vertex_velos));
    vertex_velos.clear();
  }

  return avg_seed_dists;
}

double AVG_velocity_difference_per_vertex(std::vector<float>& velos_dsA, std::vector<float>& velos_dsB, std::vector<int>& valid_ranges, std::size_t seeds)
{
  auto vertex_count = velos_dsA.size() / 4;
  auto vertices_per_seed = vertex_count / seeds;
  auto* veloA_it = reinterpret_cast<glm::vec4*>(velos_dsA.data());
  auto* veloB_it = reinterpret_cast<glm::vec4*>(velos_dsB.data());

  std::vector<double> vertex_velos;
  std::vector<double> avg_seed_dists;

  for (auto s = 0; s < seeds; s++)
  {
    for (auto v = 0; v < vertices_per_seed; v++)
    {
      if (v >= valid_ranges[s])
      {
        veloA_it++;
        veloB_it++;
        continue;
      }

      const auto& a_velo = *veloA_it;
      const auto& b_velo = *veloB_it;
      auto velocity_diff = glm::abs(glm::length(a_velo) - glm::length(b_velo));
      vertex_velos.push_back(velocity_diff);

      veloA_it++;
      veloB_it++;
    }
  }

  return boost::math::statistics::mean(vertex_velos);
}

std::vector<double> ABS_velocity_diff_per_vertex(std::vector<float>& velos_dsA, std::vector<float>& velos_dsB, std::vector<int>& valid_ranges, std::size_t seeds)
{
  auto vertex_count = velos_dsA.size() / 4;
  auto vertices_per_seed = vertex_count / seeds;
  auto* veloA_it = reinterpret_cast<glm::vec4*>(velos_dsA.data());
  auto* veloB_it = reinterpret_cast<glm::vec4*>(velos_dsB.data());

  std::vector<double> vertex_velos;

  for (auto s = 0; s < seeds; s++)
  {
    for (auto v = 0; v < vertices_per_seed; v++)
    {
      if (v >= valid_ranges[s])
      {
        veloA_it++;
        veloB_it++;
        continue;
      }

      const auto& a_velo = *veloA_it;
      const auto& b_velo = *veloB_it;
      auto velocity_diff = glm::abs(glm::length(a_velo) - glm::length(b_velo));
      vertex_velos.push_back(velocity_diff);

      veloA_it++;
      veloB_it++;
    }
  }

  return vertex_velos;
}

std::vector<double> MAX_velocity_difference_per_seed(std::vector<float>& velos_dsA, std::vector<float>& velos_dsB, std::vector<int>& valid_ranges, std::size_t seeds)
{
  auto vertex_count = velos_dsA.size() / 4;
  auto vertices_per_seed = vertex_count / seeds;
  auto* veloA_it = reinterpret_cast<glm::vec4*>(velos_dsA.data());
  auto* veloB_it = reinterpret_cast<glm::vec4*>(velos_dsB.data());

  std::vector<double> max_seed_dists;

  for (auto s = 0; s < seeds; s++)
  {
    double max = -INFINITY;
    double min = INFINITY;

    for (auto v = 0; v < vertices_per_seed; v++)
    {
      if (v >= valid_ranges[s])
      {
        veloA_it++;
        veloB_it++;
        continue;
      }

      const auto& a_velo = *veloA_it;
      const auto& b_velo = *veloB_it;
      auto velocity_diff = glm::abs(glm::length(a_velo) - glm::length(b_velo));

      if (velocity_diff < min)
        min = velocity_diff;

      if (velocity_diff > max)
        max = velocity_diff;

      veloA_it++;
      veloB_it++;
    }

    max_seed_dists.push_back(max);
  }

  return max_seed_dists;
}

double MSE_per_vertex(std::vector<double>& error)
{
  std::vector<double> squared_error;
  squared_error.reserve(error.size());

  for (const auto& e : error)
  {
    squared_error.push_back(e * e);
  }

  return boost::math::statistics::mean(squared_error);
}

std::vector<double> MSE_per_seed(std::vector<double>& error, std::vector<int>& valid_ranges)
{
  const auto seeds = valid_ranges.size();
  std::vector<double> mse_per_seed;
  auto id = 0;

  for (auto s = 0; s < seeds; s++)
  {
    std::vector<double> squared_error;
    squared_error.reserve(valid_ranges[s]);

    for (auto v = 0; v < valid_ranges[s]; v++)
    {
      squared_error.push_back(error[id] * error[id]);
      id++;
    }

    mse_per_seed.push_back(boost::math::statistics::mean(squared_error));
  }

  return mse_per_seed;
}

double MSE(std::vector<double>& error)
{
  std::vector<double> squared_error;
  squared_error.reserve(error.size());

  for (const auto& e : error)
  {
    squared_error.push_back(e * e);
  }

  return boost::math::statistics::mean(squared_error);
}

double weighted_mean_of_seeds(std::vector<double>& averaged_error, std::vector<int>& valid_ranges)
{
  double mean = 0.0;
  std::size_t datapoints = 0;

  for (const auto& v : valid_ranges)
    datapoints += v;

  for (auto s = 0; s < averaged_error.size(); s++)
  {
    mean += valid_ranges[s] * averaged_error[s];
  }

  return mean / datapoints;
}

TEST_CASE("Results Test.", "[jay::engine]")
{
  std::string input_path = "../files/input/";
  std::string output_path = "../files/exports/";
  std::vector <std::pair<std::filesystem::path, std::filesystem::path>> files;

  jay::io filedriver = jay::io();
  std::uint32_t seeds = 0;

  for (const auto& entry0 : std::filesystem::directory_iterator(input_path))
  {
    if (entry0.path().filename().string().find("_") != std::string::npos)
      continue;

    for (const auto& entry1 : std::filesystem::directory_iterator(input_path))
    {
      if (entry0 == entry1)
        continue;

      if (entry1.path().filename().string().find("_") == std::string::npos)
        continue;

      if (entry0.path().extension().string()._Equal((entry1.path().extension().string())))
        files.push_back({ entry0, entry1 });
    }
  }
  std::vector<std::string> pos_descriptions;
  pos_descriptions.push_back("Seeds skipped");
  pos_descriptions.push_back("Steps skipped");
  pos_descriptions.push_back("AVG Distance Vertex");
  pos_descriptions.push_back("MSE Distance Vertex");
  pos_descriptions.push_back("MSE AVG Distance Seed");
  pos_descriptions.push_back("MSE Max Distance");
  pos_descriptions.push_back("RMSE Distance Vertex");
  pos_descriptions.push_back("RMSE AVG Distance Seed");   
  pos_descriptions.push_back("RMSE Max Distance");

  std::vector<std::string> velo_descriptions;
  velo_descriptions.push_back("Seeds skipped");
  velo_descriptions.push_back("Steps skipped");
  velo_descriptions.push_back("AVG Velocity Vertex");
  velo_descriptions.push_back("MSE Velocity Vertex");
  velo_descriptions.push_back("MSE AVG Velocity Seed");
  velo_descriptions.push_back("MSE Max Velocity");
  velo_descriptions.push_back("RMSE Velocity Vertex");
  velo_descriptions.push_back("RMSE AVG Velocity Seed");
  velo_descriptions.push_back("RMSE Max Velocity");

  for (const auto& paths : files)
  { 
    auto src = filedriver.read_vector<float>(paths.first.string());
    auto cmp = filedriver.read_vector<float>(paths.second.string());

    std::cout << "Processing: " << paths.first.filename() << std::endl;
    std::cout << "Comparing: " << paths.second.filename() << std::endl;

    if (paths.first.extension().string()._Equal(".positions"))
    {
      seeds = *reinterpret_cast<std::uint32_t*>(&src[0]);
      src.erase(src.begin());
      cmp.erase(cmp.begin());
      std::vector<double> scalars;

      int steps_skipped = 0;
      int seeds_skipped = 0;
      auto valid_ranges = getValidRanges(src, cmp, seeds, steps_skipped, seeds_skipped);

      if (true)
      {
          std::fill(valid_ranges.begin(), valid_ranges.end(), 2000);
          steps_skipped = 0;
          seeds_skipped = 0;
      }

      auto abs_dist_per_vertex = ABS_distance_per_vertex(src, cmp, valid_ranges, seeds);

      auto avg_dist_per_vertex = AVG_distance_per_vertex(src, cmp, valid_ranges, seeds);
      auto avg_dist_per_seed = AVG_distance_per_seed(src, cmp, valid_ranges, seeds);

      auto max_dist_per_seed = MAX_distance_per_seed(src, cmp, valid_ranges, seeds);

      auto mse_dist_per_vertex = MSE_per_vertex(abs_dist_per_vertex);
      auto mse_dist_per_seed = MSE_per_seed(abs_dist_per_vertex, valid_ranges);
      auto mse_max = MSE(max_dist_per_seed);
      auto mse_dist_per_advection = weighted_mean_of_seeds(mse_dist_per_seed, valid_ranges);

      auto rmse_dist_per_vertex = std::sqrt(mse_dist_per_vertex);
      auto rmse_dist_per_seed = std::sqrt(mse_dist_per_advection);
      auto rmse_max = std::sqrt(mse_max);

      scalars.push_back(seeds_skipped);
      scalars.push_back(steps_skipped);
      scalars.push_back(avg_dist_per_vertex);
      scalars.push_back(mse_dist_per_vertex);
      scalars.push_back(mse_dist_per_advection);
      scalars.push_back(mse_max);
      scalars.push_back(rmse_dist_per_vertex);
      scalars.push_back(rmse_dist_per_seed);
      scalars.push_back(rmse_max);

      double max = -INFINITY;
      double min = INFINITY;

      std::cout << "AVG Distance per Vertex: " << avg_dist_per_vertex << "\n";
      std::cout << "MSE Distance per Vertex: " << mse_dist_per_vertex << "\n";
      std::cout << "MSE Distance per Seed: " << mse_dist_per_advection << "\n";
      std::cout << "MSE Max Distance: " << mse_max << "\n";
      std::cout << "RMSE Distance per Vertex: " << rmse_dist_per_vertex << "\n";
      std::cout << "RMSE Distance per Seed: " << rmse_dist_per_seed << "\n";
      std::cout << "RMSE Max Distance: " << rmse_max << "\n";

      jay::data_io::store_descriptive_text("Scalar statistics of:\n" + paths.first.filename().string() + "\n" + paths.second.filename().string(), pos_descriptions, scalars, output_path + "SCALARS_POS_" + paths.second.filename().string() + ".txt", 10, true);
      //jay::data_io::store_text(paths.first.filename().string() + "\n" + paths.second.filename().string(), max_dist_per_seed, output_path + "SCALARS_" + paths.second.filename().string() + ".txt", 10, true);
      jay::data_io::store_text("MAX Distance per Seed: \n" + paths.first.filename().string() + "\n" + paths.second.filename().string(), max_dist_per_seed, output_path + "MAX_POS_DIST_SEED_" + paths.second.filename().string() + ".txt", 10, true);
      jay::data_io::store_text("AVG Distance per Seed: \n" + paths.first.filename().string() + "\n" + paths.second.filename().string(), avg_dist_per_seed, output_path + "AVG_POS_DIST_SEED_" + paths.second.filename().string() + ".txt", 10, true);
      jay::data_io::store_text("MSE Distance per Seed: \n" + paths.first.filename().string() + "\n" + paths.second.filename().string(), mse_dist_per_seed, output_path + "MSE_POS_DIST_SEED_" + paths.second.filename().string() + ".txt", 10, true);
    }
    else
    {
      seeds = *reinterpret_cast<std::uint32_t*>(&src[0]);
      src.erase(src.begin());
      cmp.erase(cmp.begin());

      std::vector<double> scalars;

      int steps_skipped = 0;
      int seeds_skipped = 0;
      auto valid_ranges = getValidRanges(src, cmp, seeds, steps_skipped, seeds_skipped);

      if (true)
      {
        std::fill(valid_ranges.begin(), valid_ranges.end(), 2000);
        steps_skipped = 0;
        seeds_skipped = 0;
      }

      auto abs_dist_per_vertex = ABS_velocity_diff_per_vertex(src, cmp, valid_ranges, seeds);

      auto avg_dist_per_vertex = AVG_velocity_difference_per_vertex(src, cmp, valid_ranges, seeds);
      auto avg_dist_per_seed = AVG_velocity_difference_per_seed(src, cmp, valid_ranges, seeds);

      auto max_dist_per_seed = MAX_velocity_difference_per_seed(src, cmp, valid_ranges, seeds);

      auto mse_dist_per_vertex = MSE_per_vertex(abs_dist_per_vertex);
      auto mse_dist_per_seed = MSE_per_seed(abs_dist_per_vertex, valid_ranges);
      auto mse_max = MSE(max_dist_per_seed);
      auto mse_dist_per_advection = weighted_mean_of_seeds(mse_dist_per_seed, valid_ranges);

      auto rmse_dist_per_vertex = std::sqrt(mse_dist_per_vertex);
      auto rmse_dist_per_seed = std::sqrt(mse_dist_per_advection);
      auto rmse_max = std::sqrt(mse_max);

      scalars.push_back(seeds_skipped);
      scalars.push_back(steps_skipped);
      scalars.push_back(avg_dist_per_vertex);
      scalars.push_back(mse_dist_per_vertex);
      scalars.push_back(mse_dist_per_advection);
      scalars.push_back(mse_max);
      scalars.push_back(rmse_dist_per_vertex);
      scalars.push_back(rmse_dist_per_seed);
      scalars.push_back(rmse_max);

      double max = -INFINITY;
      double min = INFINITY;

      std::cout << "AVG Difference per Vertex: " << avg_dist_per_vertex << "\n";
      std::cout << "MSE Difference per Vertex: " << mse_dist_per_vertex << "\n";
      std::cout << "MSE Difference per Seed: " << mse_dist_per_advection << "\n";
      std::cout << "MSE Max Difference: " << mse_max << "\n";
      std::cout << "RMSE Difference per Vertex: " << rmse_dist_per_vertex << "\n";
      std::cout << "RMSE Difference per Seed: " << rmse_dist_per_seed << "\n";
      std::cout << "RMSE Max Difference: " << rmse_max << "\n";

      jay::data_io::store_descriptive_text("Scalar statistics of:\n" + paths.first.filename().string() + "\n" + paths.second.filename().string(), velo_descriptions, scalars, output_path + "SCALARS_VELO_" + paths.second.filename().string() + ".txt", 10, true);
      //jay::data_io::store_text(paths.first.filename().string() + "\n" + paths.second.filename().string(), max_dist_per_seed, output_path + "SCALARS_" + paths.second.filename().string() + ".txt", 10, true);
      jay::data_io::store_text("MAX Difference per Seed: \n" + paths.first.filename().string() + "\n" + paths.second.filename().string(), max_dist_per_seed, output_path + "MAX_VELO_DIFF_SEED_" + paths.second.filename().string() + ".txt", 10, true);
      jay::data_io::store_text("AVG Difference per Seed: \n" + paths.first.filename().string() + "\n" + paths.second.filename().string(), avg_dist_per_seed, output_path + "AVG_VELO_DIFF_SEED_" + paths.second.filename().string() + ".txt", 10, true);
      jay::data_io::store_text("MSE Difference per Seed: \n" + paths.first.filename().string() + "\n" + paths.second.filename().string(), mse_dist_per_seed, output_path + "MSE_VELO_DIFF_SEED_" + paths.second.filename().string() + ".txt", 10, true);



      std::cout << "Velocity Distance Error:" << std::endl;
      //vector_difference(src, cmp, seeds);
    }

    std::cout << "\n\n";
  }
};