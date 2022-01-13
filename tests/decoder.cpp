#include <catch2/catch.hpp>

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <filesystem>
#include <regex>
#include <chrono>
#include <jay/api.hpp>

#include <vector>


void compare(std::vector<float> source, std::vector<float> data0, std::vector<float> data1)
{
  std::vector<std::size_t> accumulator_0(3);
  std::vector<std::size_t> accumulator_1(3);
  double diff0 = 0.0;
  double diff1 = 0.0;

  for (auto i = 0; i < data0.size(); i++)
  {
    auto d0_diff = std::abs(source[i] - data0[i]);
    auto d1_diff = std::abs(source[i] - data1[i]);

    diff0 += d0_diff;//* d0_diff;
    diff1 += d1_diff;//* d1_diff;

    if (d0_diff > d1_diff)
      accumulator_0[i % 3]++;
    else if (d0_diff < d1_diff)
      accumulator_1[i % 3]++;
  }

  if ((accumulator_0[0] + accumulator_0[1] + accumulator_0[2]) > (accumulator_1[0] + accumulator_1[1] + accumulator_1[2]))
    std::cout << "data0 loses" << std::endl;
  else
    std::cout << "data1 loses" << std::endl;

  std::cout << accumulator_0[0] << ", " << accumulator_0[1] << ", " << accumulator_0[2] << " (" << diff0 / source.size() << ")" << std::endl;
  std::cout << "vs." << std::endl;
  std::cout << accumulator_1[0] << ", " << accumulator_1[1] << ", " << accumulator_1[2] << " (" << diff1 / source.size() << ")" << std::endl << std::endl;
}



TEST_CASE("Decoder.", "[jay::engine]") 
{
  std::string input_path = "../files/";
  std::string output_path = "../files/";
  std::vector<std::string> datasets = { "u", "v", "w" };

  jay::io filedriver = jay::io();

  // Load ASTC file
  auto astc_imgs0 = filedriver.astc_read(output_path + "ctbl3d_EXH-1N-Mask-4x4x1.astc");
  auto peaks0 = filedriver.read_vector<float>(output_path + "ctbl3d0" + ".peaks");

  auto astc_imgs1 = filedriver.astc_read(output_path + "ctbl3d_EXH-3N-Mask-4x4x1.astc");
  auto peaks1 = filedriver.read_vector<float>(output_path + "ctbl3d1" + ".peaks");

  // Load original file for meta knowledge
  filedriver.hdf5_open(input_path + "ctbl3d" + ".nc", datasets);
  auto dataset = filedriver.hdf5_read<float>();

  auto grid = filedriver.hdf5_get_grid_fixsize();
  auto vec_len = filedriver.hdf5_get_vec_len();

  // Decompress & convert
  jay::astc astc_compressor = jay::astc();
  astc_compressor.set_profile(astcenc_profile::ASTCENC_PRF_LDR);
  astc_compressor.apply_all_settings();

  auto imgs0 = astc_compressor.decompress(astc_imgs0);
  auto imgs1 = astc_compressor.decompress(astc_imgs1);
  auto data0 = astc_compressor.convert_img_to_data(imgs0, grid, vec_len, true, peaks0, jay::colorspace(vec_len));
  auto data1 = astc_compressor.convert_img_to_data_refined(imgs1, grid, vec_len, true, peaks1, jay::colorspace(vec_len));

  compare(dataset.data, data0, data1);

  /*
  for (auto i = 0; i < input_filenames.size(); i++)
  {
    // Load ASTC file
    auto astc_imgs = filedriver.astc_read(output_path + input_filenames[i]);
    auto peaks = filedriver.read_vector<float>(output_path + filenames[i] + ".peaks");

    // Load original file for meta knowledge
    filedriver.hdf5_open(input_path + filenames[i] + ".nc", datasets);
    auto dataset = filedriver.hdf5_read<float>();
    auto grid = filedriver.hdf5_get_grid_fixsize();
    auto vec_len = filedriver.hdf5_get_vec_len();

    // Decompress & convert
    jay::astc astc_compressor = jay::astc();
    auto imgs = astc_compressor.decompress(astc_imgs);
    auto data = (i == 1) ? astc_compressor.convert_img_to_data_refined(imgs, grid, vec_len, true, peaks, jay::colorspace(vec_len)) : astc_compressor.convert_img_to_data(imgs, grid, vec_len, true, peaks, jay::colorspace(vec_len));
    //auto data = astc_compressor.convert_img_to_data(imgs, grid, vec_len, false, peaks, jay::colorspace(vec_len));

    auto meanX = jay::get_mean(data, 3, 0, (data.size() - 128 * 128 * 3));
    auto meanY = jay::get_mean(data, 3, 1, (data.size() - 128 * 128 * 3));
    auto meanZ = jay::get_mean(data, 3, 2, (data.size() - 128 * 128 * 3));

    std::cout << meanX << std::endl;
    std::cout << meanY << std::endl;
    std::cout << meanZ << std::endl << std::endl;


    
    std::vector<double> sdevX;
    std::vector<double> sdevY;
    std::vector<double> sdevZ;


    for (auto i = 0; i < 127; i++)
    {
      auto meanX = jay::get_mean(data, 3, i * 128 * 128 * 3, (data.size() - 128 * 128 * 3));
      auto svarX = jay::calculate_variance(data, 3, i * 128 * 128 * 3, (data.size() - (i + 1) * 128 * 128 * 3));
      //auto svarY = jay::calculate_variance(data, 3, 1, (data.size() - 128 * 128 * 3));
      //auto svarZ = jay::calculate_variance(data, 3, 2, (data.size() - 128 * 128 * 3));

      sdevX.push_back(jay::get_standard_deviation(svarX));
      //sdevY.push_back(jay::get_standard_deviation(svarY));
      //sdevZ.push_back(jay::get_standard_deviation(svarZ));
    }

    
    for (const auto& e : sdevX)
      std::cout << e << std::endl;

    std::cout << std::endl;

  }
  */
};