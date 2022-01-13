#include <jay/core/menu.hpp>
#include <jay/io/data_io.hpp>
#include <jay/core/camera.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

class application;

namespace jay
{
  // =====================
  // ANTTWEAKBAR CALLBACKS
  // =====================

  // String Handler
  // ==============
  void TW_CALL CopyStdStringToClient(std::string& destinationClientString, const std::string& sourceLibraryString)
  {
    destinationClientString = sourceLibraryString;
  }

  // Button Callbacks
  // ================
  void TW_CALL buttonDirtyCB(void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData); // menu pointer is stored in clientData
    //menu->calcSeedsTotal();
    //menu->calcIterations();
    menu->generated_seed_vertices = menu->int_global_step_count;
    menu->markDirty();                                 // Re-initialize generation
  }

  void TW_CALL button_export_image_CB(void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData);
    menu->export_img = true;
  }

  void TW_CALL button_export_positions_CB(void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData);
    menu->export_pos = true;
  }

  void TW_CALL button_export_velos_CB(void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData);
    menu->export_velo = true;
  }

  void TW_CALL button_export_timers_CB(void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData);
    menu->export_timers();
  }

  void TW_CALL button_calc_cell_size_CB(void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData);
    menu->calc_cell_size();
  }

  void TW_CALL buttonExportMeshCB(void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData);
    menu->export_pos = true;
  }

  void TW_CALL buttonExportVelosCB(void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData);
    menu->export_velo = true;
  }

  void TW_CALL button_set_camera_CB(void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData);
    menu->change_camera = true;
  }

  // CB Var Callbacks
  // ================
  void TW_CALL getStrideStrCB(void* value, void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData);
    menu->calc_seed_stride();
    menu->calc_seed_stride_string();

    std::string* destPtr = static_cast<std::string*>(value);
    TwCopyStdStringToLibrary(*destPtr, menu->seed_stride_string);
  }

  void TW_CALL getSPDStrCB(void* value, void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData);

    menu->calc_seed_directional();
    menu->calc_seed_directional_string();
    std::string* destPtr = static_cast<std::string*>(value);
    TwCopyStdStringToLibrary(*destPtr, menu->seed_directional_string);
  }

  void TW_CALL getSeedsCB(void* value, void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData);
    
    menu->calc_seed_count();
    menu->calcIterations();
    menu->calcExportSize();

    *static_cast<std::uint32_t*>(value) = menu->seed_count;
  }

  void TW_CALL setSeedingStrategyCB(const void* value, void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData);
    menu->seed_strategy_strided = *static_cast<const bool*>(value);
    menu->display_seed_params();
  }

  void TW_CALL getSeedingStrategyCB(void* value, void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData);

    *static_cast<bool*>(value) = menu->seed_strategy_strided;
  }

  void TW_CALL setIntegrationTechniqueCB(const void* value, void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData);
    menu->int_strategy_rk4 = *static_cast<const bool*>(value);
  }

  void TW_CALL getIntegrationTechniqueCB(void* value, void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData);

    *static_cast<bool*>(value) = menu->int_strategy_rk4;
  }

  void TW_CALL setIntegrationTypeCB(const void* value, void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData);
    menu->int_unsteady = *static_cast<const bool*>(value);
    menu->displayIntegrationParams();
  }

  void TW_CALL getIntegrationTypeCB(void* value, void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData);

    *static_cast<bool*>(value) = menu->int_unsteady;
  }

  void TW_CALL setStepsCB(const void* value, void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData);
    menu->int_global_step_count = *static_cast<const std::uint32_t*>(value);
    menu->calc_executed_steps();
    menu->calcIterations();
    menu->calcExportSize();
  }

  void TW_CALL getStepsCB(void* value, void* clientData)
  {
    antMenu* menu = static_cast<antMenu*>(clientData);

    *static_cast<std::uint32_t*>(value) = menu->int_global_step_count;
  }

  // ================================
  // ANTTWEAKBAR INTEGRATION: AntMenu
  // ================================

  // Constructors / Destructors
  // ==========================
  antMenu::antMenu()
  {
    std::string mainBarName = "Jay";
    auto mainBar = TwNewBar(mainBarName.data());
    antBars.push_back(mainBar);

    // Add all the available functionality (and initialize its values).
    // To show it call "useX()" (e.g. useSrcInfo()).
    addSrcInfo();
    addCompInfo();
    addSeedingParams();
    addIntegrationParams();
    addShadingParams();
    addDrawingParams();
    addExport();
    addOGLInfo();
    addTimers();
    addCameraParams();

    dirty = true;

    TwAddButton(mainBar, "Apply Changes", buttonDirtyCB, this, "");

    TwDefine((mainBarName + " size='300 430'").data()); // resize bar
    TwDefine((mainBarName + " valueswidth=fit").data()); // column width fits content
    TwDefine((mainBarName + " buttonalign=left ").data()); // buttons are left-aligned
    TwDefine((" " + mainBarName + " help='This window displays data information and defines integration behaviour.' ").data());

    timers.reserve(10000);
    timer_descs.reserve(10000);

    TwCopyStdStringToClientFunc(CopyStdStringToClient); // must be called once (just after TwInit for instance)
  }

  // Setter
  // ======
    void antMenu::set_src_grid(std::vector<std::size_t>& grid, int grid_dim)
  {
    src_grid.x = (grid_dim >= 1) ? grid[0] : 0;
    src_grid.y = (grid_dim >= 2) ? grid[1] : 0;
    src_grid.z = (grid_dim >= 3) ? grid[2] : 0;
    src_grid.w = (grid_dim >= 4) ? grid[3] : 0;

    src_grid_dim = grid_dim;
  }

  void antMenu::set_src_element_dim(std::uint32_t vec_len)
  {
    src_element_dim = vec_len;
  }
  void antMenu::set_src_datasize(std::size_t datasize_bytes)
  {
    src_datasize = (double)datasize_bytes / (1000 * 1000); // MB
  }

  void antMenu::set_src_fieldtype(bool steady)
  {
    int_unsteady = !steady;

    if (steady)
      field_type_string = "Steady";
    else
      field_type_string = "Unsteady";
  }

  void antMenu::set_cmp_dims(std::size_t dim_x, std::size_t dim_y, std::size_t dim_z)
  {
    cmp_img_dimensions = { dim_x, dim_y, dim_z };
    cmp_img_dims_string = std::to_string(dim_x) + " x " + std::to_string(dim_y) + " x " + std::to_string(dim_z);
  }

  void antMenu::set_cmp_datasize(std::size_t datasize_bytes)
  {
    cmp_data_size    = datasize_bytes; // MB
    cmp_data_size_mb = (double)datasize_bytes / (1000 * 1000); // MB
  }

  void antMenu::set_cmp_imgsize(std::size_t imgsize_bytes)
  {
    cmp_img_size    = imgsize_bytes;
    cmp_img_size_mb = (double)imgsize_bytes / (1000 * 1000); // MB
  }

  void antMenu::set_cmp_blocksizes(std::size_t block_x, std::size_t block_y, std::size_t block_z)
  {
    cmp_block_sizes.x = block_x;
    cmp_block_sizes.y = block_y;
    cmp_block_sizes.z = block_z;
  }

  void antMenu::set_seed_strategy(bool strided)
  {
    seed_strategy_strided = strided;
  }

  void antMenu::set_seed_stride(glm::uvec3 stride)
  {
    seed_stride = stride;
  }

  void antMenu::set_seed_ranges(glm::uvec2 rangeX, glm::uvec2 rangeY, glm::uvec2 rangeZ)
  {
    seed_range_x = rangeX;
    seed_range_y = rangeY;
    seed_range_z = rangeZ;
  }

  void antMenu::set_seed_directional(glm::vec3 spd)
  {
    seed_directional = spd;
  }



  // Getters
  // =======
  /*
  glm::vec4 antMenu::getStepSize()
  {
    if (int_step_scalar && !int_unsteady)
      return glm::vec4(int_step_size.x, int_step_size.x, int_step_size.x, int_step_size.x);
    else if (int_step_scalar && int_unsteady)
      return glm::vec4(int_step_size.x, int_step_size.x, int_step_size.x, int_step_size.w);
    else
      return int_step_size;
  }
  */

  float antMenu::getDrawPercent()
  {
    return (draw_vertices_per_seed / 100.0);
  }

  std::int32_t antMenu::getDrawCount()
  {
    return (draw_vertices_per_seed / 100.0) * generated_seed_vertices;
  }

  // Calculators
  // ===========
  void antMenu::calc_src_grid_string()
  {
    src_grid_string.clear();
    src_grid_string += (src_grid.x > 0) ? std::to_string(src_grid.x) : "";
    src_grid_string += (src_grid.y > 0) ? " x " + std::to_string(src_grid.y) : "";
    src_grid_string += (src_grid.z > 0) ? " x " + std::to_string(src_grid.z) : "";
    src_grid_string += (src_grid.w > 0) ? " x " + std::to_string(src_grid.w) : "";
  }

  void antMenu::calc_seed_directional()
  {
    seed_directional.x = floor(((float)seed_range_x[1] - (float)seed_range_x[0]) / seed_stride.x);
    seed_directional.y = floor(((float)seed_range_y[1] - (float)seed_range_y[0]) / seed_stride.y);
    seed_directional.z = floor(((float)seed_range_z[1] - (float)seed_range_z[0]) / seed_stride.z);
  }

  void antMenu::calc_cmp_img_count()
  {
    cmp_img_count = (std::uint32_t)cmp_data_size / cmp_img_size;
  }

  void antMenu::calc_cmp_blocks_string()
  {
    cmp_blocks_string.clear();
    cmp_blocks_string += std::to_string(cmp_block_sizes.x);
    cmp_blocks_string += " x " + std::to_string(cmp_block_sizes.y);
    cmp_blocks_string += " x " + std::to_string(cmp_block_sizes.z);
  }

  void antMenu::calc_seed_stride()
  {
    seed_stride.x = ((float)seed_range_x[1] - (float)seed_range_x[0]) / (float)seed_directional.x;
    seed_stride.y = ((float)seed_range_y[1] - (float)seed_range_y[0]) / (float)seed_directional.y;
    seed_stride.z = ((float)seed_range_z[1] - (float)seed_range_z[0]) / (float)seed_directional.z;
  }

  void antMenu::calc_seed_count()
  {
    seed_count = seed_directional.x * seed_directional.y * seed_directional.z;
  }

  void antMenu::calc_seed_stride_string()
  {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << seed_stride.x;
    stream << " x " << seed_stride.y;
    stream << " x " << seed_stride.z;
    seed_stride_string = stream.str();
  }

  void antMenu::calc_seed_directional_string()
  {
    seed_directional_string.clear();
    seed_directional_string += std::to_string(seed_directional.x);
    seed_directional_string += " x " + std::to_string(seed_directional.y);
    seed_directional_string += " x " + std::to_string(seed_directional.z);
  }

  void antMenu::calc_cell_size()
  {
    int_cell_size.x = (src_grid.x > 0) ? int_simulation_range.x / (float)(src_grid.x - 1) : 0;
    int_cell_size.y = (src_grid.y > 0) ? int_simulation_range.y / (float)(src_grid.y - 1) : 0;
    int_cell_size.z = (src_grid.z > 0) ? int_simulation_range.z / (float)(src_grid.z - 1) : 0;
    int_cell_size.w = 0;

    // Unsteady 4D dataset
    if (int_unsteady && (src_grid_dim >= 4))
      int_cell_size.w = int_simulation_range.w / (float)src_grid.w;
    // Unsteady 3D dataset
    else if (int_unsteady)
    {
      int_cell_size.z = 0;
      int_cell_size.w = int_simulation_range.w / (float)src_grid.z;
    }
  }

  // Calculate timestep-size and how many steps are executed per GPU-run.
  // If integration time higher than dataset time, then scale down timestep-size dt in relation to integration-stepsize h.
  void antMenu::calc_executed_steps()
  {
    std::uint32_t global_steps = int_global_step_count;
    std::uint32_t local_steps = int_local_step_count;
    std::uint32_t remaining_steps = int_remainder_step_count;
    std::uint32_t time_dim = src_grid.w - 1; // -1 Represents the fact, that timesteps are always handled as pairs
    float step_size_dt = int_step_size_dt;
    float step_size_h = int_step_size_h;
    float timesize = (int_cell_size.w > 0) ? int_cell_size.w : 1.0;

    if (int_unsteady)
    {
      float t = time_dim * timesize;
      float tau = global_steps * step_size_h;

      // dt = h and if tau > t then clamp to last timeslice
      if (false)
      {
        local_steps = std::floor(timesize / step_size_h + 0.01);
        step_size_dt = timesize / local_steps;

        if (tau <= t)
          remaining_steps = global_steps % local_steps;
        else
          remaining_steps = global_steps - time_dim * local_steps;
      }
      // Scale dt such that tau = c * t
      else if (false)
      {
        step_size_dt = (t / tau) * step_size_h;
        local_steps = std::floor(timesize / step_size_dt + 0.01);
        remaining_steps = global_steps - local_steps * time_dim;
      }
      else
      {
        step_size_dt = step_size_dt;
        local_steps = std::floor(timesize / step_size_dt + 0.001);
        int full_passes = (local_steps > 0) ? std::min(global_steps / local_steps, time_dim) : 0;
        remaining_steps = global_steps - full_passes * local_steps;
      }
    }
    else
    {
      step_size_dt = int_step_size_h;
      local_steps = int_global_step_count;
      remaining_steps = 0;
    }

    int_vram = (local_steps > 0) ? std::min(global_steps / local_steps, time_dim) * local_steps : 0;
    int_vram += remaining_steps;
    int_vram *= seed_count;
    int_vram *= 4.0 * 4.0 / 1024.0 / 1024.0 / 1024.0;
    int_step_size_dt = step_size_dt;
    int_local_step_count = local_steps;
    int_remainder_step_count = remaining_steps;
  }

  void antMenu::calcIterations()
  {
    int_iteration_count = seed_count * int_global_step_count;
  }

  void antMenu::calcExportSize()
  {
    export_pos_size = ((double)int_iteration_count * 4.0 * sizeof(float)) / (1000.0 * 1000.0);
    export_velo_size = ((double)int_iteration_count * 4.0 * sizeof(float)) / (1000.0 * 1000.0);
  }

  void antMenu::export_timers()
  {
    auto time = std::chrono::system_clock::now();
    auto t_time = std::chrono::system_clock::to_time_t(time);

    std::string filepath = "../files/exports/" + export_timers_name_string + ".txt";
    std::string description = "Timer Export of Jay\n===================\n";
    description += "Date: " + std::string(std::ctime(&t_time)) + "\n";
    description += "Grid: " + src_grid_string + "\n";
    description += "Data Size: ";
    if (cmp_img_count > 0)
    {
      description += std::to_string(cmp_data_size_mb);
      description += " MB\n";
      description += "Image Size: ";
      description += std::to_string(cmp_img_size_mb);
      description += " MB\n";
    }
    else
    {
      description += std::to_string(src_datasize);
      description += " MB\n";
    }
    
    if (!ds_name.empty())
      description += "Dataset: " + ds_name + "\n";

    description += "Time Unit: ms\n";

    auto status = data_io::store_descriptive_text(description, timer_descs, timers, filepath, 16, false);

    if (status == -1)
      export_timers_status = false;
    else
      export_timers_status = true;
  }

  bool antMenu::export_velocities()
  {
    auto result = export_velo;
    export_velo = false;
    return result;
  }

  bool antMenu::export_positions()
  {
    auto result = export_pos;
    export_pos = false;
    return result;
  }

  bool antMenu::export_image()
  {
    auto result = export_img;
    export_img = false;
    return result;
  }

  void antMenu::calcOGLInfoStrs()
  {
    ogl_max_size_3dtex_str            = std::to_string(ogl_max_size_3dtex);
    ogl_max_layers_2darraytex_str     = std::to_string(ogl_max_layers_2darraytex);
    ogl_max_size_2dtex_str            = std::to_string(ogl_max_size_2dtex);
    ogl_max_size_texturebuffers_str   = std::to_string(ogl_max_size_texturebuffers);
    ogl_max_count_textures_str        = std::to_string(ogl_max_count_textures);
    ogl_count_compression_formats_str = std::to_string(ogl_count_compression_formats);
    ogl_cs_max_ssbos_str              = std::to_string(ogl_cs_max_ssbos);
    ogl_cs_max_ubos_str               = std::to_string(ogl_cs_max_ubos);
    ogl_cs_max_textures_str           = std::to_string(ogl_cs_max_textures);
    ogl_cs_max_workgroups_str         = std::to_string(ogl_cs_max_workgroups_x) + " x " + std::to_string(ogl_cs_max_workgroups_y) + " x " + std::to_string(ogl_cs_max_workgroups_z);
  }


  // Adders
  // ======
  void antMenu::addSrcInfo()
  {
    // Mainbar
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    // Variable Initialization
    src_grid      = { 0, 0, 0, 0 };
    src_grid_dim   = 0;
    src_grid_string = "";
    src_element_dim = 0;
    src_datasize = 0;
    field_type_string = "";

    // AntTweakBar Variables
    TwAddVarRO(mainBar, "Grid",                  TW_TYPE_STDSTRING, &src_grid_string,   "group='Source Info'");
    TwAddVarRO(mainBar, "Element dimension",     TW_TYPE_UINT32,    &src_element_dim,    "group='Source Info'");
    TwAddVarRO(mainBar, "Source Data Size (MB)", TW_TYPE_DOUBLE,    &src_datasize,  "group='Source Info' label='Data Size (MB)'");
    TwAddVarRO(mainBar, "Field Type",            TW_TYPE_STDSTRING, &field_type_string, "group='Source Info'");

    TwDefine((mainBarName + "/'Source Info' opened=true visible='false'").data());
  }

  void antMenu::addCompInfo()
  {
    // Mainbar
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    // Variable Initialization
    cmp_img_dimensions = { 0, 0, 0 };
    cmp_img_dims_string = "";
    cmp_data_size = 0;
    cmp_img_size = 0;
    cmp_data_size_mb = 0;
    cmp_img_size_mb = 0;
    cmp_img_count = 0;
    cmp_block_sizes = { 0, 0, 0 };
    cmp_blocks_string = "";

    // AntTweakBar Variables
    TwAddVarRO(mainBar, "Image Dimensions",    TW_TYPE_STDSTRING, &cmp_img_dims_string,      "group='Compressed Info'");
    TwAddVarRO(mainBar, "Block Dimensions",    TW_TYPE_STDSTRING, &cmp_blocks_string,       "group='Compressed Info'");
    TwAddVarRO(mainBar, "Image Size (MB)",     TW_TYPE_DOUBLE,    &cmp_img_size_mb,         "group='Compressed Info'");
    TwAddVarRO(mainBar, "Comp Data Size (MB)", TW_TYPE_DOUBLE,    &cmp_data_size_mb,        "group='Compressed Info' label='Data Size (MB)'");

    TwDefine((mainBarName + "/'Compressed Info' opened=true visible='false'").data());
  }

  void antMenu::addSeedingParams()
  {
    // Mainbar
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    // Variable Initialization
    seed_strategy_strided = 0;
    seed_stride = { 0.0, 0.0, 0.0 };
    seed_range_x = { 0, 0 };
    seed_range_y = { 0, 0 };
    seed_range_z = { 0, 0 };
    seed_directional = { 0, 0, 0 };
    seed_count = 0;
    seed_directional_string = "";
    seed_stride_string = "";

    // AntTweakBar Variables
    TwAddVarCB(mainBar, "Seeding Strategy", TW_TYPE_BOOLCPP, setSeedingStrategyCB, getSeedingStrategyCB, this, "group='Seeding Parameters' true='Strided' false='Explicit'");

    TwAddVarRW(mainBar, "Range in X (from)", TW_TYPE_UINT32, &(seed_range_x[0]), "group='Set Grid Range'");
    TwAddVarRW(mainBar, "Range in X (to)",   TW_TYPE_UINT32, &(seed_range_x[1]), "group='Set Grid Range'");
    TwAddVarRW(mainBar, "Range in Y (from)", TW_TYPE_UINT32, &(seed_range_y[0]), "group='Set Grid Range'");
    TwAddVarRW(mainBar, "Range in Y (to)",   TW_TYPE_UINT32, &(seed_range_y[1]), "group='Set Grid Range'");
    TwAddVarRW(mainBar, "Range in Z (from)", TW_TYPE_UINT32, &(seed_range_z[0]), "group='Set Grid Range'");
    TwAddVarRW(mainBar, "Range in Z (to)",   TW_TYPE_UINT32, &(seed_range_z[1]), "group='Set Grid Range'");
    TwDefine((mainBarName + "/'Set Grid Range' group='Seeding Parameters' opened=false").data());

    TwAddVarRW(mainBar, "Set Stride in X", TW_TYPE_FLOAT, &seed_stride[0], "label='Stride in X' group='Seeding Parameters' min=1.0 step=0.1");
    TwAddVarRW(mainBar, "Set Stride in Y", TW_TYPE_FLOAT, &seed_stride[1], "label='Stride in Y' group='Seeding Parameters' min=1.0 step=0.1");
    TwAddVarRW(mainBar, "Set Stride in Z", TW_TYPE_FLOAT, &seed_stride[2], "label='Stride in Z' group='Seeding Parameters' min=1.0 step=0.1");

    TwAddVarRW(mainBar, "Set Seeds in X", TW_TYPE_UINT32, &seed_directional[0], "label='Seeds in X' group='Seeding Parameters' min=0");
    TwAddVarRW(mainBar, "Set Seeds in Y", TW_TYPE_UINT32, &seed_directional[1], "label='Seeds in Y' group='Seeding Parameters' min=0");
    TwAddVarRW(mainBar, "Set Seeds in Z", TW_TYPE_UINT32, &seed_directional[2], "label='Seeds in Z' group='Seeding Parameters' min=0");

    TwAddVarCB(mainBar, "Stride",         TW_TYPE_STDSTRING, NULL, getStrideStrCB, this, "group='Seeding Parameters'");
    TwAddVarCB(mainBar, "Seeds",          TW_TYPE_STDSTRING, NULL, getSPDStrCB,    this, "group='Seeding Parameters'");
    TwAddVarCB(mainBar, "Seeds in Total", TW_TYPE_UINT32,    NULL, getSeedsCB,     this, "group='Seeding Parameters'");

    display_seed_params();

    TwDefine((mainBarName + "/'Seeding Parameters' opened=false visible='false'").data());
  }

  void antMenu::addIntegrationParams()
  {
    // Mainbar
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    // Variable Initialization
    int_strategy_rk4 = false;
    int_unsteady = !src_steady;
    int_simulation_range = { 0.0, 0.0, 0.0, 0.0 };
    int_global_step_count = 500;
    int_local_step_count = 500;
    int_remainder_step_count = 0;
    int_cell_size = { 1.0, 1.0, 1.0, 1.0 };
    int_step_size_h = 0.2;
    int_step_size_dt = int_step_size_h;
    int_iteration_count = 0;
    int_texelfetch = 0;
    int_dataset_factor = 1.0;
    generated_seed_vertices = 0;
    int_vram = 0;

    calc_executed_steps();
    calcIterations();
    calcExportSize();


    // AntTweakBar Variables
    TwAddVarRW(mainBar, "Texture Access",        TW_TYPE_BOOLCPP, &int_texelfetch,                                     "group='Integration Parameters' true='texelFetch' false='texture'");
    TwAddVarCB(mainBar, "Integration Algorithm", TW_TYPE_BOOLCPP, setIntegrationTechniqueCB, getIntegrationTechniqueCB, this, "group='Integration Parameters' true='Runge Kutta 4th' false='Improved Euler'");
    TwAddVarCB(mainBar, "Integration Mode",      TW_TYPE_BOOLCPP, setIntegrationTypeCB, getIntegrationTypeCB,           this, "group='Integration Parameters' true='Unsteady' false='Steady'");

    //TwAddVarRW(mainBar, "Steps",            TW_TYPE_UINT32, &integrationStepCount,                                 "group='Integration Parameters'");
    TwAddVarCB(mainBar, "Global Steps",      TW_TYPE_UINT32,  setStepsCB,             getStepsCB,             this, "group='Integration Parameters'");
    TwAddVarRO(mainBar, "Local Steps (GPU)", TW_TYPE_UINT32, &int_local_step_count,                                 "group='Integration Parameters'");
    TwAddVarRO(mainBar, "Remainder Steps (Last)", TW_TYPE_UINT32, &int_remainder_step_count,                        "group='Integration Parameters'");
    TwAddVarRO(mainBar, "VRAM needed (GB)", TW_TYPE_UINT32, &int_vram, "group='Integration Parameters'");
    TwAddVarRW(mainBar, "Cell Size in X",    TW_TYPE_FLOAT,  &int_cell_size.x,                                      "group='Integration Parameters' min=0.0");
    TwAddVarRW(mainBar, "Cell Size in Y",    TW_TYPE_FLOAT,  &int_cell_size.y,                                      "group='Integration Parameters' min=0.0");
    TwAddVarRW(mainBar, "Cell Size in Z",    TW_TYPE_FLOAT,  &int_cell_size.z,                                      "group='Integration Parameters' min=0.0");
    TwAddVarRW(mainBar, "Cell Size in T",    TW_TYPE_FLOAT,  &int_cell_size.w,                                      "group='Integration Parameters' min=0.0");
    TwAddVarRW(mainBar, "Step Size h",       TW_TYPE_FLOAT,  &int_step_size_h,                                      "group='Integration Parameters' min=0.0 step=0.0001");
    TwAddVarRW(mainBar, "Step Size dt",      TW_TYPE_FLOAT,  &int_step_size_dt,                                     "group='Integration Parameters'");
    TwAddVarRW(mainBar, "Dataset Factor",    TW_TYPE_FLOAT,  &int_dataset_factor,                                   "group='Integration Parameters'");

    TwAddVarRW(mainBar, "Simulation Range in X", TW_TYPE_FLOAT, &int_simulation_range.x, "group='Integration Parameters' min=0.0 step=0.01");
    TwAddVarRW(mainBar, "Simulation Range in Y", TW_TYPE_FLOAT, &int_simulation_range.y, "group='Integration Parameters' min=0.0 step=0.01");
    TwAddVarRW(mainBar, "Simulation Range in Z", TW_TYPE_FLOAT, &int_simulation_range.z, "group='Integration Parameters' min=0.0 step=0.01");
    TwAddVarRW(mainBar, "Simulation Range in T", TW_TYPE_FLOAT, &int_simulation_range.w, "group='Integration Parameters' min=0.0 step=0.01");

    TwAddButton(mainBar, "Calculate Cell Sizes", button_calc_cell_size_CB, this, "group='Integration Parameters'");

    TwAddVarRO(mainBar, "Iterations in total", TW_TYPE_UINT32, &int_iteration_count,    "group='Integration Parameters'");

    displayIntegrationParams();

    TwDefine((mainBarName + "/'Integration Parameters' opened=false visible='false'").data());
  }

  void antMenu::addShadingParams()
  {
    // Mainbar
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    // Variable Initialization
    shading_max_value = 1.0;
    shading_min_value = 0.0;
    shading_highlight_threshold = 1.0;
    shading_highlight_color = { 1.0, 0.0, 0.0 };
    shading_damping_factor = 1.0;

    // AntTweakBar Variables
    TwAddVarRW(mainBar, "Max velocity Threshold", TW_TYPE_FLOAT, &shading_max_value, "group='Shading Parameters' step=0.01");
    TwAddVarRW(mainBar, "Min velocity Threshold", TW_TYPE_FLOAT, &shading_min_value, "group='Shading Parameters' step=0.01");
    //TwAddVarRW(mainBar, "Highlight Color",     TW_TYPE_COLOR3F, &shading_highlight_color,     "group='Shading Parameters'");
    //TwAddVarRW(mainBar, "Highlight Threshold", TW_TYPE_FLOAT,   &shading_highlight_threshold, "group='Shading Parameters' step=0.1");
    TwAddVarRW(mainBar, "Damping",             TW_TYPE_FLOAT,   &shading_damping_factor,      "group='Shading Parameters' min=0.0 max=1.0 step=0.1");

    TwDefine((mainBarName + "/'Shading Parameters' label='Shading Parameters (Live)' opened=false visible='false'").data());
  }

  void antMenu::addDrawingParams()
  {
    // Mainbar
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    // Variable Initialization
    draw_vertices_per_seed = 100.0;
    draw_seeds = seed_count;
    draw_offset = 0;

    // AntTweakBar Variables
    TwAddVarRW(mainBar, "Draw % vertices per Seed", TW_TYPE_FLOAT, &draw_vertices_per_seed, "group='Drawing Parameters' min=0.0 max=100.0 step=0.1");
    TwAddVarRW(mainBar, "Draw seeds", TW_TYPE_UINT32, &draw_seeds, "group='Drawing Parameters' min=0");
    TwAddVarRW(mainBar, "Draw offset", TW_TYPE_UINT32, &draw_offset, "group='Drawing Parameters' min=0");

    TwDefine((mainBarName + "/'Drawing Parameters' label='Drawing Parameters (Live)' opened=false visible='false'").data());
  }

  void antMenu::addOGLInfo()
  {
    // Mainbar
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    // Variable Initialization
    ogl_max_size_3dtex            = 0;
    ogl_max_layers_2darraytex     = 0;
    ogl_max_size_2dtex            = 0;
    ogl_max_size_texturebuffers   = 0;
    ogl_max_count_textures        = 0;
    ogl_count_compression_formats = 0;
    ogl_cs_max_ssbos              = 0;
    ogl_cs_max_ubos               = 0;
    ogl_cs_max_textures           = 0;
    ogl_cs_max_workgroups_x       = 0;
    ogl_cs_max_workgroups_y       = 0;
    ogl_cs_max_workgroups_z       = 0;
    ogl_support_4d_tex            = 0;     
    ogl_support_astc_ldr          = 0;
    ogl_support_astc_sliced       = 0;
    ogl_support_astc_hdr          = 0;

    calcOGLInfoStrs();

    // AntTweakBar Variables
    TwAddVarRO(mainBar, "CS: Max SSBOs",                     TW_TYPE_STDSTRING, &ogl_cs_max_ssbos_str,              "group='OpenGL Info'");
    TwAddVarRO(mainBar, "CS: Max UBOs",                      TW_TYPE_STDSTRING, &ogl_cs_max_ubos_str,               "group='OpenGL Info'");
    TwAddVarRO(mainBar, "CS: Max Texture Image Units",       TW_TYPE_STDSTRING, &ogl_cs_max_textures_str,           "group='OpenGL Info'");
    TwAddVarRO(mainBar, "CS: Max Work Group Count",          TW_TYPE_STDSTRING, &ogl_cs_max_workgroups_str,         "group='OpenGL Info'");
    TwAddVarRO(mainBar, "Max 3D Texture Size",               TW_TYPE_STDSTRING, &ogl_max_size_3dtex_str,            "group='OpenGL Info'");
    TwAddVarRO(mainBar, "Max 2D Texture Size",               TW_TYPE_STDSTRING, &ogl_max_size_2dtex_str,            "group='OpenGL Info'");
    TwAddVarRO(mainBar, "Max Array Texture Layers",          TW_TYPE_STDSTRING, &ogl_max_layers_2darraytex_str,     "group='OpenGL Info'");
    TwAddVarRO(mainBar, "Max Texture Buffer Size",           TW_TYPE_STDSTRING, &ogl_max_size_texturebuffers_str,   "group='OpenGL Info'");
    TwAddVarRO(mainBar, "Max Texture Image Units",           TW_TYPE_STDSTRING, &ogl_max_count_textures_str,        "group='OpenGL Info'");
    TwAddVarRO(mainBar, "Count of Compression Formats",      TW_TYPE_STDSTRING, &ogl_count_compression_formats_str, "group='OpenGL Info'");
    TwAddVarRO(mainBar, "Extension: ASTC LDR Support",       TW_TYPE_BOOLCPP,   &ogl_support_astc_ldr,                "group='OpenGL Info'");
    TwAddVarRO(mainBar, "Extension: ASTC Sliced 3D Support", TW_TYPE_BOOLCPP,   &ogl_support_astc_sliced,                "group='OpenGL Info'");
    TwAddVarRO(mainBar, "Extension: ASTC HDR Support",       TW_TYPE_BOOLCPP,   &ogl_support_astc_hdr,                "group='OpenGL Info'");
    TwAddVarRO(mainBar, "Extension: 4D Texture Support",     TW_TYPE_BOOLCPP,   &ogl_support_4d_tex,                "group='OpenGL Info'");



    TwDefine((mainBarName + "/'OpenGL Info' opened=false visible='false'").data());

    // OGL Parameters
    // GL_MAX_COMPUTE_UNIFORM_BLOCKS
    // GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS
    // GL_MAX_COMPUTE_WORK_GROUP_COUNT
    // GL_LINE_SMOOTH
    // GL_LINE_WIDTH 
    // GL_MAX_3D_TEXTURE_SIZE
    // GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
    // GL_MAX_RECTANGLE_TEXTURE_SIZE
    // GL_MAX_TEXTURE_BUFFER_SIZE
    // GL_MAX_TEXTURE_IMAGE_UNITS
    // GL_MAX_TEXTURE_SIZE
    // GL_NUM_COMPRESSED_TEXTURE_FORMATS
    // GL_NUM_EXTENSIONS
    // GL_PACK_ALIGNMENT
    // GL_POINT_SIZE
    // GL_POINT_SIZE_RANGE
    // GL_SMOOTH_LINE_WIDTH_RANGE
    // GL_TEXTURE_COMPRESSION_HINT
    // GL_TIMESTAMP
  }

  void antMenu::addExport()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    export_pos_name_string = "";
    export_velo_name_string = "";
    export_pos_size = 0.0;
    export_velo_size = 0.0;
    export_pos = false;
    export_velo = false;
    export_img = false;
    export_timers_status = true;
    export_velo_status = true;
    export_pos_status = true;
    export_img_status = true;

    TwAddVarRW(mainBar, "Positions Size (MB)",         TW_TYPE_DOUBLE,    &export_pos_size,    "group='Export'");
    TwAddVarRW(mainBar, "Export Positions Name",       TW_TYPE_STDSTRING, &export_pos_name_string, "group='Export'");
    TwAddButton(mainBar, "Export Positions",       buttonExportMeshCB,  this, "group='Export'");
    TwAddVarRO(mainBar, "Pos Export Status", TW_TYPE_BOOLCPP, &export_pos_status, "group='Export' true='OK' false='ERROR' label='Export Status'");
    TwAddVarRW(mainBar, "Velocities Size (MB)",   TW_TYPE_DOUBLE,    &export_velo_size,   "group='Export'");
    TwAddVarRW(mainBar, "Export Velocities Name", TW_TYPE_STDSTRING, &export_velo_name_string, "group='Export'");
    TwAddButton(mainBar, "Export Velocities", buttonExportVelosCB, this, "group='Export'");
    TwAddVarRO(mainBar, "Velo Export Status", TW_TYPE_BOOLCPP, &export_velo_status, "group='Export' true='OK' false='ERROR' label='Export Status'");
    TwAddButton(mainBar, "Export Image", button_export_image_CB, this, "group='Export'");
    TwAddVarRO(mainBar, "Image Export Status", TW_TYPE_BOOLCPP, &export_velo_status, "group='Export' true='OK' false='ERROR' label='Export Status'");



    TwDefine((mainBarName + "/'Export' opened=false visible='false'").data());
  }

  void antMenu::addCameraParams()
  {
    std::string cameraBarName = "Camera";
    auto cameraBar = TwNewBar(cameraBarName.data());
    antBars.push_back(cameraBar);

    TwAddVarRO(cameraBar, "Pos x",  TW_TYPE_FLOAT, &cam_pos.x,   "group='Camera Position'");
    TwAddVarRO(cameraBar, "Pos y",  TW_TYPE_FLOAT, &cam_pos.y,   "group='Camera Position'");
    TwAddVarRO(cameraBar, "Pos z",  TW_TYPE_FLOAT, &cam_pos.z,   "group='Camera Position'");

    TwAddVarRO(cameraBar, "Dir x",  TW_TYPE_FLOAT, &cam_dir.x,   "group='Camera Direction'");
    TwAddVarRO(cameraBar, "Dir y",  TW_TYPE_FLOAT, &cam_dir.y,   "group='Camera Direction'");
    TwAddVarRO(cameraBar, "Dir z",  TW_TYPE_FLOAT, &cam_dir.z,   "group='Camera Direction'");

    TwAddVarRW(cameraBar, "New Pos x",  TW_TYPE_FLOAT, &n_cam_pos.x, "group='Set Position'");
    TwAddVarRW(cameraBar, "New Pos y",  TW_TYPE_FLOAT, &n_cam_pos.y, "group='Set Position'");
    TwAddVarRW(cameraBar, "New Pos z",  TW_TYPE_FLOAT, &n_cam_pos.z, "group='Set Position'");
             
    TwAddVarRW(cameraBar, "New Dir x",  TW_TYPE_FLOAT, &n_cam_dir.x, "group='Set Direction'");
    TwAddVarRW(cameraBar, "New Dir y",  TW_TYPE_FLOAT, &n_cam_dir.y, "group='Set Direction'");
    TwAddVarRW(cameraBar, "New Dir z",  TW_TYPE_FLOAT, &n_cam_dir.z, "group='Set Direction'");

    TwAddButton(cameraBar, "Apply Camera Settings", button_set_camera_CB, this, "");


    cam_pos     = glm::vec3(0, 0, 0);
    cam_dir     = glm::vec3(0, 0, 0);
    n_cam_pos   = glm::vec3(0, 0, 0);
    n_cam_dir   = glm::vec3(0, 0, 0);
    change_camera = false;
  }

  void antMenu::addTimers()
  {
    std::string timerBarName = "Timers";
    auto timerBar = TwNewBar(timerBarName.data());
    antBars.push_back(timerBar);

    export_timers_name_string = "Timers";

    TwAddVarRO(timerBar, "ms per frame", TW_TYPE_DOUBLE, &ms_per_frame, "");
    TwAddVarRO(timerBar, "ms per frame (frequent maximum)", TW_TYPE_DOUBLE, &ms_per_frame_lastMax, "");

    TwAddVarRW(timerBar, "<Filename>.txt", TW_TYPE_STDSTRING, &export_timers_name_string, "");
    TwAddButton(timerBar, "Export Timers", button_export_timers_CB, this, "");
    TwAddVarRO(timerBar, "Timer Export Status", TW_TYPE_BOOLCPP, &export_timers_status, "true='OK' false='ERROR' label='Export Status'");
  }

  void antMenu::addUtilTimer(std::string name, int pass_number, double timer, bool close)
  {
    auto timerBar = antBars[1];
    std::string timerBarName = TwGetBarName(timerBar);


    timers.push_back(timer);
    std::string timer_description = std::to_string(pass_number);
    // Because of the leading minus

    if (pass_number < 0)
      timer_description += " " + name;
    else if (pass_number < 10)
      timer_description += "  " + name;
    else
      timer_description += " " + name;

    timer_descs.push_back(timer_description);
    TwAddVarRO(timerBar, timer_description.data(), TW_TYPE_DOUBLE, &timers.back(), ("group='Pass " + std::to_string(pass_number) + "'" + " label='" + name + "'").data());

    if ((pass_number > 0) && close)
      TwDefine((timerBarName + "/'Pass " + std::to_string(pass_number - 1) + "' opened=false").data());
  }

  void antMenu::getIntegrationTimings(int pass_number)
  {

  }

  void antMenu::getRenderTimings()
  {

  }


  // Users
  // =====
  void antMenu::useSrcInfo(std::vector<std::size_t> grid, std::size_t grid_dim, std::size_t vec_len, std::size_t data_size, bool steady)
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    set_src_grid(grid, grid_dim);
    set_src_element_dim(vec_len);
    set_src_datasize(data_size);
    set_src_fieldtype(steady);
    calc_src_grid_string();

    TwDefine((mainBarName + "/'Source Info' visible='true'").data());
  }

  void antMenu::useCompInfo(std::size_t block_x, std::size_t block_y, std::size_t block_z, std::size_t dim_x, std::size_t dim_y, std::size_t dim_z, std::size_t img_len, std::size_t data_len)
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    set_cmp_dims(dim_x, dim_y, dim_z);
    set_cmp_datasize(data_len);
    set_cmp_imgsize(img_len);
    calc_cmp_img_count();
    set_cmp_blocksizes(block_x, block_y, block_z);
    calc_cmp_blocks_string();

    TwDefine((mainBarName + "/'Compressed Info' visible='true'").data());
  }

  void antMenu::useSeedingParams()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    TwDefine((mainBarName + "/'Range in X (from)' max=" + std::to_string(src_grid.x)).data());
    TwDefine((mainBarName + "/'Range in X (to)'   max=" + std::to_string(src_grid.x)).data());
    TwDefine((mainBarName + "/'Range in Y (from)' max=" + std::to_string(src_grid.y)).data());
    TwDefine((mainBarName + "/'Range in Y (to)'   max=" + std::to_string(src_grid.y)).data());
    TwDefine((mainBarName + "/'Range in Z (from)' max=" + std::to_string(src_grid.z)).data());
    TwDefine((mainBarName + "/'Range in Z (to)'   max=" + std::to_string(src_grid.z)).data());
                         
    set_seed_strategy(true);
    set_seed_stride({ 10U, 10U, 10U });
    set_seed_ranges({ 0, src_grid.x }, { 0, src_grid.y }, { 0, src_grid.z });
    calc_seed_directional();
    calc_seed_count();
    calc_seed_stride_string();
    calc_seed_directional_string();

    display_seed_params();

    TwDefine((mainBarName + "/'Seeding Parameters' visible='true'").data());
  }

  void antMenu::useIntegrationParams()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    calcIterations();
    calc_executed_steps();

    displayIntegrationParams();

    TwDefine((mainBarName + "/'Integration Parameters' visible='true'").data());
  }

  void antMenu::useShadingParams()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    TwDefine((mainBarName + "/'Shading Parameters' visible='true'").data());
  }

  void antMenu::useDrawingParams()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    draw_seeds = seed_count;
    TwDefine((mainBarName + "/'Drawing Parameters' visible='true'").data());
  }

  void antMenu::useExport()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    export_pos_name_string = "particle_positions";
    export_velo_name_string = "particle_velocities";
    calcExportSize();

    TwDefine((mainBarName + "/'Export' visible='true'").data());
  }

  void antMenu::useOGLInfo()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    TwDefine((mainBarName + "/'OpenGL Info' visible='true'").data());
  }

  void antMenu::useCameraParams(camera* cam)
  {
    cam->getCameraConfig(cam_pos, cam_dir);
    cam->getCameraConfig(n_cam_pos, n_cam_dir);
  }

  void antMenu::showSrcInfo()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    TwDefine((mainBarName + "/'Source Info' visible='true'").data());
  }

  void antMenu::showCompInfo()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    TwDefine((mainBarName + "/'Compressed Info' visible='true'").data());
  }

  void antMenu::showSeedingParams()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    TwDefine((mainBarName + "/'Seeding Parameters' visible='true'").data());
  }

  void antMenu::showIntegrationParams()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    TwDefine((mainBarName + "/'Integration Parameters' visible='true'").data());
  }

  void antMenu::showShadingParams()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    TwDefine((mainBarName + "/'Shading Parameters' visible='true'").data());
  }

  void antMenu::showDrawingParams()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    TwDefine((mainBarName + "/'Drawing Parameters' visible='true'").data());
  }

  void antMenu::showExport()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    TwDefine((mainBarName + "/'Export' visible='true'").data());
  }

  void antMenu::showOGLInfo()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    TwDefine((mainBarName + "/'OpenGL Info' visible='true'").data());
  }

  void antMenu::hideSrcInfo()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    TwDefine((mainBarName + "/'Source Info' visible='false'").data());
  }

  void antMenu::hideCompInfo()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    TwDefine((mainBarName + "/'Compressed Info' visible='false'").data());
  }

  void antMenu::hideSeedingParams()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    TwDefine((mainBarName + "/'Seeding Parameters' visible='false'").data());
  }

  void antMenu::hideIntegrationParams()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    TwDefine((mainBarName + "/'Integration Parameters' visible='false'").data());
  }

  void antMenu::hideShadingParams()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    TwDefine((mainBarName + "/'Shading Parameters' visible='false'").data());
  }

  void antMenu::hideDrawingParams()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    TwDefine((mainBarName + "/'Drawing Parameters' visible='false'").data());
  }

  void antMenu::hideExport()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    TwDefine((mainBarName + "/'Export' visible='false'").data());
  }

  void antMenu::hideOGLInfo()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    TwDefine((mainBarName + "/'OpenGL Info' visible='false'").data());
  }
  
  // Display Methods
  // ================
  void antMenu::display_seed_params()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    if (seed_strategy_strided)
    {
      TwDefine((mainBarName + "/'Set Seeds in X' visible='false'").data());
      TwDefine((mainBarName + "/'Set Seeds in Y' visible='false'").data());
      TwDefine((mainBarName + "/'Set Seeds in Z' visible='false'").data());
      TwDefine((mainBarName + "/'Stride' visible='false'").data());

      TwDefine((mainBarName + "/'Set Stride in X' visible='true'").data());
      TwDefine((mainBarName + "/'Set Stride in Y' visible='true'").data());
      TwDefine((mainBarName + "/'Set Stride in Z' visible='true'").data());
      TwDefine((mainBarName + "/'Seeds' visible='true'").data());

      seed_stride.x = (std::floor(seed_stride.x * 100.0))/100.0;
      seed_stride.y = (std::floor(seed_stride.y * 100.0))/100.0;
      seed_stride.z = (std::floor(seed_stride.z * 100.0))/100.0;
    }
    else
    {
      TwDefine((mainBarName + "/'Set Stride in X' visible='false'").data());
      TwDefine((mainBarName + "/'Set Stride in Y' visible='false'").data());
      TwDefine((mainBarName + "/'Set Stride in Z' visible='false'").data());
      TwDefine((mainBarName + "/'Seeds' visible='false'").data());

      TwDefine((mainBarName + "/'Set Seeds in X' visible='true'").data());
      TwDefine((mainBarName + "/'Set Seeds in Y' visible='true'").data());
      TwDefine((mainBarName + "/'Set Seeds in Z' visible='true'").data());
      TwDefine((mainBarName + "/'Stride' visible='true'").data());
    }
  }  

  void antMenu::displayIntegrationParams()
  {
    auto mainBar = antBars[0];
    std::string mainBarName = TwGetBarName(mainBar);

    if (int_unsteady)
    {
      TwDefine((mainBarName + "/'Cell Size in T' visible='true'").data());
      TwDefine((mainBarName + "/'Simulation Range in T' visible='true'").data());
    }
    else
    {
      TwDefine((mainBarName + "/'Cell Size in T' visible='false'").data());
      TwDefine((mainBarName + "/'Simulation Range in T' visible='false'").data());
    }
  }

  bool antMenu::change_camera_state()
  {
    auto result = change_camera;
    change_camera = false;
    return result;
  }

  void antMenu::set_camera_state(camera* cam)
  {
    cam->setCameraConfig(n_cam_pos, n_cam_dir);
  }

  void antMenu::update_camera_info(camera* cam)
  {
    cam->getCameraConfig(cam_pos, cam_dir);
  }

  // Regenerate Mesh
  // ===============
  bool antMenu::isDirty()   { return dirty; }
  void antMenu::markClean() { dirty = false; }
  void antMenu::markDirty()
  {
    draw_seeds = seed_count;

    dirty = true;
  }
}