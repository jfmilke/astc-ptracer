#include <catch2/catch.hpp>

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <filesystem>
#include <regex>
#include <chrono>
#include <jay/api.hpp>

#include <vector>

std::vector<std::size_t> timings;
std::vector<std::string> descriptions;

std::string input_path = "./input/";
std::string output_path = "./output/";
std::vector<std::string> datasets = { "u", "v", "w" };

void compress_custom(std::string filename, std::vector<astcenc_image*> data_imgs, std::vector<float>& peaks, jay::io& filedriver, jay::astc& astc_compressor, astcenc_preset preset, glm::ivec3 blocksize, bool per_channel_normalization, bool mask)
{
  astc_compressor.set_profile(astcenc_profile::ASTCENC_PRF_LDR);
  astc_compressor.set_preset(preset);
  astc_compressor.set_blocksizes(blocksize.x, blocksize.y, blocksize.z);
  astc_compressor.color_setting = jay::colorspace::RGB;
  astc_compressor.edit_flags(0, 0, mask);
  //astc_compressor.edit_edge_scaling(1.0);
  astc_compressor.apply_all_settings();
  std::string settings = "";

  switch (preset)
  {
  case astcenc_preset::ASTCENC_PRE_FAST:
    settings += "_FAST";
    break;
  case astcenc_preset::ASTCENC_PRE_MEDIUM:
    settings += "_MED";
    break;
  case astcenc_preset::ASTCENC_PRE_THOROUGH:
    settings += "_THOR";
    break;
  case astcenc_preset::ASTCENC_PRE_EXHAUSTIVE:
    settings += "_EXH";
  }

  if (per_channel_normalization)
    settings += "-3N";
  else
    settings += "-1N";

  if (mask)
    settings += "-Mask";

  settings += "-" + std::to_string(blocksize.x) + "x" + std::to_string(blocksize.y) + "x" + std::to_string(blocksize.z);

  auto t0 = std::chrono::steady_clock::now();
  auto astc_imgs = astc_compressor.compress(data_imgs);
  descriptions.push_back("Compress File (s)");
  auto t1 = std::chrono::steady_clock::now();
  timings.push_back((double)std::chrono::duration_cast<std::chrono::seconds>(t1 - t0).count());
  descriptions.push_back(settings);
  timings.push_back(0.0);

  filedriver.astc_store(astc_imgs, output_path + filename + settings + ".astc", 1);
  astc_imgs.data.~vector();

  std::cout << "Stored " + settings + ".astc" << std::endl;
}

TEST_CASE("Encoder.", "[jay::engine]")
{
  // Full input Filenames with extension
  std::vector<std::string> input_filenames;
  for (const auto& entry : std::filesystem::directory_iterator(input_path))
    input_filenames.push_back(entry.path().filename().string());

  // Input filename without extension
  std::vector<std::string> filenames;
  for (const auto& input_fn : input_filenames)
  {
    filenames.push_back(input_fn.substr(0, input_fn.find_last_of('.')));
  }

  int counter = 0;

  jay::io filedriver = jay::io();

  jay::astc astc_compressor = jay::astc();

  // Generic compression schemes for all files (3 channel normalization)
  for (auto n = 0; n < 2; n++)
    for (auto i = 0; i < input_filenames.size(); i++)
    {
      std::cout << filenames[i] << std::endl;
      // "Filename 1N" or "Filename 3N" based on normalization settings
      descriptions.push_back(filenames[i] + " " + std::to_string(n * 2 + 1) + "N");
      timings.push_back(0.0);

      // Load File
      auto t0 = std::chrono::steady_clock::now();
      filedriver.hdf5_open(input_path + input_filenames[i], datasets);
      auto grid = filedriver.hdf5_get_grid_fixsize();
      auto vec_len = filedriver.hdf5_get_vec_len();
      auto dataset = filedriver.hdf5_read<float>();
      descriptions.push_back("Load File (ms)");
      auto t1 = std::chrono::steady_clock::now();
      timings.push_back((std::size_t)std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());

      // Find Peaks
      t0 = std::chrono::steady_clock::now();
      auto peaks = (n) ? astc_compressor.find_peaks_per_component<float>(dataset) : astc_compressor.find_peaks<float>(dataset);
      descriptions.push_back("Find Peaks (ms)");
      t1 = std::chrono::steady_clock::now();
      timings.push_back((std::size_t)std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());

      // Convert data
      t0 = std::chrono::steady_clock::now();
      auto imgs = (n) ? astc_compressor.convert_data_to_img_refined(dataset, true, peaks, jay::colorspace::RGB, jay::slicetype::Plane, 0) :
        astc_compressor.convert_data_to_img(dataset, true, peaks, jay::colorspace::RGB, jay::slicetype::Plane, 0);
      descriptions.push_back("Convert Data (ms)");
      t1 = std::chrono::steady_clock::now();
      timings.push_back((std::size_t)std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());

      // Store peaks
      filedriver.store_vector(peaks, output_path + filenames[i] + std::to_string(n) + ".peaks", 1);
      std::cout << "Stored .peaks" << std::endl;

      // Compress & Store data with varying settings
      for (auto m = 0; m < 1; m++)
        for (auto p = 0; p < 4; p++)
        {
          jay::astc astc_compressor0 = jay::astc();
          compress_custom(filenames[i], imgs, peaks, filedriver, astc_compressor0, astcenc_preset(p), glm::ivec3(4, 4, 1), n, m);
          astc_compressor0.free_contexts();

          jay::astc astc_compressor1 = jay::astc();
          compress_custom(filenames[i], imgs, peaks, filedriver, astc_compressor1, astcenc_preset(p), glm::ivec3(5, 4, 1), n, m);
          astc_compressor1.free_contexts();


          jay::astc astc_compressor2 = jay::astc();
          compress_custom(filenames[i], imgs, peaks, filedriver, astc_compressor2, astcenc_preset(p), glm::ivec3(5, 5, 1), n, m);
          astc_compressor2.free_contexts();


          jay::astc astc_compressor3 = jay::astc();
          compress_custom(filenames[i], imgs, peaks, filedriver, astc_compressor3, astcenc_preset(p), glm::ivec3(6, 5, 1), n, m);
          astc_compressor3.free_contexts();


          jay::astc astc_compressor4 = jay::astc();
          compress_custom(filenames[i], imgs, peaks, filedriver, astc_compressor4, astcenc_preset(p), glm::ivec3(6, 6, 1), n, m);
          astc_compressor4.free_contexts();

          jay::astc astc_compressor5 = jay::astc();
          compress_custom(filenames[i], imgs, peaks, filedriver, astc_compressor5, astcenc_preset(p), glm::ivec3(12, 12, 1), n, m);
          astc_compressor5.free_contexts();

          jay::data_io::store_descriptive_text("Compression Timings", descriptions, timings, output_path + "Timings_Maximum" + std::to_string(counter) + ".txt", 1, 1);
          counter++;
        }

      // Free memory
      for (auto& img : imgs)
        astc_compressor.free_image(img);

      std::cout << std::endl;
    }

  std::cout << "Finished Compression" << std::endl;

  jay::data_io::store_descriptive_text("Compression Timings", descriptions, timings, output_path + "Timings_Maximum.txt", 1, 1);

  std::cout << "Stored Timings.txt" << std::endl;
};