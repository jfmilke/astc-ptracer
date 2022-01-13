#include <catch2/catch.hpp>

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <jay/api.hpp>

#include <vector>

// Calculates the average error of a single timeslice
template <typename T>
void average_error(
  const std::vector<T>&              src_data,
  const std::vector<std::size_t>&    grid,
  const std::size_t                  vec_len,
  const std::vector<astcenc_image*>& astc_data,
  const jay::colorspace              astc_colorspace,
  const std::vector<float>&          peaks,
  bool                               single_slice_avg
)
{
  std::cout.precision(6);

  /* Process source data */
  std::uint8_t grid_dim = grid.size();
  std::size_t  grid_x   = grid[0];
  std::size_t  grid_y   = (grid_dim >= 2) ? grid[1] : 1;
  std::size_t  grid_z   = (grid_dim >= 3) ? grid[2] : 1;
  std::size_t  grid_t   = (grid_dim >= 4) ? grid[3] : 1;

  if (single_slice_avg)
  {
    grid_z   = 1;
    grid_t   = 1;
  }

  float       source_sum      = 0;
  std::size_t source_elements = 0;
  float       source_max      = src_data[0];
  float       source_min      = src_data[0];
  std::size_t offset          = 0;

  for (std::size_t z = 0; z < grid_z; z++)
  {
    for (std::size_t y = 0; y < grid_y; y++)
    {
      for (std::size_t x = 0; x < grid_x; x++)
      {
        for (std::size_t c = 0; c < vec_len; c++)
        {
          const float& val = src_data[offset];

          source_sum += val;

          if (val > source_max)
            source_max = val;
          
          if (val < source_min)
            source_min = val;

          source_elements++;
          offset++;
        }
      }
    }
  }

  float source_avg = source_sum / source_elements;

  std::cout << "Average(src):       " << source_avg << std::endl;
  std::cout << "Max:                " << source_max << std::endl;
  std::cout << "Min:                " << source_min << std::endl;
  std::cout << "Count:              " << source_elements << std::endl;
  std::cout << "Shape:              " << src_data.size() << std::endl;

  std::cout << std::endl;

  /* Process image data */
  uint16_t***  astc_img    = static_cast<uint16_t***>(astc_data[0]->data);
  std::size_t  padding     = astc_data[0]->dim_pad;
  std::uint8_t astc_colors = int(astc_colorspace);
  std::size_t  max_imgs    = astc_data.size();

  if (single_slice_avg)
    max_imgs = 1;

  /* Create normalization parameters */
  float norm_min = peaks[0];
  float norm_div = peaks[1] - norm_min;

  float  astc_sum      = 0;
  size_t astc_elements = 0;
  float  astc_max      = sf16_to_float(astc_img[padding][padding][padding]) * norm_div + norm_min;
  float  astc_min      = astc_max;
         offset        = 0;

  /* Loop through the whole dataset if you want */
  for (std::size_t img_nr = 0; img_nr < max_imgs; img_nr++)
  {                           
    std::uint16_t*** astc_img    = static_cast<std::uint16_t***>(astc_data[img_nr]->data);
                     norm_min    = peaks[img_nr];
                     norm_div    = peaks[img_nr + 1] - norm_min;

    /* And always loop through the whole image */
    for (std::size_t z = 0; z < astc_data[img_nr]->dim_z; z++)
    {
      std::size_t z_dst = z + padding;

      for (std::size_t y = 0; y < astc_data[img_nr]->dim_y; y++)
      {
        std::size_t y_dst = y + padding;

        for (std::size_t x = 0; x < astc_data[img_nr]->dim_x; x++)
        {
          std::size_t x_dst = x + padding;

          for (std::size_t c = 0; c < astc_colors; c++)
          {
            float nval = sf16_to_float(astc_img[z_dst][y_dst][x_dst * 4 + c]);
            float val  = jay::astc::denormalize_float(nval, peaks, img_nr);

            astc_sum += val;

            if (val > astc_max)
              astc_max = val;

            else if (val < astc_min)
              astc_min = val;

            astc_elements++;
          }
        }
      }
    }
  }


  float astc_avg = astc_sum / astc_elements;
  std::cout << "Average(img): " << astc_avg << std::endl;
  std::cout << "Max:          " << astc_max << std::endl;
  std::cout << "Min:          " << astc_min << std::endl;
  std::cout << "Count:        " << astc_elements << std::endl;
  std::cout << "Shape:        " << astc_data.size() << std::endl;

  std::cout << std::endl;
}

TEST_CASE("Compression Test.", "[jay::engine]") 
{
  // Known beforehand:
  std::string              filepath = "../files/";
  std::string              filename = "tangaroa.nc";
  std::string              output_name = "tangaroa";
  std::vector<std::string> datasets = { "u", "v", "w" };

  jay::io filedriver = jay::io();
  filedriver.hdf5_open(filepath + filename, datasets);

  // Load vectorfield:
  auto grid    = filedriver.hdf5_get_grid_fixsize();
  auto vec_len = filedriver.hdf5_get_vec_len();

  auto dataset = filedriver.hdf5_read_subset<float>({ grid[0], grid[1], grid[2], 100 }, { 0, 0, 0, 0 });


  // Initialize a specific compressor
  jay::astc astc_compressor = jay::astc();

  // Read peaks for normalization/denormalization & convert the data
  auto      peaks           = astc_compressor.find_peaks<float>(dataset);
  auto      imgs            = astc_compressor.convert_data_to_img(dataset, true, peaks, jay::colorspace::RGB, jay::slicetype::Plane, 0);

  // Don't need source anymore
  dataset.data.clear();

  // Optional: Apply custom settings
  // Otherwise it will be set to maximum resolution 2D slices, LDR profile and FAST preset.
  if (true)
  {
    astc_compressor.set_preset(astcenc_preset::ASTCENC_PRE_THOROUGH);
    astc_compressor.color_setting = jay::colorspace::RGB;
    astc_compressor.edit_flags(0,0,1);
    astc_compressor.edit_edge_scaling(1.5);
    astc_compressor.apply_all_settings();
  }

  // Compress
  auto astc_imgs = astc_compressor.compress(imgs);

  // Store vectorfield & peaks if you want
  if (true)
  {
    filedriver.astc_store(astc_imgs, filepath + output_name + "_rgb_EDGE15.astc", 1);
    filedriver.store_vector(peaks, filepath + output_name + "_PEAKS.jay", 1);
  }
};