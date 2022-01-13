#include <catch2/catch.hpp>

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <stb_image_write.h>

#include <jay/api.hpp>

TEST_CASE("Jay PSNR Test.", "[jay::engine]")
{
  std::string input_path = "../files/input/";
  std::string output_path = "../files/exports/";
  std::vector <std::pair<std::filesystem::path, std::filesystem::path>> files;

  for (const auto& entry : std::filesystem::directory_iterator(input_path))
  {
    const std::string name_str = entry.path().filename().string();
    std::string number_str = name_str.substr(name_str.find_last_of("_") + 1, 1);
    try
    {
      const int number = std::stoi(number_str);

      if (number + 1 > files.size())
        files.resize(number + 1);
      
      if (name_str.find("Mask") != std::string::npos)
        files[number].second = entry;
      else
        files[number].first = entry;
    }
    catch (std::invalid_argument)
    {
      std::cout << "Skip file without number: " << name_str << "\n";
      continue;
    }
  }

  for (const auto& paths : files)
  {
    int window_size = 11;
    std::vector<std::string> descs;

    std::cout << "Processing: " << paths.first.filename().string() << "\n";
    std::cout << "Against: " << paths.second.filename().string() << "\n";

    auto src = jay::image_io::import_image(paths.first.string());
    auto cmp = jay::image_io::import_image(paths.second.string());

    auto src_gray = jay::rgb_to_luminance(src);
    auto cmp_gray = jay::rgb_to_luminance(cmp);

    auto ssims = jay::ssim_windowed(src_gray, cmp_gray, 0, window_size);
    auto colored = jay::colorize_images_by_ssim(cmp, window_size, ssims);

    for (auto i = 0; i < ssims.size(); i++)
    {
      descs.push_back(std::to_string(i));
    }
    descs.push_back("Avg");
    descs.push_back("Median");
    descs.push_back("Full picture");
    auto mean = jay::get_mean(ssims);
    auto median = jay::get_median(ssims, 1);
    ssims.push_back(mean);
    ssims.push_back(median);
    ssims.push_back(jay::ssim(src_gray, cmp_gray, 1));

    jay::data_io::store_descriptive_text(paths.second.filename().string() + "\nSSIM Measurement by Windows. Global SSIM Value is the mean.\nWindow Size: " + std::to_string(window_size), descs, ssims, "../files/exports/" + paths.second.filename().string() + "_ssims.txt", 4, true);

    jay::image_io::export_bmp(output_path + paths.second.filename().string() + "_colored.bmp", &colored, 3, false);
    std::cout << "\n\n";
  }
  
  std::cout << "fin" << std::endl;
};