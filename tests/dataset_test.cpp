#include <catch2/catch.hpp>

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>

#include <jay/api.hpp>

std::vector<std::vector<double>> MSE_diff_per_slice(std::vector<float>& source, std::vector<float>& cmp, std::vector<std::size_t>& grid)
{
  auto slice_size = grid[0] * grid[1] * 3;
  auto depth = (grid.size() > 3) ? grid[2] * grid[3] : grid[2];

  auto* src_it = reinterpret_cast<glm::vec3*>(source.data());
  auto* cmp_it = reinterpret_cast<glm::vec3*>(cmp.data());

  std::vector<std::vector<double>> component_diffs(3);

  for (auto d = 0; d < depth; d++)
  {
    glm::dvec3 component_diff{ 0, 0, 0 };

    for (auto v = 0; v < slice_size / 3; v++)
    {
      auto vec_src = *src_it;
      auto vec_cmp = *cmp_it;

      component_diff += (vec_src - vec_cmp) * (vec_src - vec_cmp);

      src_it++;
      cmp_it++;
    }

    glm::dvec3 mse = component_diff;
    component_diffs[0].push_back(mse.x / (float)slice_size);
    component_diffs[1].push_back(mse.y / (float)slice_size);
    component_diffs[2].push_back(mse.z / (float)slice_size);
  }

  std::cout << component_diffs[0][0];
  
  return component_diffs;
}


glm::dvec3 MSE_diff_per_vector(std::vector<std::vector<double>>& mse_diff_per_slice, std::vector<std::size_t>& grid)
{
  glm::dvec3 component_diff = { 0.0, 0.0, 0.0 };

  for (const auto& slice_mse : mse_diff_per_slice[0])
  {
    component_diff.x += slice_mse;
  }

  for (const auto& slice_mse : mse_diff_per_slice[1])
  {
    component_diff.y += slice_mse;
  }

  for (const auto& slice_mse : mse_diff_per_slice[2])
  {
    component_diff.z += slice_mse;
  }



  return (component_diff / (double) mse_diff_per_slice[0].size());
}

std::vector<std::vector<double>> RMSE_diff_per_slice(std::vector<std::vector<double>>& mse_diff_per_slice)
{
  std::vector<std::vector<double>> rmse(3);

  for (const auto& mse : mse_diff_per_slice[0])
  {
    rmse[0].push_back(std::sqrt(mse));
  }

  for (const auto& mse : mse_diff_per_slice[1])
  {
    rmse[1].push_back(std::sqrt(mse));
  }

  for (const auto& mse : mse_diff_per_slice[2])
  {
    rmse[2].push_back(std::sqrt(mse));
  }

  return rmse;
}


glm::dvec3 RMSE_diff_per_vector(glm::dvec3& mse_diff_per_vector)
{
  return { std::sqrt(mse_diff_per_vector.x), std::sqrt(mse_diff_per_vector.y), std::sqrt(mse_diff_per_vector.z) };
}

std::vector<std::vector<double>> NRMSE_diff_per_slice_refined(std::vector<std::vector<double>>& rmse_diff_per_slice, std::vector<float>& src_minmax)
{
  std::vector<std::vector<double>> nrmse(3);

  for (auto d = 0; d < rmse_diff_per_slice[0].size(); d++)
  {
    const auto& rmse = rmse_diff_per_slice[0][d];

    glm::dvec2 minmax_x = { src_minmax[d * 6 + 0], src_minmax[d * 6 + 1] };

    double divisor = minmax_x[1] - minmax_x[0];

    nrmse[0].push_back(rmse / divisor);
  }

  for (auto d = 0; d < rmse_diff_per_slice[1].size(); d++)
  {
    const auto& rmse = rmse_diff_per_slice[1][d];

    glm::dvec2 minmax_y = { src_minmax[d * 6 + 2], src_minmax[d * 6 + 3] };

    double divisor = minmax_y[1] - minmax_y[0];

    nrmse[1].push_back(rmse / divisor);
  }

  for (auto d = 0; d < rmse_diff_per_slice[2].size(); d++)
  {
    const auto& rmse = rmse_diff_per_slice[2][d];

    glm::dvec2 minmax_z = { src_minmax[d * 6 + 4], src_minmax[d * 6 + 5] };

    double divisor = minmax_z[1] - minmax_z[0];

    nrmse[2].push_back(rmse / divisor);
  }

  return nrmse;
}

std::vector<std::vector<double>> NRMSE_diff_per_slice(std::vector<std::vector<double>>& rmse_diff_per_slice, std::vector<float>& src_minmax)
{
  std::vector<std::vector<double>> nrmse(3);

  for (auto d = 0; d < rmse_diff_per_slice[0].size(); d++)
  {
    const auto& rmse = rmse_diff_per_slice[0][d];

    glm::dvec2 minmax_x = { src_minmax[d * 2 + 0], src_minmax[d * 2 + 1] };

    double divisor = minmax_x[1] - minmax_x[0];

    nrmse[0].push_back(rmse / divisor);
  }

  for (auto d = 0; d < rmse_diff_per_slice[1].size(); d++)
  {
    const auto& rmse = rmse_diff_per_slice[1][d];

    glm::dvec2 minmax_y = { src_minmax[d * 2 + 0], src_minmax[d * 2 + 1] };

    double divisor = minmax_y[1] - minmax_y[0];

    nrmse[1].push_back(rmse / divisor);
  }

  for (auto d = 0; d < rmse_diff_per_slice[2].size(); d++)
  {
    const auto& rmse = rmse_diff_per_slice[2][d];

    glm::dvec2 minmax_z = { src_minmax[d * 2 + 0], src_minmax[d * 2 + 1] };

    double divisor = minmax_z[1] - minmax_z[0];

    nrmse[2].push_back(rmse / divisor);
  }

  return nrmse;
}

glm::dvec3 NRMSE_diff_per_vector(glm::dvec3& rmse_diff_per_vector, glm::dvec3 global_maximum, glm::dvec3 global_minimum)
{

  glm::dvec3 divisor = global_maximum - global_minimum;

  return (rmse_diff_per_vector / divisor);
}

std::pair<glm::dvec3, glm::dvec3> global_minmax_refined(std::vector<float>& min_max)
{

  glm::vec3 gMax = { -INFINITY, -INFINITY, -INFINITY };
  glm::vec3 gMin = { INFINITY, INFINITY, INFINITY };

  for (auto i = 0; i < min_max.size() / 6; i++)
  {
    glm::vec2 x = { min_max[i * 6 + 0], min_max[i * 6 + 1] };
    glm::vec2 y = { min_max[i * 6 + 2], min_max[i * 6 + 3] };
    glm::vec2 z = { min_max[i * 6 + 4], min_max[i * 6 + 5] };

    if (x[1] > gMax.x)
      gMax.x = x[1];
    if (y[1] > gMax.y)
      gMax.y = y[1];
    if (z[1] > gMax.z)
      gMax.z = z[1];

    if (x[0] < gMin.x)
      gMin.x = x[0];
    if (y[0] < gMin.y)
      gMin.y = y[0];
    if (z[0] < gMin.z)
      gMin.z = z[0];
  }

  return { gMin, gMax};
}

std::pair<glm::dvec3, glm::dvec3> global_minmax(std::vector<float>& min_max)
{

  glm::vec3 gMax = { -INFINITY, -INFINITY, -INFINITY };
  glm::vec3 gMin = { INFINITY, INFINITY, INFINITY };

  for (auto i = 0; i < min_max.size() / 2; i++)
  {
    glm::vec2 minmax = { min_max[i * 2 + 0], min_max[i * 2 + 1] };

    if (minmax[1] > gMax.x)
      gMax = { minmax[1], minmax[1], minmax[1] };

    if (minmax[0] < gMin.x)
      gMin = { minmax[0], minmax[0], minmax[0] };
  }

  return { gMin, gMax };
}


TEST_CASE("Dataset Test.", "[jay::engine]")
{
  std::string input_path = "../files/input/";
  std::string output_path = "../files/exports/";
  std::vector <std::pair<std::filesystem::path, std::filesystem::path>> files;
  std::vector <std::filesystem::path> source_files;
  std::vector <std::filesystem::path> peak_files;
  jay::io filedriver = jay::io();
  jay::astc astc_driver = jay::astc();
  std::uint32_t seeds = 0;

  for (const auto& entry0 : std::filesystem::directory_iterator(input_path))
  {
    if (entry0.path().filename().extension().string().compare(".nc") != 0)
      continue;
    source_files.push_back(entry0);

    for (const auto& entry1 : std::filesystem::directory_iterator(input_path))
    {
      if (entry0 == entry1)
        continue;

      if (entry1.path().filename().extension().string().compare(".astc") != 0)
        continue;

      files.push_back({ entry0, entry1 });
    }
  }

  for (const auto& entry : std::filesystem::directory_iterator(input_path))
  {
    if (!entry.path().filename().extension().string()._Equal(".peaks"))
      continue;

    peak_files.push_back(entry);

  }

  std::vector<std::string> descs;
  descs.push_back("MSE per Vector (X)");
  descs.push_back("MSE per Vector (Y)");
  descs.push_back("MSE per Vector (Z)");

  descs.push_back("RMSE per Vector (X)");
  descs.push_back("RMSE per Vector (Y)");
  descs.push_back("RMSE per Vector (Z)");

  descs.push_back("NRMSE per Vector (X)");
  descs.push_back("NRMSE per Vector (Y)");
  descs.push_back("NRMSE per Vector (Z)");


  for (const auto& src_paths : source_files)
  {
    std::cout << "Processing: " << src_paths.string() << std::endl;
    std::vector<float> minmax_src;

    for (const auto& pk_file : peak_files)
    {
      if (pk_file.filename().extension().string()._Equal(".peaks"))
      {
        std::cout << pk_file.filename().root_name().string() << std::endl;
        minmax_src = filedriver.read_vector<float>(pk_file.string());
        break;
      }
    }

    filedriver.hdf5_open(src_paths.string(), { "u", "v", "w" });
    auto src = filedriver.hdf5_read<float>();
    auto grid = filedriver.hdf5_get_grid();

    for (const auto& cmp_paths : files)
    {
      if (!src_paths.string()._Equal(cmp_paths.first.string()))
        continue;

      std::cout << "Comparing: " << cmp_paths.second.filename() << std::endl;

      std::vector<double> scalars;


      auto cmp = filedriver.astc_read(cmp_paths.second.string());

      astc_driver.set_blocksizes(cmp.block_x, cmp.block_y, cmp.block_z);
      astc_driver.apply_all_settings();

      auto decompressed = astc_driver.decompress(cmp);
      cmp.data.clear();
      cmp.data.shrink_to_fit();
      auto converted = astc_driver.convert_img_to_data_refined(decompressed, grid, 3, true, minmax_src, jay::colorspace::RGB, 0);
      for (auto& img : decompressed)
        astc_driver.free_image(img);
      decompressed.clear();
      decompressed.shrink_to_fit();

      //auto minmax_cmp = astc_driver.find_peaks_per_component<float>(converted.data(), grid, 3);

      auto minmax = global_minmax_refined(minmax_src);

      glm::vec3 gMax = minmax.second;
      glm::vec3 gMin = minmax.first;

      auto mse_slice = MSE_diff_per_slice(src.data, converted, grid);
      auto mse_vector = MSE_diff_per_vector(mse_slice, grid);
      auto rmse_slice = RMSE_diff_per_slice(mse_slice);
      auto rmse_vector = RMSE_diff_per_vector(mse_vector);
      auto nrmse_slice = NRMSE_diff_per_slice_refined(mse_slice, minmax_src);
      auto nrmse_vector = NRMSE_diff_per_vector(mse_vector, gMax, gMin);

      scalars.push_back(mse_vector.x);
      scalars.push_back(mse_vector.y);
      scalars.push_back(mse_vector.z);

      scalars.push_back(rmse_vector.x);
      scalars.push_back(rmse_vector.y);
      scalars.push_back(rmse_vector.z);

      scalars.push_back(nrmse_vector.x);
      scalars.push_back(nrmse_vector.y);
      scalars.push_back(nrmse_vector.z);

      std::cout << "MSE Vector: " << mse_vector.x << ", " << mse_vector.y << ", " << mse_vector.z << "\n";
      std::cout << "RMSE Vector: " << rmse_vector.x << ", " << rmse_vector.y << ", " << rmse_vector.z << "\n";
      std::cout << "NRMSE Vector: " << nrmse_vector.x << ", " << nrmse_vector.y << ", " << nrmse_vector.z << "\n";

      jay::data_io::store_descriptive_text("Scalar statistics of:\n" + src_paths.filename().string() + "\n" + cmp_paths.second.filename().string(), descs , scalars, output_path + "SCALARS_" + cmp_paths.second.filename().string() + ".txt", 10, true);
      //jay::data_io::store_text(paths.first.filename().string() + "\n" + paths.second.filename().string(), max_dist_per_seed, output_path + "SCALARS_" + paths.second.filename().string() + ".txt", 10, true);
      jay::data_io::store_text("MSE per Slice (X): \n" + src_paths.filename().string() + "\n" + cmp_paths.second.filename().string(), mse_slice[0], output_path + "MSE_SLICE_X_" + cmp_paths.second.filename().string() + ".txt", 10, true);
      jay::data_io::store_text("MSE per Slice (Y): \n" + src_paths.filename().string() + "\n" + cmp_paths.second.filename().string(), mse_slice[1], output_path + "MSE_SLICE_Y_" + cmp_paths.second.filename().string() + ".txt", 10, true);
      jay::data_io::store_text("MSE per Slice (Z): \n" + src_paths.filename().string() + "\n" + cmp_paths.second.filename().string(), mse_slice[2], output_path + "MSE_SLICE_Z_" + cmp_paths.second.filename().string() + ".txt", 10, true);

      jay::data_io::store_text("RMSE per Slice (X): \n" + src_paths.filename().string() + "\n" + cmp_paths.second.filename().string(), rmse_slice[0], output_path + "RMSE_SLICE_X_" + cmp_paths.second.filename().string() + ".txt", 10, true);
      jay::data_io::store_text("RMSE per Slice (Y): \n" + src_paths.filename().string() + "\n" + cmp_paths.second.filename().string(), rmse_slice[1], output_path + "RMSE_SLICE_Y_" + cmp_paths.second.filename().string() + ".txt", 10, true);
      jay::data_io::store_text("RMSE per Slice (Z): \n" + src_paths.filename().string() + "\n" + cmp_paths.second.filename().string(), rmse_slice[2], output_path + "RMSE_SLICE_Z_" + cmp_paths.second.filename().string() + ".txt", 10, true);

      jay::data_io::store_text("NRMSE per Slice (X): \n" + src_paths.filename().string() + "\n" + cmp_paths.second.filename().string(), nrmse_slice[0], output_path + "NRMSE_SLICE_X_" + cmp_paths.second.filename().string() + ".txt", 10, true);
      jay::data_io::store_text("NRMSE per Slice (Y): \n" + src_paths.filename().string() + "\n" + cmp_paths.second.filename().string(), nrmse_slice[1], output_path + "NRMSE_SLICE_Y_" + cmp_paths.second.filename().string() + ".txt", 10, true);
      jay::data_io::store_text("NRMSE per Slice (Z): \n" + src_paths.filename().string() + "\n" + cmp_paths.second.filename().string(), nrmse_slice[2], output_path + "NRMSE_SLICE_Z_" + cmp_paths.second.filename().string() + ".txt", 10, true);
    }

    // Per Value (component)
    // Mean (RMS)
    // Min (RMS)
    // Max (RMS)

    // Per Vector
    // Magnitude
    //  - Min (RMS)
    //  - Max (RMS)
    //  - Mean (RMS)
    // Angle
    //   -Min(RMS)
    //  - Max (RMS)
    //  - Mean (RMS)

    std::cout << "\n\n";
  }
};