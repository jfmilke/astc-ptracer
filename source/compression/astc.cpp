#include <chrono>

#include <astcenc_internal.h>
#include <astcenc_mathlib.h>

#include <jay/compression/astc.hpp>
#include <jay/io/image_io.hpp>

namespace jay
{
  astc::astc()
    : preset         { astcenc_preset::ASTCENC_PRE_FAST }
    , swz_encode     { ASTCENC_SWZ_R, ASTCENC_SWZ_G, ASTCENC_SWZ_B, ASTCENC_SWZ_A }
    , swz_decode     { ASTCENC_SWZ_R, ASTCENC_SWZ_G, ASTCENC_SWZ_B, ASTCENC_SWZ_A }
    , padding        { 0 }
    , config         { }
    , status         { }
    , contexts       (get_pyhsical_cpu_cores())
    , threads        (get_pyhsical_cpu_cores())
  {
    // Initialize astc compressor for highest bitrate and fast compression.
    auto profile = astcenc_profile::ASTCENC_PRF_LDR;
    auto block_x = 4;
    auto block_y = 4;
    auto block_z = 1;
    auto flags   = 0;

    status  = astcenc_config_init(profile, block_x, block_y, block_z, preset, flags, config);
    for (auto& context : contexts)
      status  = astcenc_context_alloc(config, 1, &context);

    color_setting = colorspace::RGB;
    slice_setting = slicetype::Plane;
  }


  void astc::print_settings()
  {
    printf("Compressor settings\n");
    printf("===================\n\n");

    switch (config.profile)
    {
    case ASTCENC_PRF_LDR:
      printf("    Color profile:              LDR linear\n");
      break;
    case ASTCENC_PRF_LDR_SRGB:
      printf("    Color profile:              LDR sRGB\n");
      break;
    case ASTCENC_PRF_HDR_RGB_LDR_A:
      printf("    Color profile:              HDR RGB + LDR A\n");
      break;
    case ASTCENC_PRF_HDR:
      printf("    Color profile:              HDR RGBA\n");
      break;
    }

    if (config.block_z == 1)
    {
      printf("    Block size:                 %ux%u\n", config.block_x, config.block_y);
    }
    else
    {
      printf("    Block size:                 %ux%ux%u\n", config.block_x, config.block_y, config.block_z);
    }

    printf("    Bitrate:                    %3.2f bpp\n", 128.0 / (config.block_x * config.block_y * config.block_z));

    printf("    Radius mean/stdev:          %u texels\n", config.v_rgba_radius);
    printf("    RGB power:                  %g\n", (double)config.v_rgb_power);
    printf("    RGB base weight:            %g\n", (double)config.v_rgb_base);
    printf("    RGB mean weight:            %g\n", (double)config.v_rgb_mean);
    printf("    RGB stdev weight:           %g\n", (double)config.v_rgba_mean_stdev_mix);
    printf("    RGB mean/stdev mixing:      %g\n", (double)config.v_rgba_mean_stdev_mix);
    printf("    Alpha power:                %g\n", (double)config.v_a_power);
    printf("    Alpha base weight:          %g\n", (double)config.v_a_base);
    printf("    Alpha mean weight:          %g\n", (double)config.v_a_mean);
    printf("    Alpha stdev weight:         %g\n", (double)config.v_a_stdev);
    printf("    RGB alpha scale weight:     %d\n", (config.flags & ASTCENC_FLG_MAP_NORMAL));
    if ((config.flags & ASTCENC_FLG_MAP_NORMAL))
    {
      printf("    Radius RGB alpha scale:     %u texels\n", config.a_scale_radius);
    }

    printf("    R channel weight:           %g\n", (double)config.cw_r_weight);
    printf("    G channel weight:           %g\n", (double)config.cw_g_weight);
    printf("    B channel weight:           %g\n", (double)config.cw_b_weight);
    printf("    A channel weight:           %g\n", (double)config.cw_a_weight);
    printf("    Deblock artifact setting:   %g\n", (double)config.b_deblock_weight);
    printf("    Block partition cutoff:     %u partitions\n", config.tune_partition_limit);
    printf("    PSNR cutoff:                %g dB\n", (double)config.tune_db_limit);
    printf("    1->2 partition cutoff:      %g\n", (double)config.tune_partition_early_out_limit);
    printf("    2 plane correlation cutoff: %g\n", (double)config.tune_two_plane_early_out_limit);
    printf("    Block mode centile cutoff:  %g%%\n", (double)(config.tune_block_mode_limit));
    printf("    Max refinement cutoff:      %u iterations\n", config.tune_refinement_limit);
    printf("    Compressor thread count:    %d\n", 1);
    printf("\n");
  }


  /* =============================================================

                General Compression Options

     ============================================================= */

  void astc::apply_all_settings()
  {
    free_contexts();
    for (auto& context : contexts)
      status = astcenc_context_alloc(config, 1, &context);
  }

  void astc::free_contexts()
  {
    for (auto& context : contexts)
      astcenc_context_free(context);
  }


  void astc::set_astc_threads(unsigned int thread_count)
  {
    // Have at most as many threads as cpu cores
    int thread_change = (thread_count < get_pyhsical_cpu_cores()) ? thread_count - contexts.size() : get_pyhsical_cpu_cores();

    if (thread_change == 0)
      return;

    else if (thread_change < 0)
      for (int i = contexts.size(); i > thread_count; i--)
      {
        contexts.erase(contexts.end());
        threads.erase(threads.end());
      }

    else
      for (int i = contexts.size(); i < thread_count; i++)
      {
        contexts.push_back(nullptr);
        threads.push_back(nullptr);
        status = astcenc_context_alloc(config, 1, &contexts[i]);
      }
  }



  astcenc_config astc::get_astc_config()
  {
    return config;
  }


  void astc::set_astc_config(astcenc_config custom_config)
  {
    config = custom_config;
    if (config.block_z > 1)
      slice_setting = slicetype::Volume;
    else
      slice_setting = slicetype::Plane;
  }


  void astc::set_preset(astcenc_preset custom_preset)
  {
    preset = custom_preset;
    status = astcenc_config_init(config.profile, config.block_x, config.block_y, config.block_z, preset, config.flags, config);

    apply_all_settings();
  }


  void astc::set_profile(astcenc_profile custom_profile)
  {
    config.profile = custom_profile;
  }


  void astc::set_blocksizes(
    unsigned int block_x,
    unsigned int block_y,
    unsigned int block_z
  )
  {
    config.block_x = block_x;
    config.block_y = block_y;
    config.block_z = (block_z != 0) ? block_z : 1;

    if (block_z > 1)
      slice_setting = slicetype::Volume;
    else
      slice_setting = slicetype::Plane;
  }


  void astc::set_flags(unsigned int custom_flags)
  {
    config.flags = custom_flags;
  }


  void astc::edit_flags(
    bool normal,
    bool perceptual,
    bool mask
  )
  {
    /* alpha weight is currently not utilized by astc-encoder
    if (alpha_weight)
    {
      config.flags |= ASTCENC_FLG_USE_ALPHA_WEIGHT;
    }
    else
    {
      config.flags &= ~ASTCENC_FLG_USE_ALPHA_WEIGHT;
    }
    */

    if (normal)
    {
      config.flags |= ASTCENC_FLG_MAP_NORMAL;
      swz_encode = { ASTCENC_SWZ_R, ASTCENC_SWZ_R, ASTCENC_SWZ_R, ASTCENC_SWZ_G };
      std::cout << "Set encoding swizzle: RRRG" << std::endl;
      swz_decode = { ASTCENC_SWZ_R, ASTCENC_SWZ_A, ASTCENC_SWZ_Z, ASTCENC_SWZ_1 };
      std::cout << "Set decoding swizzle: RAZ1" << std::endl;
    }
    else
    {
      config.flags &= ~ASTCENC_FLG_MAP_NORMAL;
      swz_encode = { ASTCENC_SWZ_R, ASTCENC_SWZ_G, ASTCENC_SWZ_B, ASTCENC_SWZ_A };
      std::cout << "Set encoding swizzle: RGBA" << std::endl;
      swz_decode = { ASTCENC_SWZ_R, ASTCENC_SWZ_G, ASTCENC_SWZ_B, ASTCENC_SWZ_A };
      std::cout << "Set decoding swizzle: RGBA" << std::endl;
    }

    if (perceptual)
      if (config.flags & ASTCENC_FLG_MAP_NORMAL || normal)
        config.flags |= ASTCENC_FLG_USE_PERCEPTUAL;
      else
        std::cout << "Error: Could not set perceptual flag. You need to also enable normal flag." << std::endl;
    else
      config.flags &= ~ASTCENC_FLG_USE_PERCEPTUAL;

    if (mask)
      config.flags |= ASTCENC_FLG_MAP_MASK;
    else
      config.flags &= ~ASTCENC_FLG_MAP_MASK;
  }


  /* =============================================================

                    Error Weighting Options

     ============================================================= */

  void astc::edit_rgb_error_weighting(
    int radius,
    float power,
    float base,
    float mean,
    float stdev,
    float mix
  )
  {
    config.v_rgba_radius = radius;
    config.v_rgb_power = power;
    config.v_rgb_base = base;
    config.v_rgb_mean = mean;
    config.v_rgb_stdev = stdev;
    config.v_rgba_mean_stdev_mix = mix;
  }


  void astc::edit_alpha_error_weighting(
    float power,
    float base,
    float mean,
    float stdev
  )
  {
    config.v_rgb_power = power;
    config.v_rgb_base = base;
    config.v_rgb_mean = mean;
    config.v_rgb_stdev = stdev;
  }


  void astc::edit_alpha_scaling(int radius)
  {
    config.a_scale_radius = radius;

  }


  void astc::edit_rgba_scaling(
    float red,
    float green,
    float blue,
    float alpha
  )
  {
    config.cw_r_weight = red;
    config.cw_g_weight = green;
    config.cw_b_weight = blue;
    config.cw_a_weight = alpha;
  }


  void astc::edit_edge_scaling(float edge_scale)
  {
    config.b_deblock_weight = edge_scale;
  }


  void astc::edit_partition_limit(int limit)
  {
    config.tune_partition_limit = limit;
  }


  void astc::edit_blockmode_limit(int limit)
  {
    config.tune_block_mode_limit = limit;
  }


  void astc::edit_refinement_limit(int limit)
  {
    config.tune_refinement_limit = limit;
  }


  void astc::edit_db_limit(float limit)
  {
    config.tune_db_limit = limit;
  }


  void astc::edit_partitionearly_limit(float factor)
  {
    config.tune_partition_early_out_limit = factor;
  }


  void astc::edit_planecor_limit(float factor)
  {
    config.tune_two_plane_early_out_limit = factor;
  }


  /*
  
      std::size_t found_fileending = filename.find_last_of("_");
    std::string filename_plain;

    // Be sure to only take the base-filename without file-extension
    if (found_fileending == std::string::npos)
      filename_plain = filename;
    else
    {
      filename_plain = filename.substr(0, found_fileending);
    }

    std::size_t filesize;
    bool           searchForFiles = true;
    std::uint16_t  filenumber = 0;

    while (searchForFiles)
    {
      // This will increment for every slice within the file
      size_t addr_offset = 0;

      // Change the filename to the I/O-conention of jay
      std::string searchname = filename_plain + "_" + std::to_string(filenumber) + ".astc";
      std::ifstream searchfile(searchname, std::ios::in | std::ios::binary | std::ios::ate);

      // If file exists read all slices within it
      if (searchfile.is_open())
      {
        filesize = searchfile.tellg();
        searchfile.seekg(0, std::ios::beg);


        // Here comes some astcenc magic ...
        astc_header hdr;
        searchfile.read((char*)&hdr, sizeof(astc_header));
        addr_offset += sizeof(astc_header);

        unsigned int magicval = astc::unpack_bytes(hdr.magic[0], hdr.magic[1], hdr.magic[2], hdr.magic[3]);

        if (magicval != ASTC_MAGIC_ID)
        {
          printf("ERROR: File not recognized '%s'\n", filename);
          return;
        }

        unsigned int block_x = MAX(hdr.block_x, 1);
        unsigned int block_y = MAX(hdr.block_y, 1);
        unsigned int block_z = MAX(hdr.block_z, 1);

        unsigned int dim_x = astc::unpack_bytes(hdr.dim_x[0], hdr.dim_x[1], hdr.dim_x[2], 0);
        unsigned int dim_y = astc::unpack_bytes(hdr.dim_y[0], hdr.dim_y[1], hdr.dim_y[2], 0);
        unsigned int dim_z = astc::unpack_bytes(hdr.dim_z[0], hdr.dim_z[1], hdr.dim_z[2], 0);

        if (dim_x == 0 || dim_y == 0 || dim_z == 0)
        {
          printf("ERROR: File corrupt '%s'\n", filename);
          return;
        }

        unsigned int xblocks = (dim_x + block_x - 1) / block_x;
        unsigned int yblocks = (dim_y + block_y - 1) / block_y;
        unsigned int zblocks = (dim_z + block_z - 1) / block_z;

        size_t   data_size = xblocks * yblocks * zblocks * 16;
        // ... this was the astcenc magic.

        while (addr_offset < filesize)
        {
          uint8_t* buffer = new uint8_t[data_size];

          searchfile.read((char*)buffer, data_size);
          if (!searchfile)
          {
            printf("ERROR: File read failed '%s'\n", filename);
            return;
          }

          astc_compressed_image out_image{};
          out_image.data = buffer;
          out_image.data_len = data_size;
          out_image.block_x = block_x;
          out_image.block_y = block_y;
          out_image.block_z = block_z;
          out_image.dim_x = dim_x;
          out_image.dim_y = dim_y;
          out_image.dim_z = dim_z;

          // Save (compressed) image and go to next slice
          comp_images.push_back(out_image);
          addr_offset += data_size;
        }

        // If the file is read close it and prepare to look for a 2nd file
        searchfile.close();
        filenumber++;
      }
      // If no file with the given name exists this was it
      else
      {
        searchForFiles = false;
      }
    }
  
  */


  /* =============================================================

                         ASTC Compressor

     ============================================================= */

  void astc::call_astc_compressor(
    astcenc_image*                 img,
    astc_datatype*                 compressed_img,
    std::size_t                    compressed_img_size,
    std::uint16_t                  context_id
  )
  {
    // Compress
    status = astcenc_compress_image(contexts[context_id], *img, swz_encode, compressed_img, compressed_img_size, 0);

    // This must be performed before next compression
    astcenc_compress_reset(contexts[context_id]);
  }


  jayComp<astc_datatype> astc::compress(
    std::vector<astcenc_image*> source_imgs
  )
  {
    jayComp<astc_datatype> compressed;

    // The number of compressed blocks is based on image-dimensions and block-size
    std::size_t blocks_x = (source_imgs[0]->dim_x + config.block_x - 1) / config.block_x;
    std::size_t blocks_y = (source_imgs[0]->dim_y + config.block_y - 1) / config.block_y;
    std::size_t blocks_z = (source_imgs[0]->dim_z + config.block_z - 1) / config.block_z;
    compressed.dim_x     = source_imgs[0]->dim_x;
    compressed.dim_y     = source_imgs[0]->dim_y;
    compressed.dim_z     = source_imgs[0]->dim_z;
    compressed.block_x   = config.block_x;
    compressed.block_y   = config.block_y;
    compressed.block_z   = config.block_z;
    compressed.img_len   = (blocks_x * blocks_y * blocks_z) << 4;
    compressed.data_len  = compressed.img_len * source_imgs.size();

    // Reserve memory for all compressed images
    compressed.data.resize(compressed.data_len);

    std::uint16_t threads_available = contexts.size();
    std::uint16_t processed_images = 0;

    // Divide the number of jobs by the available threads (& make sure every image gets processed, overshooting here is okay)
    for (std::uint16_t i = 0; i < (source_imgs.size() + threads_available - 1) / threads_available; i++)
    {
      // Start all threads
      for (std::uint16_t thread_nr = 0; thread_nr < threads_available; thread_nr++)
      {
        // Check if the array has been overshot and end the processing
        if (processed_images >= source_imgs.size())
          break;
        
        // Check if the preferred thread_nr is currently working
        if (threads[thread_nr] != nullptr && threads[thread_nr]->joinable())
          threads[thread_nr]->join();

        // Handy references
        auto  index          = i * threads_available + thread_nr;
        auto  img            = source_imgs[index];
        auto* compressed_img = &compressed.data[compressed.img_len * index];

        // Start a thread
        threads[thread_nr] = std::make_shared<std::thread>(std::thread(&astc::call_astc_compressor, this, img, compressed_img, compressed.img_len, thread_nr));
        std::cout << "Started thread: Compressing image " << index << std::endl;
        
        processed_images++;
      }
    }

    for (auto& th : threads)
    {
      // If thread Object is Joinable then Join that thread.
      if (th->joinable())
        th->join();
    }

    for (auto& th : threads)
    {
      // Destroy any evidence to the old threads
      th.reset();
    }

    return compressed;
  }


  std::vector<astcenc_image*> astc::decompress(
    const jayComp<astc_datatype>& comp_imgs
  )
  {
    std::size_t number_of_images = comp_imgs.data_len / comp_imgs.img_len;
    std::vector<astcenc_image*> decompressed_imgs(number_of_images);

    for (std::uint16_t i = 0; i < number_of_images; i++)
    {
      // Will be written to
      auto& img = decompressed_imgs[i];
      // Allocate memory for the decompressed image
      img = alloc_data(comp_imgs.dim_x, comp_imgs.dim_y, comp_imgs.dim_z, 0, 16);
      // Decompress & reset
      status = astcenc_decompress_image(contexts[0], &comp_imgs.data[i * comp_imgs.img_len], comp_imgs.img_len, *img, swz_decode);
      astcenc_compress_reset(contexts[0]);

      if (status != ASTCENC_SUCCESS)
      {
        printf("ERROR: Codec decompress failed: %s\n", astcenc_get_error_string(status));
        return {};
      }
    }

    return decompressed_imgs;
  }
  

  /* =============================================================

                      ASTC Image Operations

   ============================================================= */

  astcenc_image* astc::alloc_data(
    std::size_t dim_width,
    std::size_t dim_height,
    std::size_t dim_depth,
    std::size_t dim_pad,
    unsigned int bitness
  )
  {
    astcenc_image* img          = new astcenc_image;
                   img->dim_x   = dim_width;
                   img->dim_y   = dim_height;
                   img->dim_z   = dim_depth;
                   img->dim_pad = dim_pad;

    std::size_t    dim_ex       = dim_width  + 2 * dim_pad;
    std::size_t    dim_ey       = dim_height + 2 * dim_pad;
    std::size_t    dim_ez       = (dim_depth == 1) ? 1 : dim_depth + 2 * dim_pad;

    assert(bitness == 8 || bitness == 16);

    if (bitness == 8)
    {
      uint8_t*** data8       = new uint8_t ** [    dim_ez                  ];
                 data8[0]    = new uint8_t *  [    dim_ez * dim_ey         ];
                 data8[0][0] = new uint8_t    [4 * dim_ez * dim_ey * dim_ex];
      memset(data8[0][0], 0, 4 * dim_ez * dim_ey * dim_ex);

      for (std::size_t z = 1; z < dim_ez; z++)
      {
        data8[z]    = data8[0]    +     z * dim_ey;
        data8[z][0] = data8[0][0] + 4 * z * dim_ey * dim_ex;
      }

      for (std::size_t z = 0; z < dim_ez; z++)
      {
        for (std::size_t y = 1; y < dim_ey; y++)
        {
          data8[z][y] = data8[z][0] + 4 * y * dim_ex;
        }
      }

      img->data_type = ASTCENC_TYPE_U8;
      img->data      = static_cast<void*>(data8);
    }
    else if (bitness == 16)
    {
      uint16_t*** data16       = new uint16_t ** [    dim_ez                  ];
                  data16[0]    = new uint16_t *  [    dim_ez * dim_ey         ];
                  data16[0][0] = new uint16_t    [4 * dim_ez * dim_ey * dim_ex];
      memset(data16[0][0], 0, 4 * dim_ez * dim_ey * dim_ex);

      for (std::size_t z = 1; z < dim_ez; z++)
      {
        data16[z]    = data16[0]    +     z * dim_ey;
        data16[z][0] = data16[0][0] + 4 * z * dim_ey * dim_ex;
      }

      for (std::size_t z = 0; z < dim_ez; z++)
      {
        for (std::size_t y = 1; y < dim_ey; y++)
        {
          data16[z][y] = data16[z][0] + 4 * y * dim_ex;
        }
      }

      img->data_type = ASTCENC_TYPE_F16;
      img->data      = static_cast<void*>(data16);
    }

    return img;
  }


  void astc::fill_image_padding_area(
    astcenc_image* img
  )
  {
    if (img->dim_pad == 0)
    {
      return;
    }

    unsigned int dim_ex = img->dim_x + 2 * img->dim_pad;
    unsigned int dim_ey = img->dim_y + 2 * img->dim_pad;
    unsigned int dim_ez = (img->dim_z == 1) ? 1 : (img->dim_z + 2 * img->dim_pad);

    unsigned int xmin = img->dim_pad;
    unsigned int ymin = img->dim_pad;
    unsigned int zmin = (img->dim_z == 1) ? 0 : img->dim_pad;
    unsigned int xmax = img->dim_x + img->dim_pad - 1;
    unsigned int ymax = img->dim_y + img->dim_pad - 1;
    unsigned int zmax = (img->dim_z == 1) ? 0 : img->dim_z + img->dim_pad - 1;

    // This is a very simple implementation. Possible optimizations include:
    // * Testing if texel is outside the edge.
    // * Looping over texels that we know are outside the edge.
    if (img->data_type == ASTCENC_TYPE_U8)
    {
      uint8_t*** data8 = static_cast<uint8_t***>(img->data);
      for (unsigned int z = 0; z < dim_ez; z++)
      {
        int zc = MIN(MAX(z, zmin), zmax);
        for (unsigned int y = 0; y < dim_ey; y++)
        {
          int yc = MIN(MAX(y, ymin), ymax);
          for (unsigned int x = 0; x < dim_ex; x++)
          {
            int xc = MIN(MAX(x, xmin), xmax);
            for (unsigned int i = 0; i < 4; i++)
            {
              data8[z][y][4 * x + i] = data8[zc][yc][4 * xc + i];
            }
          }
        }
      }
    }
    else // if (img->data_type == ASTCENC_TYPE_F16)
    {
      assert(img->data_type == ASTCENC_TYPE_F16);
      uint16_t*** data16 = static_cast<uint16_t***>(img->data);
      for (unsigned int z = 0; z < dim_ez; z++)
      {
        int zc = MIN(MAX(z, zmin), zmax);
        for (unsigned int y = 0; y < dim_ey; y++)
        {
          int yc = MIN(MAX(y, ymin), ymax);
          for (unsigned int x = 0; x < dim_ex; x++)
          {
            int xc = MIN(MAX(x, xmin), xmax);
            for (unsigned int i = 0; i < 4; i++)
            {
              data16[z][y][4 * x + i] = data16[zc][yc][4 * xc + i];
            }
          }
        }
      }
    }
  }


  void astc::free_image(astcenc_image * img)
  {
    if (img == nullptr)
    {
      return;
    }

    if (img->data_type == ASTCENC_TYPE_U8)
    {
      uint8_t*** data8 = static_cast<uint8_t***>(img->data);
      delete[] data8[0][0];
      delete[] data8[0];
      delete[] data8;
    }
    else // if (img->data_type == ASTCENC_TYPE_F16)
    {
      assert(img->data_type == ASTCENC_TYPE_F16);
      uint16_t*** data16 = static_cast<uint16_t***>(img->data);
      delete[] data16[0][0];
      delete[] data16[0];
      delete[] data16;
    }

    delete img;
  }
}