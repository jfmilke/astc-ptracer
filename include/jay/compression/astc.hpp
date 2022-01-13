#ifndef JAY_COMP_ASTC_HPP
#define JAY_COMP_ASTC_HPP

#include <fstream>
#include <thread>


#include <astcenc.h>
#include <astcenc_mathlib.h>
#include <glm/glm.hpp>

#include <jay/compression/compressor.hpp>
#include <jay/types/image.hpp>
#include <jay/types/jaydata.hpp>
#include <jay/export.hpp>



namespace jay
{


class JAY_EXPORT astc : public compressor
{
private:
  std::vector<std::shared_ptr<std::thread>> threads;

protected:
  astcenc_preset   preset;
  astcenc_swizzle  swz_encode;
  astcenc_swizzle  swz_decode;
                   
  std::uint16_t    padding;
                   
  astcenc_config                config;
  astcenc_error                 status;
  std::vector<astcenc_context*> contexts;

public:
  astc();

  // Prints an overview of the settings in the current config.
  void print_settings();

  /* =============================================================

                  General Compression Options

     ============================================================= */

  // Allocates a new context on all settings that have been set so far.
  void apply_all_settings();

  // Delets the allocated memory of the contexts
  void free_contexts();

  // Uploads a customized config-file to the class, replacing the current.
  void set_astc_threads(unsigned int thread_count);

  // Returns the config-file of the class (e.g. for modifications).
  astcenc_config get_astc_config();

  // Uploads a customized config-file to the class, replacing the current.
  void set_astc_config(astcenc_config new_config);

  // Uploads a custom preset to the class.
  // Presets: ASTCENC_PRE_FAST, MEDIUM, THOROUGH, EXHAUSTIVE
  // Attention: This method may reset settings you set! Call at first if you need to.
  void set_preset(astcenc_preset custom_preset);

  // Uploads a custom profile to the current config.
  // Profiles: ASTCENC_PRF_LDR_SRGB, LDR, HDR_RGB_LDR_A, HDR
  void set_profile(astcenc_profile new_profile);

  // Upload custom block sizes to the current config.
  // Supported 2D block sizes are:
  //
  //             4x4 : 8.00 bpp        10x5 : 2.56 bpp
  //             5x4 : 6.40 bpp        10x6 : 2.13 bpp
  //             5x5 : 5.12 bpp         8x8 : 2.00 bpp
  //             6x5 : 4.27 bpp        10x8 : 1.60 bpp
  //             6x6 : 3.56 bpp       10x10 : 1.28 bpp
  //             8x5 : 3.20 bpp       12x10 : 1.07 bpp
  //             8x6 : 2.67 bpp       12x12 : 0.89 bpp
  //             
  // Supported 3D block sizes are :
  //             
  //             3x3x3 : 4.74 bpp       5x5x4 : 1.28 bpp
  //             4x3x3 : 3.56 bpp       5x5x5 : 1.02 bpp
  //             4x4x3 : 2.67 bpp       6x5x5 : 0.85 bpp
  //             4x4x4 : 2.00 bpp       6x6x5 : 0.71 bpp
  //             5x4x4 : 1.60 bpp       6x6x6 : 0.59 bpp
  // Block sizes will not be tested for correctness beforehand.
  void set_blocksizes(unsigned int block_x, unsigned int block_y, unsigned int block_z);

  // Upload custom flags to the current config.
  // These are additional compressor options for certain image data types.
  // Flags: ASTCENC_FLG_MAP_NORMAL, ASTCENC_FLG_USE_PERCEPTUAL, ASTCENC_FLG_MAP_MASK
  void set_flags(unsigned int new_flags);

  // Edit the flags of the current config.
  // Additional compressor options.
  // normal map:   Input texture is 3 channel normal map, storing unit length normals as
  //               R=X, G=Y, B=Z, optimized for angular PSNR.
  //               Compressor will only compress X+Y with RGB=X, A=Y.
  //               You need to recover Z in the shader.
  // perceptual:   Codec should optimize perceptual error instead of direct RMS error.
  //               Aims to improve image quality, but lowers PSNR score.
  //               Only available to normal maps.
  // mask:         Input texture is mask texture with unrelated data stored in the color channels.
  //               Improves image quality by trying to minimize effect of error cross-talk across channels.
  void edit_flags(bool normal, bool perceptual, bool mask);


  /* =============================================================

                      Error Weighting Options

     ============================================================= */

  // Compute the per-texel relative error weighting for the RGB color channels as follows:
  // weight = 1 / (<base> + <mean> * mean^2 + <stdev> * stdev^2)
  // <mix>:     controls the degree of mixing of the average and stddev error components across the color cahnnels.
  //            = 0: causes the computation to be done completely separately for each color channel
  //            = 1: causes the results from the RGB channels to be combined and applied to all three together.
  // <power>:   power used to raise the values of the input texels before computing averageand standard deviation.
  // <radius>:  specifies the texel radius of the neighborhood over which the averageand standard deviation are computed.
  void edit_rgb_error_weighting(int radius, float power, float base, float mean, float stdev, float mix);

  // Compute the per-texel relative error weighting for the alpha channel, when used in conjunction with rgb_error_weighting.
  void edit_alpha_error_weighting(float power, float base, float mean, float stdev);

  // Scale per-texel weights by the alpha value.
  // The alpha-value for scaling a particular texel is the average of all alpha-values within radius.
  void edit_alpha_scaling(int radius);

  // Additional weight scaling to each color channel, allowing them to be treated differently in terms of error significance.
  // > 1: Increase significane
  // < 1: decrease significance
  // = 0: exclude from error computation.
  void edit_rgba_scaling(float red, float green, float blue, float alpha);

  // Additional weight scaling for texels at compression block edges and corners.
  // > 1: Increase significance of texels close to the edges of a block (helps to reduce block artifacts)
  void edit_edge_scaling(float edge_scale);

  // Test only <limit> block partitions.
  // Higher numbers => better quality, however diminishing results for smaller block sizes.
  // Defaults: Fast 4, Medium 25, Thorough 100, Exhaustive 1024
  void edit_blockmode_limit(int limit);

  // Test only block modes below the <limit> usage centile in an distribution of block mode frequency.
  // Defaults: Fast 50, Medium 75, Thorough 95, Exhaustive 100
  void edit_partition_limit(int limit);

  // Iterate only <limit> refinement iterations on colors and weights. Minimum value is 1.
  // Defaults: Fast 1, Medium 2, Thorough 4, Exhaustive 4
  void edit_refinement_limit(int limit);

  // Stop compression work on a block as soon as the PSNR of the block, measured in dB, exceeds <limit>.
  // Defaults: (N is number of texels in a block)
  // Fast MAX(63-19*log10(N),  85-35*log10(N)), Medium MAX(70-19*log10(N),  95-35*log10(N)),
  // Thorough MAX(77-19*log10(N), 105-35*log10(N)), Exhaustive 999
  void edit_db_limit(float limit);

  // Stop compression work on a block after only testing blocks with up to two partitions and one plane of weights,
  // according to some calculations on <factor>.
  // Defaults: Fast 1.0, Medium 1.2, Thorough 2.5, Exhaustive 1000.0
  void edit_partitionearly_limit(float factor);

  // Stop compression after testing only one plane of weights, unless minimum color correlation 
  // between any pair of color channels is below this <factor>.
  // Defaults: Fast 0.50, Medium 0.75, Thorough 0.95, Exhaustive 0.99
  void edit_planecor_limit(float factor);


  /* =============================================================

                         ASTC Compressor

     ============================================================= */

  // Compresses the given image and returns the result.
  // Does not manage threads.
  void call_astc_compressor(astcenc_image* img, astc_datatype* compressed_img, std::size_t compressed_img_size, std::uint16_t context_id);

  // Compresses a vector of images and returns the results in a vector.
  // This method manages threads and assigns one to each image to be compressed.
  // Multithreading only works for seperate images at this moment.
  jayComp<astc_datatype> compress(std::vector<astcenc_image*> source_imgs);

  // Decompresses a vector of compressed images and returns the results in a vector.
  std::vector<astcenc_image*> decompress(const jayComp<astc_datatype>& comp_imgs);


  /* =============================================================

                      ASTC Image Operations

   ============================================================= */

   // Allocates memory for an (uncompressed) astcenc_image with the given dimensions.
  astcenc_image* alloc_data(std::size_t dim_width, std::size_t dim_height, std::size_t dim_depth, std::size_t dim_pad, unsigned int bitness = 16);

  // Fills the padded space of an image.
  void fill_image_padding_area(astcenc_image* img);

  // Frees the allocated memory of an image.
  void free_image(astcenc_image* img);

  // Converts plain data into astcenc_image format (2D or 3D).
  // Source data:
  //    -> data_ptr:    pointer to first element of source
  //    -> grid:        underlying regular grid
  //    -> t/z_offset:  specifies starting time- and depth-level
  //    -> vec_len:     dimension of the elements in the grid
  //    -> vectorFirst: specifies if the data is sorted by vector (u,v,w), .. or by component (u, u', u''), ..
  // Normalization:
  //    Data should be normalized to the range [0.0, 1.0], it will be clamped during compression.
  //    These peaks are also needed for restoring the data in the shader.
  //    -> normalize:   switch to turn on/off normalization
  //    -> peaks:       vector containing min/max peaks of every depth layer (in this order)
  // Image options:
  //    -> color:       specifies how many color channels are available (R=1, .., RGBA=4)
  //    -> slice:       specifies if the image should be 3D or 2D
  //    -> padding:     specifies the amount of padding around the image
  template <typename T>
  astcenc_image* scan_sf16(
          T*                        data_ptr,
    const std::vector<std::size_t>& grid,
          std::size_t               vec_len,
          std::size_t               t_offset,
          std::size_t               z_offset,
          bool                      vectorFirst,
          bool                      normalize,
    const std::vector<float>&       peaks,
          colorspace                color,
          slicetype                 slice,
          std::size_t               padding = 0
  )
  {
    assert(int(color_setting) > 0);

    

    // This may not the actual dimension of the grid (if the vector is filled up artificially), but this is fine
    // (if it has been filled up with 1)
    const std::size_t& grid_dim = grid.size();

    assert(grid_dim >= int(slice));

    std::size_t grid_x = (grid_dim >= 1) ? grid[0] : 1;
    std::size_t grid_y = (grid_dim >= 2) ? grid[1] : 1;
    std::size_t grid_z = (grid_dim >= 3) ? grid[2] : 1;
    std::size_t grid_t = (grid_dim == 4) ? grid[3] : 1;

    if (grid_x == 0 || grid_y == 0 || grid_z == 0 || grid_t == 0)
    {
      std::cout << "Error: Grid-values must not be set to 0." << std::endl;
      return nullptr;
    }

    // Defined pixels reside on the x-axis (image width). These pixels are completely filled with regards to the chosen colorspace.
    std::size_t defined_pixels;
    // If the last pixel on the respective x-axis scanline cannot be filled with regards to the chosen colorspace, it is a half pixel.
    // There can be at most a single half pixel per x-axis scanline.
    std::size_t half_pixel;

    std::size_t img_w;
    std::size_t img_h;
    std::size_t img_d;

    // Prepare scan
    // ============
    if (vectorFirst)
    {
      defined_pixels = (grid_x * vec_len) / int(color);
      half_pixel     = (grid_x * vec_len) % int(color);

      img_w =                      defined_pixels + (half_pixel > 0)   ;
      img_h = (int(slice) >= 2) ?  grid_y                           : 1;
      img_d = (int(slice) >= 3) ? (grid_z - z_offset)               : 1;  // For volumetric images z should be 0, otherwise the first z depth-levels will be cut off
    }
    else
    {
      defined_pixels = grid_x / int(color);
      half_pixel     = grid_x % int(color);

      img_w =                      defined_pixels + (half_pixel > 0)   ;
      img_h = (int(slice) >= 2) ?  grid_y * vec_len                 : 1;
      img_d = (int(slice) >= 3) ? (grid_z - z_offset)               : 1;  // For volumetric images z should be 0, otherwise the first z depth-levels will be cut off
    }

    // Initialize the memory allocated by the image with 0
    astcenc_image* img = alloc_data(img_w, img_h, img_d, padding, 16);

    // Scan
    // ====
    std::uint16_t***   data16      = static_cast<std::uint16_t***>(img->data);
    T*                 addr        = data_ptr + (t_offset * grid_z * grid_y * grid_x * vec_len) + (z_offset * grid_y * grid_x * vec_len);
    std::size_t        addr_offset = 0;

    // DEPTH
    for (std::size_t d = 0; d < img_d; d++)
    {
      std::size_t d_dst = d + padding;

      // HEIGHT
      for (std::size_t h = 0; h < img_h; h++)
      {
        std::size_t h_dst = h + padding;

        // WIDTH
        for (std::size_t w = 0; w < defined_pixels; w++)
        {
          std::size_t w_dst = w + padding;

          // CHANNELS
          for (std::uint8_t i = 0; i < int(color); i++)
          {
            float val   = *(addr + addr_offset);        // src value
            float n_val = (normalize) ? normalize_val(val, peaks, t_offset * grid_z + z_offset + d) : val; // normalized value

            data16[d_dst][h_dst][4 * w_dst + i] = float_to_sf16(n_val, SF_NEARESTEVEN);

            addr_offset++;
          }

          // Alpha should be set to 1.0 if not otherwise utilized
          if (int(color) < 4)
            data16[d_dst][h_dst][4 * w_dst + 3] = 0x3C00; // 1.0 in SF16 notation
        }

        // HALF PIXEL
        if (half_pixel)
        {
          std::size_t w     = defined_pixels;
          std::size_t w_dst = w + padding;

          // CHANNELS
          for (std::uint16_t k = 0; k < half_pixel; k++)
          {
            float val   = *(addr + addr_offset);       // src value
            float n_val = (normalize) ? normalize_val(val, peaks, t_offset * grid_z + z_offset + d) : val; // normalized value

            data16[d_dst][h_dst][4 * w_dst + k] = float_to_sf16(n_val, SF_NEARESTEVEN);

            addr_offset++;
          }

          // Alpha should be set to 1.0
          data16[d_dst][h_dst][4 * w_dst + 3] = 0x3C00; // 1.0 in SF16 notation
        }
      }
    }
    return img;
  }

  // Same as scan_sf16 but only with normalization per component.
  // And not for glm::vec types.
  template <typename T>
  astcenc_image* scan_sf16_refined(
    T* data_ptr,
    const std::vector<std::size_t>& grid,
    std::size_t               vec_len,
    std::size_t               t_offset,
    std::size_t               z_offset,
    bool                      vectorFirst,
    bool                      normalize,
    const std::vector<float>& peaks,
    colorspace                color,
    slicetype                 slice,
    std::size_t               padding = 0
  )
  {
    assert(int(color_setting) > 0);



    // This may not the actual dimension of the grid (if the vector is filled up artificially), but this is fine
    // (if it has been filled up with 1)
    const std::size_t& grid_dim = grid.size();

    assert(grid_dim >= int(slice));

    std::size_t grid_x = (grid_dim >= 1) ? grid[0] : 1;
    std::size_t grid_y = (grid_dim >= 2) ? grid[1] : 1;
    std::size_t grid_z = (grid_dim >= 3) ? grid[2] : 1;
    std::size_t grid_t = (grid_dim == 4) ? grid[3] : 1;

    if (grid_x == 0 || grid_y == 0 || grid_z == 0 || grid_t == 0)
    {
      std::cout << "Error: Grid-values must not be set to 0." << std::endl;
      return nullptr;
    }

    // Defined pixels reside on the x-axis (image width). These pixels are completely filled with regards to the chosen colorspace.
    std::size_t defined_pixels;
    // If the last pixel on the respective x-axis scanline cannot be filled with regards to the chosen colorspace, it is a half pixel.
    // There can be at most a single half pixel per x-axis scanline.
    std::size_t half_pixel;

    std::size_t img_w;
    std::size_t img_h;
    std::size_t img_d;

    // Prepare scan
    // ============
    if (vectorFirst)
    {
      defined_pixels = (grid_x * vec_len) / int(color);
      half_pixel = (grid_x * vec_len) % int(color);

      img_w = defined_pixels + (half_pixel > 0);
      img_h = (int(slice) >= 2) ? grid_y : 1;
      img_d = (int(slice) >= 3) ? (grid_z - z_offset) : 1;  // For volumetric images z should be 0, otherwise the first z depth-levels will be cut off
    }
    else
    {
      defined_pixels = grid_x / int(color);
      half_pixel = grid_x % int(color);

      img_w = defined_pixels + (half_pixel > 0);
      img_h = (int(slice) >= 2) ? grid_y * vec_len : 1;
      img_d = (int(slice) >= 3) ? (grid_z - z_offset) : 1;  // For volumetric images z should be 0, otherwise the first z depth-levels will be cut off
    }

    // Initialize the memory allocated by the image with 0
    astcenc_image* img = alloc_data(img_w, img_h, img_d, padding, 16);

    // Scan
    // ====
    std::uint16_t*** data16 = static_cast<std::uint16_t***>(img->data);
    T* addr = data_ptr + (t_offset * grid_z * grid_y * grid_x * vec_len) + (z_offset * grid_y * grid_x * vec_len);
    std::size_t        addr_offset = 0;

    // DEPTH
    for (std::size_t d = 0; d < img_d; d++)
    {
      std::size_t d_dst = d + padding;

      // HEIGHT
      for (std::size_t h = 0; h < img_h; h++)
      {
        std::size_t h_dst = h + padding;

        // WIDTH
        for (std::size_t w = 0; w < defined_pixels; w++)
        {
          std::size_t w_dst = w + padding;

          // CHANNELS
          for (std::uint8_t i = 0; i < int(color); i++)
          {
            int component = addr_offset % vec_len;
            float val = *(addr + addr_offset);        // src value
            float n_val = (normalize) ? normalize_val_per_component(val, vec_len, peaks, t_offset * grid_z + z_offset + d, component) : val; // normalized value

            data16[d_dst][h_dst][4 * w_dst + i] = float_to_sf16(n_val, SF_NEARESTEVEN);

            addr_offset++;
          }

          // Alpha should be set to 1.0 if not otherwise utilized
          if (int(color) < 4)
            data16[d_dst][h_dst][4 * w_dst + 3] = 0x3C00; // 1.0 in SF16 notation
        }

        // HALF PIXEL
        if (half_pixel)
        {
          std::size_t w = defined_pixels;
          std::size_t w_dst = w + padding;

          // CHANNELS
          for (std::uint16_t k = 0; k < half_pixel; k++)
          {
            int component = addr_offset % vec_len;
            float val = *(addr + addr_offset);       // src value
            float n_val = (normalize) ? normalize_val_per_component(val, vec_len, peaks, t_offset * grid_z + z_offset + d, component) : val; // normalized value

            data16[d_dst][h_dst][4 * w_dst + k] = float_to_sf16(n_val, SF_NEARESTEVEN);

            addr_offset++;
          }

          // Alpha should be set to 1.0
          data16[d_dst][h_dst][4 * w_dst + 3] = 0x3C00; // 1.0 in SF16 notation
        }
      }
    }
    return img;
  }

  // Converts plain data into astcenc_image format (2D or 3D).
  // Source data:
  //    -> data_ptr:    pointer to first element of source
  //    -> grid:        underlying regular grid
  //    -> t/z_offset:  specifies starting time- and depth-level
  //    -> vec_len:     dimension of the elements in the grid
  //    -> vectorFirst: Specifies if the data is sorted by vector (u,v,w), .. or by component (u, u', u''), ..
  // Normalization:
  //    Data should be normalized to the range [0.0, 1.0], it will be clamped during compression.
  //    These peaks are also needed for restoring the data in the shader.
  //    -> normalize:   switch to turn on/off normalization
  //    -> peaks:       vector containing min/max peaks of every depth layer (in this order)
  // Image options:
  //    -> color:       specifies how many color channels are available (R=1, .., RGBA=4)
  //    -> slice:       specifies if the image should be 3D or 2D
  //    -> padding:     specifies the amount of padding around the image
  template <typename T, int i>
  astcenc_image* scan_sf16(
          glm::vec<i, T>*           data_ptr,
    const std::vector<std::size_t>& grid,
          std::size_t               vec_len,
          std::size_t               t_offset,
          std::size_t               z_offset,
          bool                      vectorFirst,
          bool                      normalize,
    const std::vector<float>&       peaks,
          colorspace                color,
          slicetype                 slice,
          std::size_t               padding = 0
  )
  {
    assert(int(color_setting) > 0);



    // This may not the actual dimension of the grid (if the vector is filled up artificially), but this is fine
    // (if it has been filled up with 1)
    const std::size_t grid_dim = grid.size();

    assert(grid_dim >= int(slice));

    std::size_t grid_x         = (grid_dim >= 1) ? grid[0] : 1      ;
    std::size_t grid_y         = (grid_dim >= 2) ? grid[1] : 1      ;
    std::size_t grid_z         = (grid_dim >= 3) ? grid[2] : 1      ;
    std::size_t grid_t         = (grid_dim == 4) ? grid[3] : 1      ;
    std::size_t max_components = (i < vec_len)   ? i       : vec_len;


    if (grid_x == 0 || grid_y == 0 || grid_z == 0 || grid_t == 0)
    {
      std::cout << "Error: Grid-values must not be set to 0." << std::endl;
      return nullptr;
    }

    // Defined pixels reside on the x-axis (image width). These pixels are completely filled with regards to the chosen colorspace.
    std::size_t defined_pixels;
    // If the last pixel on the respective x-axis scanline cannot be filled with regards to the chosen colorspace, it is a half pixel.
    // There can be at most a single half pixel per x-axis scanline.
    std::size_t half_pixel;

    std::size_t img_w;
    std::size_t img_h;
    std::size_t img_d;

    // Prepare scan
    // ============
    if (vectorFirst)
    {
      defined_pixels = (grid_x * max_components) / int(color);
      half_pixel     = (grid_x * max_components) % int(color);

      img_w = defined_pixels + (half_pixel > 0)          ;
      img_h = (int(slice) >= 2) ? grid_y              : 1;
      img_d = (int(slice) >= 3) ? (grid_z - z_offset) : 1;  // For volumetric images z should be 0, otherwise the first z depth-levels will be cut off
    }
    else
    {
      defined_pixels = grid_x / int(color);
      half_pixel     = grid_x % int(color);

      img_w = defined_pixels + (half_pixel > 0);
      img_h = (int(slice) >= 2) ? grid_y * max_components : 1;
      img_d = (int(slice) >= 3) ? (grid_z - z_offset)     : 1;  // For volumetric images z should be 0, otherwise the first z depth-levels will be cut off
    }

    // Initialize the memory allocated by the image with 0
    astcenc_image* img = alloc_data(img_w, img_h, img_d, padding, 16);

    // Scan
    // ====
    std::uint16_t*** data16      = static_cast<std::uint16_t***>(img->data);
    glm::vec<i, T>*  addr        = data_ptr + (t_offset * grid_z * grid_y * grid_x) + (z_offset * grid_y * grid_x);
    std::size_t      addr_offset = 0;
    std::size_t      component   = 0;

    // DEPTH
    for (std::size_t d = 0; d < img_d; d++)
    {
      std::size_t d_dst = d + padding;

      // HEIGHT
      for (std::size_t h = 0; h < img_h; h++)
      {
        std::size_t h_dst = h + padding;

        // WIDTH
        for (std::size_t w = 0; w < defined_pixels; w++)
        {
          std::size_t w_dst = w + padding;

          // CHANNELS
          for (std::uint8_t i = 0; i < int(color); i++)
          {
            float val   = (*(addr + addr_offset))[component];  // src value
            float n_val = (normalize) ? normalize_val(val, peaks, t_offset * grid_z + z_offset) : val; // normalized value

            data16[d_dst][h_dst][4 * w_dst + i] = float_to_sf16(n_val, SF_NEARESTEVEN);

            // First process every set vector component before incrementing to the next vector
            component = (component + 1) % max_components;
            if (component == 0)
              addr_offset++;
          }

          // Alpha should be set to 1.0 if not otherwise utilized
          if (int(color) < 4)
            data16[d_dst][h_dst][4 * w_dst + 3] = 0x3C00; // 1.0 in SF16 notation
        }

        // HALF PIXEL
        if (half_pixel)
        {
          std::size_t w     = defined_pixels;
          std::size_t w_dst = w + padding;

          // CHANNELS
          for (std::uint16_t k = 0; k < half_pixel; k++)
          {
            float val   = (*(addr + addr_offset))[component];   // src value
            float n_val = (normalize) ? normalize_val(val, peaks, t_offset * grid_z + z_offset) : val; // normalized value

            data16[d_dst][h_dst][4 * w_dst + k] = float_to_sf16(n_val, SF_NEARESTEVEN);

            // First process every set vector component before incrementing to the next vector
            component = (component + 1) % vec_len;
            if (component == 0)
              addr_offset++;
          }

          // Alpha should be set to 1.0
          data16[d_dst][h_dst][4 * w_dst + 3] = 0x3C00; // 1.0 in SF16 notation
        }
      }
    }
    return img;
  }


  std::vector<float> scan_f32(
    astcenc_image*                  astc_img,
    const std::vector<std::size_t>& grid,
    std::size_t                     vec_len,
    bool                            denormalize,
    const std::vector<float>&       peaks,
    std::size_t                     peaks_id,
    colorspace                      color = jay::colorspace::Unknown,
    std::size_t                     padding = 0
  )
  {
    // This may not the actual dimension of the grid (if the vector is filled up artificially), but this is fine
    // (if it has been filled up with 1)
    const std::size_t& grid_dim = grid.size();

    std::size_t grid_x = (grid_dim >= 1) ? grid[0] : 1;
    std::size_t grid_y = (grid_dim >= 2) ? grid[1] : 1;
    std::size_t grid_z = (grid_dim >= 3) ? grid[2] : 1;
    std::size_t grid_t = (grid_dim == 4) ? grid[3] : 1;
    std::uint8_t colors = (color != colorspace::Unknown) ? int(color) : 4;

    if (grid_x == 0 || grid_y == 0 || grid_z == 0 || grid_t == 0)
    {
      std::cout << "Error: Grid-values must not be set to 0." << std::endl;
      return {};
    }

    // Scan
    // ====
    std::vector<float> f32_img(astc_img->dim_x * astc_img->dim_y * astc_img->dim_z * vec_len);
    std::uint16_t***   data16 = static_cast<std::uint16_t***>(astc_img->data);
    std::size_t        offset = 0;

    // DEPTH
    for (std::size_t d = 0; d < astc_img->dim_z; d++)
    {
      std::size_t d_dst = d + padding;

      // HEIGHT
      for (std::size_t h = 0; h < astc_img->dim_y; h++)
      {
        std::size_t h_dst = h + padding;

        // WIDTH
        for (std::size_t w = 0; w < astc_img->dim_x; w++)
        {
          std::size_t w_dst = w + padding;

          // CHANNELS
          for (std::uint8_t i = 0; i < colors; i++)
          {
            float val = sf16_to_float(data16[d_dst][h_dst][4 * w_dst + i]); // normalized sf16 value
            float n_val = (denormalize) ? denormalize_float(val, peaks, peaks_id) : val; // denormalized float32 value

            f32_img[offset] = n_val;

            offset++;
          }
        }
      }
    }

    return f32_img;
  }

  std::vector<float> scan_f32_refined(
    astcenc_image* astc_img,
    const std::vector<std::size_t>& grid,
    std::size_t                     vec_len,
    bool                            denormalize,
    const std::vector<float>& peaks,
    std::size_t                     peaks_id,
    colorspace                      color = jay::colorspace::Unknown,
    std::size_t                     padding = 0
  )
  {
    // This may not the actual dimension of the grid (if the vector is filled up artificially), but this is fine
    // (if it has been filled up with 1)
    const std::size_t& grid_dim = grid.size();

    std::size_t grid_x = (grid_dim >= 1) ? grid[0] : 1;
    std::size_t grid_y = (grid_dim >= 2) ? grid[1] : 1;
    std::size_t grid_z = (grid_dim >= 3) ? grid[2] : 1;
    std::size_t grid_t = (grid_dim == 4) ? grid[3] : 1;
    std::uint8_t colors = (color != colorspace::Unknown) ? int(color) : 4;

    if (grid_x == 0 || grid_y == 0 || grid_z == 0 || grid_t == 0)
    {
      std::cout << "Error: Grid-values must not be set to 0." << std::endl;
      return {};
    }

    // Scan
    // ====
    std::vector<float> f32_img(astc_img->dim_x * astc_img->dim_y * astc_img->dim_z * vec_len);
    std::uint16_t*** data16 = static_cast<std::uint16_t***>(astc_img->data);
    std::size_t        offset = 0;

    // DEPTH
    for (std::size_t d = 0; d < astc_img->dim_z; d++)
    {
      std::size_t d_dst = d + padding;

      // HEIGHT
      for (std::size_t h = 0; h < astc_img->dim_y; h++)
      {
        std::size_t h_dst = h + padding;

        // WIDTH
        for (std::size_t w = 0; w < astc_img->dim_x; w++)
        {
          std::size_t w_dst = w + padding;

          // CHANNELS
          for (std::uint8_t i = 0; i < colors; i++)
          {
            int component = offset % vec_len;
            float val = sf16_to_float(data16[d_dst][h_dst][4 * w_dst + i]); // normalized sf16 value
            float n_val = (denormalize) ? denormalize_per_component(val, vec_len, peaks, peaks_id, component) : val; // denormalized float32 value

            f32_img[offset] = n_val;

            offset++;
          }
        }
      }
    }

    return f32_img;
  }

  // Converts a complete dataset into a vector of astcenc_images (2D or 3D).
  // Source data:
  //    -> data_ptr:    pointer to first element of source
  //    -> grid:        underlying regular grid
  //    -> vec_len:     dimension of the elements in the grid
  //    -> vectorFirst: Specifies if the data is sorted by vector (u,v,w), .. or by component (u, u', u''), ..
  // Normalization:
  //    Data should be normalized to the range [0.0, 1.0], it will be clamped during compression.
  //    These peaks are also needed for restoring the data in the shader.
  //    -> normalize:   switch to turn on/off normalization
  //    -> peaks:       vector containing min/max peaks of every depth layer (in this order)
  // Image options:
  //    -> color:       specifies how many color channels are available (R=1, .., RGBA=4)
  //    -> slice:       specifies if the image should be 3D or 2D
  //    -> padding:     specifies the amount of padding around the image
  template <typename T>
  std::vector<astcenc_image*> convert_data_to_img(
          T*                        data_ptr,
    const std::vector<std::size_t>& grid,
          std::size_t               vec_len,
          bool                      vectorFirst,
          bool                      normalize,
    const std::vector<float>&       peaks,
          colorspace                color,
          slicetype                 slice,
          std::size_t               padding = 0
  )
  {
    // This may not be the actual grid_dim, but its fine
    const grid_dim = grid.size();

    std::size_t  grid_z = (grid_dim >= 3) ? grid[2] : 1;
    std::size_t  grid_t = (grid_dim == 4) ? grid[3] : 1;

    if (slice == slicetype::Plane)
    {
      std::vector<astcenc_image*> astc_data(grid_t * grid_z);

      for (std::size_t t = 0; t < grid_t; t++)
        for (std::size_t z = 0; z < grid_z; z++)
          astc_data[t * grid_z + z] = scan_sf16(data_ptr, t, z, grid, vec_len, peaks, color, slice, padding, vectorFirst, normalize);

      return astc_data;
    }
    else // if (this->slice_setting == slicetype::Volume)
    {
      std::vector<astcenc_image*> astc_data(grid_t);

      for (std::size_t t = 0; t < grid_t; t++)
        astc_data[t * grid_z + z] = scan_sf16(data_ptr, t, z, grid, vec_len, peaks, color, slice, padding, vectorFirst, normalize);

      return astc_data;
    }

    return {};
  }

  // Converts a complete dataset into a vector of astcenc_images (2D or 3D). (preferred method)
  // Source data:
  //    -> data:        data container including a pointer to the first element of the dataset,
  //                    the underlying regular grid, the dimension of the grids elements
  //                    and a value specifying the ordering of the data.
  // Normalization:
  //    Data should be normalized to the range [0.0, 1.0], it will be clamped during compression.
  //    These peaks are also needed for restoring the data in the shader.
  //    -> normalize:   switch to turn on/off normalization
  //    -> peaks:       vector containing min/max peaks of every depth layer (in this order)
  // Image options:
  //    -> color:       specifies how many color channels are available (R=1, .., RGBA=4)
  //    -> slice:       specifies if the image should be 3D or 2D
  //    -> padding:     specifies the amount of padding around the image
  template <typename T>
  std::vector<astcenc_image*> convert_data_to_img(
          jaySrc<T>&              data,
          bool                      normalize,
    const std::vector<float>&       peaks,
          colorspace                color,
          slicetype                 slice,
          std::size_t               padding = 0
  )
  {
    // This may not be the actual grid_dim, but its fine
    const auto grid_dim = data.grid.size();

    std::size_t  max_z = (grid_dim >= 3) ? data.grid[2] : 1;
    std::size_t  max_t = (grid_dim == 4) ? data.grid[3] : 1;

    // Don't iterate through multiple depth-offsets if you want to get a volumetric image
    if (slice == slicetype::Volume)
      max_z = 1;

    std::vector<astcenc_image*> astc_data(max_t * max_z);

    for (std::size_t t = 0; t < max_t; t++)
      for (std::size_t z = 0; z < max_z; z++)
        astc_data[t * max_z + z] = scan_sf16(data.data.data(), data.grid, data.vec_len, t, z, data.ordering == Order::VectorFirst, normalize, peaks, color, slice, padding);

    return astc_data;
  }

  template <typename T>
  std::vector<astcenc_image*> convert_data_to_img_refined(
    jaySrc<T>& data,
    bool                      normalize,
    const std::vector<float>& peaks,
    colorspace                color,
    slicetype                 slice,
    std::size_t               padding = 0
  )
  {
    // This may not be the actual grid_dim, but its fine
    const auto grid_dim = data.grid.size();

    std::size_t  max_z = (grid_dim >= 3) ? data.grid[2] : 1;
    std::size_t  max_t = (grid_dim == 4) ? data.grid[3] : 1;

    // Don't iterate through multiple depth-offsets if you want to get a volumetric image
    if (slice == slicetype::Volume)
      max_z = 1;

    std::vector<astcenc_image*> astc_data(max_t * max_z);

    for (std::size_t t = 0; t < max_t; t++)
      for (std::size_t z = 0; z < max_z; z++)
        astc_data[t * max_z + z] = scan_sf16_refined(data.data.data(), data.grid, data.vec_len, t, z, data.ordering == Order::VectorFirst, normalize, peaks, color, slice, padding);

    return astc_data;
  }


  std::vector<float> convert_img_to_data(
    std::vector<astcenc_image*> imgs,
    std::vector<std::size_t>    grid,
    std::size_t                 vec_len,
    bool                        denormalize,
    const std::vector<float>&   peaks,
    colorspace                  color,
    std::size_t                 padding = 0
  )
  {
    // This may not be the actual grid_dim, but its fine
    const auto grid_dim = grid.size();

    std::size_t  grid_x =  grid[0];
    std::size_t  grid_y = (grid_dim >= 2) ? grid[1] : 1;
    std::size_t  grid_z = (grid_dim >= 3) ? grid[2] : 1;
    std::size_t  grid_t = (grid_dim == 4) ? grid[3] : 1;

    std::vector<float> f32_data;

    for (std::size_t i = 0;i < imgs.size(); i++)
    {
      auto f32_img = scan_f32(imgs[i], grid, vec_len, denormalize, peaks, i, color, padding);
      f32_data.insert(f32_data.end(), std::make_move_iterator(f32_img.begin()), std::make_move_iterator(f32_img.end()));
    }

    return f32_data;
  }

  std::vector<float> convert_img_to_data_refined(
    std::vector<astcenc_image*> imgs,
    std::vector<std::size_t>    grid,
    std::size_t                 vec_len,
    bool                        denormalize,
    const std::vector<float>& peaks,
    colorspace                  color,
    std::size_t                 padding = 0
  )
  {
    // This may not be the actual grid_dim, but its fine
    const auto grid_dim = grid.size();

    std::size_t  grid_x = grid[0];
    std::size_t  grid_y = (grid_dim >= 2) ? grid[1] : 1;
    std::size_t  grid_z = (grid_dim >= 3) ? grid[2] : 1;
    std::size_t  grid_t = (grid_dim == 4) ? grid[3] : 1;

    std::vector<float> f32_data;

    for (std::size_t i = 0; i < imgs.size(); i++)
    {
      auto f32_img = scan_f32_refined(imgs[i], grid, vec_len, denormalize, peaks, i, color, padding);
      f32_data.insert(f32_data.end(), std::make_move_iterator(f32_img.begin()), std::make_move_iterator(f32_img.end()));
    }

    return f32_data;
  }
};
}

#endif