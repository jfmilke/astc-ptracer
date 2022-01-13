#ifndef JAY_ANALYSIS_PSNR_HPP
#define JAY_ANALYSIS_PSNR_HPP
#include <jay/export.hpp>
#include <vector>
#include <jay/analysis/statistics.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <jay/types/image.hpp>

namespace jay
{
  // Converts 3 channel rgb to 1 channel luminance.
  image rgb_to_luminance(image& img);
  // Euclidean difference between each pixel in red.
  // img1 can be used as overlay & error coloring can be intensified
  image rgb_distance_image(image& img1, image& img2, bool overlay = false, float visualization_factor = 1.0);
  // Like rgb_distance_image, but only show error for non-black pixels of img1.
  image rgb_edgedistance_image(image& img1, image& img2, bool overlay = false, float visualization_factor = 1.0);

  // Converts 1 channel luminance to 3 channel rgb.
  image luminance_to_rgb(image& img);
  // img1 - img2 with colormapping (negative to red, positive to blue).
  image lum_difference_image(image& img1, image& img2);
  // Absolute distance of scalar values.
  image lum_distance_image(image& img1, image& img2, float visualization_factor = 1.0);
  // Like lum_distance_image, but only show error for non-black pixels of img1.
  image lum_edgedistance_image(image& img1, image& img2, float visualization_factor = 1.0);

  // Calculates the luminance component of SSIM (for whole images, given the mean)
  double luminance_comp(double mean_img0, double mean_img1);

  // Calculates the luminance component of SSIM (for subwindows, given the means)
  std::vector<double> luminance_comp(std::vector<double>& mean_img0, std::vector<double>& mean_img1);

  // Calculates the contrast component of SSIM (for whole images, given the variance)
  double contrast_comp(double variance_img0, double variance_img1);

  // Calculates the contrast component of SSIM (for subwindows, given the variances)
  std::vector<double> contrast_comp(std::vector<double>& variance_img0, std::vector<double>& variance_img1);

  // Calculates the structure component of SSIM (for whole images, given the (co-)variance)
  double structure_comp(double variance_img0, double variance_img1, double covariance);

  // Calculates the structure component of SSIM (for subwindows, given the (co-)variances)
  std::vector<double> structure_comp(std::vector<double>& variance_img0, std::vector<double>& variance_img1, std::vector<double>& covariance);

  // Structural Similarity Index Measure for whole images.
  // Channel 0 - 2 will calculate for the seperate color channels (RGB).
  // If all channels should be tested convert to grayscale before.
  double ssim(image& source, image& compare, int channel, double alpha = 1.0, double beta = 1.0, double gamma = 1.0);

  // Structural Similarity Index Measure for subwindows.
  // Channel 0 - 2 will calculate for the seperate color channels (RGB).
  // If all channels should be tested convert to grayscale before.
  std::vector<double> ssim_windowed(image& source, image& compare, int channel, int window_size, double alpha = 1.0, double beta = 1.0, double gamma = 1.0);

  // Returns PSNR for a whole image.
  // Channel 0 - 2 selects the seperate color channel for calculation.
  // If all channels should be tested convert to grayscale before.
  // PSNR = 10 * log10(255² / mse)
  //      = 20 * log10(255) - 10 * log10(mse)
  double psnr(image& source, image& compare, int channel);

  // Returns PSNR for the given windows.
  // Channel 0 - 2 selects the seperate color channel for calculation.
  // If all channels should be tested convert to grayscale before.
  // PSNR = 10 * log10(255² / mse)
  //      = 20 * log10(255) - 10 * log10(mse)
  std::vector<double> psnr_windowed(image& source, image& compare, int channel, int window_size);

  image colorize_images_by_ssim(image img, int window_size, std::vector<double>& ssim);

  image windows_to_img(std::vector<std::vector<int>>& windows, int window_size, glm::ivec2 img_dimensions);
}

#endif