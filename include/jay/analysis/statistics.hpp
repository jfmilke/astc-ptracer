#ifndef JAY_ANALYSIS_STATISTICS_HPP
#define JAY_ANALYSIS_STATISTICS_HPP
#include <vector>
#include <algorithm>
#include <numeric>
#include <jay/export.hpp>

#include <boost/range/adaptor/strided.hpp>
#include <boost/range/adaptor/sliced.hpp>
#include <boost/math/statistics/univariate_statistics.hpp>
#include <boost/math/statistics/bivariate_statistics.hpp>

#include <jay/types/image.hpp>

#include <glm/vec2.hpp>

// Calculate elements in strideded & offsetted array:
// floor( (n - 1 - o + s) / s )
// n: number of elements
// o: offset
// s: stride
// (1: kernelsize)

namespace jay
{
  // Returns arithmetic mean of image
  template <typename T>
  double get_mean(std::vector<T>& data, long unsigned int stride = 1, std::size_t offset_begin = 0, std::size_t offset_end = 0)
  {
    auto sliced = boost::adaptors::slice(data, offset_begin, data.size() - offset_end);
    auto strided = boost::adaptors::stride(sliced, stride);
    return boost::math::statistics::mean(strided.begin(), strided.end());
  }

  // Returns arithmetic mean of given windows
  template <typename T>
  std::vector<double> get_mean(std::vector<std::vector<T>>& windows)
  {
    std::vector<double> means(windows.size());

    for (auto i = 0; i < windows.size(); i++)
    {
      const auto& window = windows[i];

      means[i] = boost::math::statistics::mean(window.begin(), window.end());
    }

    return means;
  }

  // Subdivides an image into windows of given size.
  // Non-full windows are 
  // Best use when transforming the image into windows per color channels (use stride and offset accordingly)
  template <typename T, typename Tvec>
  std::vector<std::vector<Tvec>> get_windows(std::vector<T>& data, glm::ivec2 data_size, int window_size, int channel, int img_channels)
  {
    // for (auto i = 0; i < data.size() / img_channels; i++)
    // {
    //   if ((int)data[i * img_channels + 2] > 0)
    //     std::cout << (int)data[i * img_channels + 2] << "\n";
    // }

    const auto sliced = boost::adaptors::slice(data, channel, data.size());
    const auto strided = boost::adaptors::stride(sliced, img_channels);

    const auto& x_dim = data_size.x;
    const auto& y_dim = data_size.y;

    const int x_windows = std::ceil((float)x_dim / window_size);
    const int y_windows = std::ceil((float)y_dim / window_size);

    // In case of nun full windows, fill them artifically
    // const int x_fill = x_dim % window_size;
    // const int y_fill = y_dim % window_size;
    const int x_fill = (x_dim % window_size) ? window_size - (x_dim % window_size) : 0;
    const int y_fill = (y_dim % window_size) ? window_size - (y_dim % window_size) : 0;


    std::vector<std::vector<Tvec>> windows(x_windows * y_windows);

    for (auto y = 0; y < y_dim; y++)
    {
      const auto y_win = y / window_size;

      for (auto x = 0; x < x_dim; x++)
      {
        const auto x_win = x / window_size;

        const T val = strided[y * x_dim + x];
        windows[y_win * x_windows + x_win].push_back((int)val);
      }

      if (x_fill)
      {
        const auto x_win = (x_dim) / window_size;
        const T val = strided[y * x_dim + x_dim - 1];

        for (auto xf = 0; xf < x_fill; xf++)
        {
          windows[y_win * x_windows + x_win].push_back((int)val);
        }
      }
    }


    if (y_fill)
    {
      for (auto yf = 0; yf < y_fill; yf++)
      {
        const auto y_win = yf / window_size;

        for (auto x = 0; x < x_dim; x++)
        {
          const auto x_win = x / window_size;

          const T val = strided[(y_dim - 1) * x_dim + x];
          windows[y_win * x_windows + x_win].push_back((int)val);
        }

        if (x_fill)
        {
          const auto x_win = (x_dim) / window_size;
          const T val = strided[(y_dim - 1) * x_dim + x_dim - 1];

          for (auto xf = 0; xf < x_fill; xf++)
          {
            windows[y_win * x_windows + x_win].push_back((int)val);
          }
        }
      }
    }

    return windows;
  }

  // Returns median of an image and modifies the input according to std::nth_element
  template <typename T>
  T get_median(std::vector<T>* data, long unsigned int stride, std::size_t offset_begin = 0, std::size_t offset_end = 0)
  {
    auto sliced = boost::adaptors::slice(*data, offset_begin, data->size() - offset_end);
    auto strided = boost::adaptors::stride(sliced, stride);
    return boost::math::statistics::median(strided.begin(), strided.end());
  }

  // Returns medians of the given windows and modifies the input according to std::nth_element
  template <typename T>
  std::vector<T> get_median(std::vector<std::vector<T>>* windows, long unsigned int stride = 1, std::size_t offset_begin = 0, std::size_t offset_end = 0)
  {
    auto sliced = boost::adaptors::slice(*windows, offset_begin, windows.size() - offset_end);
    auto strided = boost::adaptors::stride(sliced, stride);

    std::vector<double> medians(strided.size());

    for (auto i = 0; i < strided.size(); i++)
      medians[i] = boost::math::statistics::median(strided[i].begin(), strided[i].end());

    return medians;
  }

  // Returns median of an image (leaves input untouched)
  template <typename T>
  T get_median(std::vector<T> data, long unsigned int stride, std::size_t offset_begin = 0, std::size_t offset_end = 0)
  {
    return get_median(&data, stride, offset_begin, offset_end);
  }

  // Returns median of the given windows (leaves input untouched)
  template <typename T>
  std::vector<T> get_median(std::vector<std::vector<T>> windows, long unsigned int stride = 1, std::size_t offset_begin = 0, std::size_t offset_end = 0)
  {
    auto sliced = boost::adaptors::slice(windows, offset_begin, windows.size() - offset_end);
    auto strided = boost::adaptors::stride(sliced, stride);

    std::vector<T> medians(strided.size());

    for (auto i = 0; i < strided.size(); i++)
      medians[i] = boost::math::statistics::median(strided[i].begin(), strided[i].end());

    return medians;
  }

  // Returns median absolute deviation of an image
  template <typename T>
  double calculate_mad(std::vector<T>& data, T median, long unsigned int stride = 1, std::size_t offset_begin = 0, std::size_t offset_end = 0)
  {
    auto sliced = boost::adaptors::slice(data, offset_begin, data.size() - offset_end);
    auto strided = boost::adaptors::stride(sliced, stride);
    
    return boost::math::statistics::median_absolute_deviation(strided.begin(), strided.end(), median);
  }

  // Returns median absolute deviation of the given windows
  template <typename T>
  std::vector<double> calculate_mad(std::vector<std::vector<T>>& windows, std::vector<T> medians, long unsigned int stride = 1, std::size_t offset_begin = 0, std::size_t offset_end = 0)
  {
    auto sliced = boost::adaptors::slice(windows, offset_begin, windows.size() - offset_end);
    auto strided = boost::adaptors::stride(sliced, stride);

    std::vector<double> mads(strided.size());

    for (auto i = 0; i < strided.size(); i++)
      mads[i] = boost::math::statistics::median_absolute_deviation(strided[i].begin(), strided[i].end(), medians[i]);

    return mads;
  }
  
  // Returns variance of the image
  template <typename T>
  double calculate_variance(std::vector<T>& data, long unsigned int stride = 1, std::size_t offset_begin = 0, std::size_t offset_end = 0)
  {
    auto sliced = boost::adaptors::slice(data, offset_begin, data.size() - offset_end);
    auto strided = boost::adaptors::stride(sliced, stride);

    return boost::math::statistics::variance(strided.begin(), strided.end());
  }

  // Returns variances of the given windows
  template <typename T>
  std::vector<double> calculate_variance(std::vector<std::vector<T>>& windows)
  {
    std::vector<double> variances(windows.size());

    for (auto i = 0; i < windows.size(); i++)
    {
      const auto& window = windows[i];

      variances[i] = boost::math::statistics::variance(window.begin(), window.end());
    }

    return variances;
  }
  
  // Returns covariance of two images
  // Manual implementation as the boost method seems not to work here
  template <typename T>
  double calculate_covariance(std::vector<T>& data0, std::vector<T>& data1, double mean0, double mean1, long unsigned int stride = 1, std::size_t offset_begin0 = 0, std::size_t offset_begin1 = 0, std::size_t offset_end0 = 0, std::size_t offset_end1 = 0)
  {
    auto sliced0 = boost::adaptors::slice(data0, offset_begin0, data0.size() - offset_end0);
    auto strided0 = boost::adaptors::stride(sliced0, stride);

    auto sliced1 = boost::adaptors::slice(data1, offset_begin1, data1.size() - offset_end1);
    auto strided1 = boost::adaptors::stride(sliced1, stride);

    double cov = 0.0;

    for (auto i = 0; i < strided0.size(); i++)
    {
      cov += (strided0[i] - mean0) * (strided1[i] - mean1) / strided0.size();
    }
    return cov;
  }

  // Returns covariance of the given windows of two images
  // Manual implementation as the boost method seems not to work here
  template <typename T>
  std::vector<double> calculate_covariance(std::vector<std::vector<T>>& windows0, std::vector<std::vector<T>>& windows1, std::vector<double>& mean0, std::vector<double>& mean1)
  {
    std::vector<double> covs(windows0.size());

    for (auto w = 0; w < windows0.size(); w++)
    {
      for (auto i = 0; i < windows0[w].size(); i++)
      {
        covs[w] += (windows0[w][i] - mean0[w]) * (windows1[w][i] - mean1[w]);
      }

      covs[w] /= windows0[w].size();
    }

    return covs;
  }
  
  // Returns the interquartile range of an image
  template <typename T>
  double calculate_interquartile_range(std::vector<T>& data, long unsigned int stride, std::size_t offset_begin = 0, std::size_t offset_end = 0)
  {
    auto sliced = boost::adaptors::slice(data, offset_begin, data.size() - offset_end);
    auto strided = boost::adaptors::stride(sliced, stride);
    return boost::math::statistics::interquartile_range(strided.begin(), strided.end());
  }

  // Returns the interquartile ranges of the given windows
  template <typename T>
  std::vector<double> calculate_interquartile_range(std::vector<std::vector<T>> windows, long unsigned int stride = 1, std::size_t offset_begin = 0, std::size_t offset_end = 0)
  {
    auto sliced = boost::adaptors::slice(windows, offset_begin, windows.size() - offset_end);
    auto strided = boost::adaptors::stride(sliced, stride);

    std::vector<double> ranges(strided.size());

    for (auto i = 0; i < strided.size(); i++)
      ranges[i] = boost::math::statistics::interquartile_range(strided[i].begin(), strided[i].end());

    return ranges;
  }

  // Returns Empiric Standard Deviation of an image
  template <typename T>
  double get_standard_deviation(T variance)
  {
    return sqrt(variance);
  }

  // Returns Empiric Standard Deviation of the given windows
  template <typename T>
  std::vector<double> get_standard_deviation(std::vector<double> variances)
  {
    std::vector<double> sds(strided.size());

    for (auto i = 0; i < variances.size(); i++)
      sds[i] = sqrt(variances[i]);

    return sds;
  }
  
  // Returns Mean Square Error of an image for a single channel.
  // MSE = 1/size * sum((img0 - img1)²)
  template<typename T>
  double get_mse(std::vector<T>& img0, std::vector<T>& img1, glm::ivec2 dimensions, int stride, int offset)
  {
    auto sliced0 = boost::adaptors::slice(img0, offset, img0.size());
    auto strided0 = boost::adaptors::stride(sliced0, stride);

    auto sliced1 = boost::adaptors::slice(img1, offset, img1.size());
    auto strided1 = boost::adaptors::stride(sliced1, stride);

    double inv_size = 1.0 / (dimensions.x * dimensions.y);
    double mse = 0.0;

    for (auto i = 0; i < strided0.size(); i++)
    {
      const auto& p0 = strided0[i];
      const auto& p1 = strided1[i];

      mse += (p0 - p1) * (p0 - p1) * inv_size;
    }

    return mse;
  }

  // Returns Mean Square Error for the given windows for a single channel.
  // MSE = 1/size * sum((img0 - img1)²)
  template<typename T>
  std::vector<double> get_mse(std::vector<std::vector<T>>& windows0, std::vector<std::vector<T>>& windows1, glm::ivec2 dimensions, int stride, int offset)
  {
    auto sliced0 = boost::adaptors::slice(windows0, offset, windows0.size());
    auto strided0 = boost::adaptors::stride(sliced0, stride);

    auto sliced1 = boost::adaptors::slice(windows1, offset, windows1.size());
    auto strided1 = boost::adaptors::stride(sliced1, stride);

    double inv_size = 1.0 / (dimensions.x * dimensions.y);
    std::vector<double> mse(strided0.size());

    for (auto i = 0; i < strided0.size(); i++)
      for (auto j = 0; j < strided0[i].size(); j++)
      {
        const auto& p0 = strided0[i][j];
        const auto& p1 = strided1[i][j];

        mse[i] += (p0 - p1) * (p0 - p1) * inv_size;
      }

    return mse;
  }

}

#endif