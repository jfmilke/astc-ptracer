#include <jay/analysis/psnr_measure.hpp>
#include <stb_image_write.h>
#include <stb_image.h>
#include <iostream>

glm::vec3 rgb(double ratio)
{
  //we want to normalize ratio so that it fits in to 6 regions
  //where each region is 126 units long
  int normalized = int(ratio * 256 * 2);

  //find the distance to the start of the closest region
  int x = normalized % 256;

  int red = 0, grn = 0, blu = 0;
  if (ratio >= 0)
  {
    switch (normalized / 256)
    {
    case 0: red = 255;      grn = x;        blu = 0;       break;//red
    case 1: red = 255 - x;  grn = 255;      blu = 0;       break;//yellow
    }
  }
  else
  {
    return {255, 0, 0 };
  }

  return { red, grn, blu };
}

image jay::rgb_to_luminance(image& img)
{
  image luminance;
  luminance.size = img.size;
  luminance.data.resize(img.data.size() / 3);

  for (auto i = 0; i < img.data.size() / 3; i++)
  {
    luminance.data[i] = (img.data[3 * i] + img.data[3 * i + 1] + img.data[3 * i + 2])/3;
  }

  return luminance;
}

image jay::luminance_to_rgb(image& img)
{
  image rgb;
  rgb.size = img.size;
  rgb.data.resize(img.data.size() * 3);

  for (auto i = 0; i < img.data.size(); i++)
  {
    rgb.data[3 * i + 0] = img.data[i];
    rgb.data[3 * i + 1] = img.data[i];
    rgb.data[3 * i + 2] = img.data[i];

  }

  return rgb;
}


image jay::lum_difference_image(image& img1, image& img2)
{
  if (img1.size != img2.size)
  {
    std::cout << "Error: Images don't have same dimensions!" << std::endl;
    return image();
  }

  image diff;
  diff.data.resize(img1.data.size() * 3);
  diff.size = img1.size;

  for (auto i = 0; i < img1.data.size(); i++)
  {
    auto c = img1.data[i] - img2.data[i];

    if (c >= 0)
      diff.data[3 * i + 2] = c;
    else
      diff.data[3 * i] = -c;
  }

  return diff;
}

image jay::lum_distance_image(image& img1, image& img2, float visualization_factor)
{
  if (img1.size != img2.size)
  {
    std::cout << "Error: Images don't have same dimensions!" << std::endl;
    return image();
  }

  image dist;
  dist.data.resize(img1.data.size());
  dist.size = img1.size;

  for (auto i = 0; i < img1.data.size(); i++)
  {
    auto c = std::min<int>(std::abs(img1.data[i] - img2.data[i]) * visualization_factor, 255);

    dist.data[i] = c;
  }

  return dist;
}

image jay::rgb_distance_image(image& img1, image& img2, bool overlay, float visualization_factor)
{
  if (img1.size != img2.size)
  {
    std::cout << "Error: Images don't have same dimensions!" << std::endl;
    return image();
  }

  image dist;
  dist.data.resize(img1.data.size());
  dist.size = img1.size;

  float min = INFINITY;
  float max = -INFINITY;

  for (auto i = 0; i < img1.data.size() / 3; i++)
  {
    auto r = img1.data[3 * i] - img2.data[3 * i];
    auto g = img1.data[3 * i + 1] - img2.data[3 * i + 1];
    auto b = img1.data[3 * i + 2] - img2.data[3 * i + 2];
    auto d = std::sqrtf(r * r + g * g + b * b);

    if (d < min)
      min = d;
    else if (d > max)
      max = d;

    dist.data[3 * i] = d;
  }

  if (overlay)
    for (auto i = 0; i < dist.data.size() / 3; i++)
    {
      float lambda = (dist.data[3 * i] - min) / (max - min);

      dist.data[3 * i] = std::min(lambda * visualization_factor, 1.f) * 255 + std::min((1.f - lambda * visualization_factor), 1.f) * img1.data[3 * i];
      dist.data[3 * i + 1] = img1.data[3 * i + 1];
      dist.data[3 * i + 2] = img1.data[3 * i + 2];
    }
  else
    for (auto i = 0; i < dist.data.size() / 3; i++)
    {
      float lambda = (dist.data[3 * i] - min) / (max - min);

      dist.data[3 * i] = std::min(lambda * visualization_factor, 1.f) * 255;
      dist.data[3 * i + 1] = 0;
      dist.data[3 * i + 2] = 0;
    }

  return dist;
}

image jay::rgb_edgedistance_image(image& img1, image& img2, bool overlay, float visualization_factor)
{
  if (img1.size != img2.size)
  {
    std::cout << "Error: Images don't have same dimensions!" << std::endl;
    return image();
  }

  image dist;
  dist.data.resize(img1.data.size());
  dist.size = img1.size;

  float min = INFINITY;
  float max = -INFINITY;

  for (auto i = 0; i < img1.data.size() / 3; i++)
  {
    if ((img1.data[3 * i] + img1.data[3 * i + 1] + img1.data[3 * i + 2]) == 0)
    {
      dist.data[3 * i] = 0;
      continue;
    }

    auto r = img1.data[3 * i] - img2.data[3 * i];
    auto g = img1.data[3 * i + 1] - img2.data[3 * i + 1];
    auto b = img1.data[3 * i + 2] - img2.data[3 * i + 2];
    auto d = std::sqrtf(r * r + g * g + b * b);

    if (d < min)
      min = d;
    else if (d > max)
      max = d;

    dist.data[3 * i] = d;
  }

  if (overlay)
    for (auto i = 0; i < dist.data.size() / 3; i++)
    {
      float lambda = (dist.data[3 * i] - min) / (max - min);

      dist.data[3 * i] = std::min(lambda * visualization_factor, 1.f) * 255 + std::min((1.f - lambda * visualization_factor), 1.f) * img1.data[3 * i];
      dist.data[3 * i + 1] = img1.data[3 * i + 1];
      dist.data[3 * i + 2] = img1.data[3 * i + 2];
    }
  else
    for (auto i = 0; i < dist.data.size() / 3; i++)
    {
      float lambda = (dist.data[3 * i] - min) / (max - min);

      dist.data[3 * i] = std::min(lambda * visualization_factor, 1.f) * 255;
      dist.data[3 * i + 1] = 0;
      dist.data[3 * i + 2] = 0;
    }

  return dist;
}

image jay::lum_edgedistance_image(image& img1, image& img2, float visualization_factor)
{
  if (img1.size != img2.size)
  {
    std::cout << "Error: Images don't have same dimensions!" << std::endl;
    return image();
  }

  image dist;
  dist.data.resize(img1.data.size());
  dist.size = img1.size;

  float min = INFINITY;
  float max = -INFINITY;

  for (auto i = 0; i < img1.data.size(); i++)
  {
    if (img1.data[i] == 0)
    {
      dist.data[i] = 0;
      continue;
    }

    dist.data[i] = std::abs(img1.data[i] - img2.data[i]);
  }

  return dist;
}


// Calculates the luminance component of SSIM (for whole images, given the mean)
double jay::luminance_comp(double mean_img0, double mean_img1)
{
  double k1 = 0.01;
  double c1 = k1 * 255.0;
  c1 *= c1;
  return (2.0 * mean_img0 * mean_img1 + c1) / (mean_img0 * mean_img0 + mean_img1 * mean_img1 + c1);
}

// Calculates the luminance component of SSIM (for subwindows, given the means)
std::vector<double> jay::luminance_comp(std::vector<double>& mean_img0, std::vector<double>& mean_img1)
{
  std::vector<double> luminances(mean_img0.size());

  for (auto i = 0; i < mean_img0.size(); i++)
    luminances[i] = luminance_comp(mean_img0[i], mean_img1[i]);

  return luminances;
}

// Calculates the contrast component of SSIM (for whole images, given the variance)
double jay::contrast_comp(double variance_img0, double variance_img1)
{
  double k2 = 0.03;
  double c2 = k2 * 255.0;
  c2 *= c2;
  double sdev_img0 = sqrt(variance_img0);
  double sdev_img1 = sqrt(variance_img1);
  return (2.0 * sdev_img0 * sdev_img1 + c2) / (variance_img0 + variance_img1 + c2);
}

// Calculates the contrast component of SSIM (for subwindows, given the variances)
std::vector<double> jay::contrast_comp(std::vector<double>& variance_img0, std::vector<double>& variance_img1)
{
  std::vector<double> contrasts(variance_img0.size());

  for (auto i = 0; i < variance_img0.size(); i++)
    contrasts[i] = contrast_comp(variance_img0[i], variance_img1[i]);

  return contrasts;
}

// Calculates the structure component of SSIM (for whole images, given the (co-)variance)
double jay::structure_comp(double variance_img0, double variance_img1, double covariance)
{
  double k2 = 0.03;
  double c2 = k2 * 255.0;
  c2 *= c2;
  double c3 = c2 / 2.0;
  double sdev_img0 = sqrt(variance_img0);
  double sdev_img1 = sqrt(variance_img1);
  return (covariance + c3) / (sdev_img0 * sdev_img1 + c3);
}

// Calculates the structure component of SSIM (for subwindows, given the (co-)variances)
std::vector<double> jay::structure_comp(std::vector<double>& variance_img0, std::vector<double>& variance_img1, std::vector<double>& covariance)
{
  std::vector<double> structures(variance_img0.size());

  for (auto i = 0; i < variance_img0.size(); i++)
    structures[i] = structure_comp(variance_img0[i], variance_img1[i], covariance[i]);

  return structures;
}

// Structural Similarity Index Measure for whole images.
// Channel 0 - 2 will calculate for the seperate color channels (RGB).
// If all channels should be tested convert to grayscale before.
double jay::ssim(image& source, image& compare, int channel, double alpha, double beta, double gamma)
{
  auto src_mean = get_mean(source.data, 3, channel, 0);
  auto cmp_mean = get_mean(compare.data, 3, channel, 0);

  auto src_variance = calculate_variance(source.data, 3, channel, 0);
  auto cmp_variance = calculate_variance(compare.data, 3, channel, 0);

  auto cov = calculate_covariance(source.data, compare.data, src_mean, cmp_mean, 3, channel, channel);

  auto luminance = luminance_comp(src_mean, cmp_mean);
  auto contrast = contrast_comp(src_variance, cmp_variance);
  auto structure = structure_comp(src_variance, cmp_variance, cov);

  return std::pow(luminance, alpha) * std::pow(contrast, beta) * std::pow(structure, gamma);
}

// Structural Similarity Index Measure for subwindows.
// Channel 0 - 2 will calculate for the seperate color channels (RGB).
// If all channels should be tested convert to grayscale before.
std::vector<double> jay::ssim_windowed(image& source, image& compare, int channel, int window_size, double alpha, double beta, double gamma)
{
  std::vector<double> ssim;
  auto img_channels = source.data.size() / (source.size.x * source.size.y);
  auto windows_src = get_windows<std::uint8_t, int>(source.data, source.size, window_size, channel, img_channels);
  auto windows_cmp = get_windows<std::uint8_t, int>(compare.data, compare.size, window_size, channel, img_channels);  

  auto src_mean = get_mean(windows_src);
  auto cmp_mean = get_mean(windows_cmp);

  auto src_variance = calculate_variance(windows_src);
  auto cmp_variance = calculate_variance(windows_cmp);

  auto cov = calculate_covariance(windows_src, windows_cmp, src_mean, cmp_mean);

  
  auto luminance = luminance_comp(src_mean, cmp_mean);
  auto contrast = contrast_comp(src_variance, cmp_variance);
  auto structure = structure_comp(src_variance, cmp_variance, cov);

  for (auto window = 0; window < windows_src.size(); window++)
    ssim.push_back(std::pow(luminance[window], alpha) * std::pow(contrast[window], beta) * std::pow(structure[window], gamma));

  return ssim;
}

// Returns PSNR for a whole image.
// Channel 0 - 2 selects the seperate color channel for calculation.
// If all channels should be tested convert to grayscale before.
// PSNR = 10 * log10(255² / mse)
//      = 20 * log10(255) - 10 * log10(mse)
double jay::psnr(image& source, image& compare, int channel)
{
  auto mse = get_mse(source.data, compare.data, source.size, 3, channel);

  return 20.0 * log10(255) - 10.0 * log10(mse);
}

// Returns PSNR for the given windows.
// Channel 0 - 2 selects the seperate color channel for calculation.
// If all channels should be tested convert to grayscale before.
// PSNR = 10 * log10(255² / mse)
//      = 20 * log10(255) - 10 * log10(mse)
std::vector<double> jay::psnr_windowed(image& source, image& compare, int channel, int window_size)
{
  auto img_channels = source.data.size() / (source.size.x * source.size.y);

  auto windows_src = get_windows<std::uint8_t, int>(source.data, source.size, window_size, channel, img_channels);
  auto windows_cmp = get_windows<std::uint8_t, int>(compare.data, compare.size, window_size, channel, img_channels);

  std::vector<double> psnr(windows_src.size());
  auto mse = get_mse(windows_src, windows_cmp, source.size, 3, channel);

  for (auto i = 0; i < psnr.size(); i++)
    psnr[i] = 20.0 * log10(255) - 10.0 * log10(mse[i]);

  return psnr;
}

image jay::colorize_images_by_ssim(image img, int window_size, std::vector<double>& ssim)
{
  auto img_channels = img.data.size() / (img.size.x * img.size.y);

  double max = ssim[0];
  double min = ssim[0];

  const auto& x_dim = img.size.x;
  const auto& y_dim = img.size.y;

  const int x_windows = std::ceil((float)x_dim / window_size);
  const int y_windows = std::ceil((float)y_dim / window_size);

  // In case of nun full windows, fill them artifically
  const int x_fill = x_dim % window_size;
  const int y_fill = y_dim % window_size;

  for (const auto& diff : ssim)
  {
    if (diff > max)
      max = diff;

    if (diff < min)
      min = diff;
  }

  std::cout << "Min SSIM: " << min << "\n";
  std::cout << "Max SSIM: " << max << "\n\n";

  for (auto y = 0; y < y_dim; y++)
  {
    const auto win_y = y / window_size;

    for (auto x = 0; x < x_dim; x++)
    {
      const auto win_x = x / window_size;
      const auto win_id = win_y * (x_windows) + win_x;

      //float ssim_val = (max != min) ? (ssim[win_id] - min) / (max - min) : ssim[win_id];
      float ssim_val = ssim[win_id];

      auto coloring = (glm::ivec3) (rgb(ssim_val) * (1.f - ssim_val));

      img.data[img_channels * (y * x_dim + x) + 0] = std::min(img.data[img_channels * (y * x_dim + x) + 0] + coloring.r, 255);
      img.data[img_channels * (y * x_dim + x) + 1] = std::min(img.data[img_channels * (y * x_dim + x) + 1] + coloring.g, 255);
    }
  }

  return img;
}

image jay::windows_to_img(std::vector<std::vector<int>>& windows, int window_size, glm::ivec2 img_dimensions)
{
  const auto& x_dim = img_dimensions.x;
  const auto& y_dim = img_dimensions.y;

  auto color_channels = (windows.size() * windows[0].size()) / (x_dim * y_dim);

  const int x_windows = std::ceil((float)x_dim / window_size);
  const int y_windows = std::ceil((float)y_dim / window_size);

  image output;
  output.size = img_dimensions;
  output.data.resize(x_dim * y_dim * color_channels);

  for (auto y = 0; y < y_dim; y++)
  {
    const auto win_y = y / window_size;
    const auto sub_y = y % window_size;

    for (auto x = 0; x < x_dim; x++)
    {
      const auto win_x = x / window_size;
      const auto sub_x = x % window_size;

      const auto win_id = win_y * (x_windows) + win_x;

      for (auto c = 0; c < color_channels; c++)
      {
        output.data[color_channels * (y * x_dim + x) + c] = windows[win_id][sub_y * window_size + sub_x + c];
      }
    }
  }

  return output;
}
