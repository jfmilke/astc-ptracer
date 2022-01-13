#ifndef JAY_CORE_MENU_HPP
#define JAY_CORE_MENU_HPP

#include<AntTweakBar.h>
#include<jay/graphics/renderer.hpp>
#include<jay/types/jaydata.hpp>
#include<glm/vec2.hpp>
#include<glm/vec3.hpp>
#include<glm/vec4.hpp>

namespace jay
{
  typedef struct camera camera;

  // =====================
  // ANTTWEAKBAR CALLBACKS
  // =====================

  // String Handler
  // ==============
  void TW_CALL CopyStdStringToClient(std::string& destinationClientString, const std::string& sourceLibraryString);

  // Button Callbacks
  // ================
  void TW_CALL buttonDirtyCB(void* clientData);
  void TW_CALL button_export_positions_CB(void* clientData);
  void TW_CALL button_export_velos_CB(void* clientData);
  void TW_CALL button_export_timers_CB(void* clientData);
  void TW_CALL button_calc_cell_size_CB(void* clientData);
  void TW_CALL button_set_camera_CB(void* clientData);


  // CB Var Callbacks
  // ================
  void TW_CALL getStrideStrCB(void* value, void* clientData);
  void TW_CALL getSPDStrCB(void* value, void* clientData);
  void TW_CALL getSeedsCB(void* value, void* clientData);
  void TW_CALL setSeedingStrategyCB(const void* value, void* clientData);
  void TW_CALL getSeedingStrategyCB(void* value, void* clientData);
  void TW_CALL setIntegrationTechniqueCB(const void* value, void* clientData);
  void TW_CALL getIntegrationTechniqueCB(void* value, void* clientData);
  void TW_CALL setIntegrationTypeCB(const void* value, void* clientData);
  void TW_CALL getIntegrationTypeCB(void* value, void* clientData);
  void TW_CALL set_int_stepsize_divergent(const void* value, void* clientData);
  void TW_CALL get_int_stepsize_divergent(void* value, void* clientData);


  struct JAY_EXPORT antMenu
  {
    antMenu();

    // Source
    void set_src_grid(std::vector<std::size_t>& grid, int grid_dim);
    void set_src_element_dim(std::uint32_t vec_len);
    void set_src_datasize(std::size_t datasize_bytes);
    void set_src_fieldtype(bool steady);
    void calc_src_grid_string();

    // Compressed
    void set_cmp_dims(std::size_t dim_x, std::size_t dim_y, std::size_t dim_z);
    void set_cmp_datasize(std::size_t datasize_bytes);
    void set_cmp_imgsize(std::size_t imgsize_bytes);
    void set_cmp_blocksizes(std::size_t block_x, std::size_t block_y, std::size_t block_z);
    void calc_cmp_img_count();
    void calc_cmp_blocks_string();

    // Seeding
    void set_seed_strategy(bool strided);
    void set_seed_ranges(glm::uvec2 rangeX, glm::uvec2 rangeY, glm::uvec2 rangeZ);
    void set_seed_stride(glm::uvec3 stride);
    void set_seed_directional(glm::vec3 spd);
    void calc_seed_stride();
    void calc_seed_stride_string();
    void calc_seed_directional();
    void calc_seed_directional_string();
    void calc_seed_count();
    void display_seed_params();

    // Integration
    void calc_cell_size();
    void calc_step_size();
    void calc_executed_steps();
    void calcIterations();
    void displayIntegrationParams();

    void calcExportSize();

    void calcOGLInfoStrs();

    // Camera
    bool change_camera_state();
    void set_camera_state(camera* cam);
    void update_camera_info(camera* cam);


    void useSrcInfo(std::vector<std::size_t> grid, std::size_t grid_dim, std::size_t vec_len, std::size_t data_size = 0, bool steady = true);
    void useCompInfo(std::size_t block_x, std::size_t block_y, std::size_t block_z, std::size_t dim_x, std::size_t dim_y, std::size_t dim_z, std::size_t img_len, std::size_t data_len);
    void useOGLInfo();
    void useSeedingParams();
    void useIntegrationParams();
    void useShadingParams();
    void useDrawingParams();
    void useExport();
    void useCameraParams(camera* cam);

    void showSrcInfo();
    void showCompInfo();
    void showOGLInfo();
    void showSeedingParams();
    void showIntegrationParams();
    void showShadingParams();
    void showDrawingParams();
    void showExport();

    void hideSrcInfo();
    void hideCompInfo();
    void hideOGLInfo();
    void hideSeedingParams();
    void hideIntegrationParams();
    void hideShadingParams();
    void hideDrawingParams();
    void hideExport();

    void addUtilTimer(std::string name, int pass_number, double timer, bool close = true);
    void getIntegrationTimings(int pass_number);
    void getRenderTimings();


    bool export_positions();
    bool export_velocities();
    bool export_image();
    void export_timers();

    bool isDirty();
    void markDirty();
    void markClean();

    float        getDrawPercent();
    std::int32_t getDrawCount();

    // Add Information
    // ===============
    template <typename T>
    void useSrcInfo(std::string dataset_name, jaySrc<T>& srcData, bool steady = true)
    {
      auto mainBar = antBars[0];
      std::string mainBarName = TwGetBarName(mainBar);

      ds_name = dataset_name;
      set_src_grid        (srcData.grid, srcData.grid_dim);
      calc_src_grid_string();
      set_src_element_dim     (srcData.vec_len);
      set_src_datasize    (srcData.data.size() * sizeof(T));
      set_src_fieldtype(steady);

      TwDefine((mainBarName + "/'Source Info' visible='true'").data());
    }

    template <typename T>
    void useCompInfo(std::string dataset_name, jayComp<T>& cmpData, std::uint32_t pixelPerElement)
    {
      auto mainBar = antBars[0];
      std::string mainBarName = TwGetBarName(mainBar);

      ds_name = dataset_name;
      set_cmp_dims(cmpData.dim_x, cmpData.dim_y, cmpData.dim_z);
      set_cmp_datasize        (cmpData.data_len * sizeof(T));
      set_cmp_imgsize         (cmpData.img_len  * sizeof(T));
      calc_cmp_img_count       ();
      set_cmp_blocksizes      (cmpData.block_x, cmpData.block_y, cmpData.block_z);
      calc_cmp_blocks_string    ();

      TwDefine((mainBarName + "/'Compressed Info' visible='true'").data());
    }

    // Variables
    // =========
    std::string  ds_name;

    // Show Info
    glm::uvec4   src_grid;
    std::int32_t src_grid_dim;
    double       src_datasize;
    std::int32_t src_element_dim;
    bool         src_steady;

    // Show Compressed
    glm::uvec3    cmp_img_dimensions;
    glm::uvec3    cmp_block_sizes;
    std::size_t   cmp_data_size;
    std::size_t   cmp_img_size;
    double        cmp_data_size_mb;
    double        cmp_img_size_mb;
    std::uint32_t cmp_img_count;

    // Show ASTC Specific

    // Seeding Params
    bool          seed_strategy_strided; 
    glm::vec3     seed_stride;            
    glm::uvec2    seed_range_x;            
    glm::uvec2    seed_range_y;
    glm::uvec2    seed_range_z;
    glm::uvec3    seed_directional;              
    std::uint32_t seed_count;

    // Integration Params
    bool          int_strategy_rk4;
    bool          int_unsteady;
    bool          int_texelfetch;
    glm::vec4     int_simulation_range;
    std::uint32_t int_global_step_count;
    std::uint32_t int_local_step_count;
    std::uint32_t int_remainder_step_count;
    glm::vec4     int_cell_size;
    float         int_step_size_h;
    float         int_step_size_dt;
    float         int_dataset_factor;
    std::uint32_t int_iteration_count;
    std::uint32_t int_vram;


    // Shading Params (Live)
    float         shading_min_value;
    float         shading_max_value;
    float         shading_highlight_threshold;
    glm::vec3     shading_highlight_color;
    float         shading_damping_factor;

    // Drawing Params (Live)
    float         draw_vertices_per_seed; 
    std::uint32_t draw_seeds;
    std::uint32_t draw_offset;
    std::uint32_t generated_seed_vertices;

    // Export
    double        export_pos_size;
    bool          export_pos;
    bool          export_pos_status;
    double        export_velo_size;
    bool          export_velo;
    bool          export_velo_status;
    int           export_timers_count;
    bool          export_timers_status;
    bool          export_img;
    bool          export_img_status;

    // Camera
    glm::vec3 cam_pos;
    glm::vec3 cam_dir;
    glm::vec3 cam_up;
    glm::vec3 cam_right;
    glm::vec3 n_cam_pos;
    glm::vec3 n_cam_dir;
    glm::vec3 n_cam_up;
    glm::vec3 n_cam_right;
    bool change_camera;

    // Capabilities
    std::size_t ogl_cs_max_ssbos;
    std::size_t ogl_cs_max_ubos;
    std::size_t ogl_cs_max_textures;
    std::size_t ogl_cs_max_workgroups_x;
    std::size_t ogl_cs_max_workgroups_y;
    std::size_t ogl_cs_max_workgroups_z;
    std::size_t ogl_max_size_3dtex;
    std::size_t ogl_max_layers_2darraytex;
    std::size_t ogl_max_size_2dtex;
    std::size_t ogl_max_size_texturebuffers;
    std::size_t ogl_max_count_textures;
    std::size_t ogl_count_compression_formats;
    bool        ogl_support_4d_tex;
    bool        ogl_support_astc_ldr;
    bool        ogl_support_astc_sliced;
    bool        ogl_support_astc_hdr;

    // Strings of versions from above
    std::string src_grid_string;
    std::string field_type_string;
    std::string cmp_img_dims_string;
    std::string cmp_blocks_string;
    std::string seed_stride_string;
    std::string seed_directional_string;
    std::string export_pos_name_string;
    std::string export_velo_name_string;
    std::string export_timers_name_string;
    std::string export_img_name = "img.bmp";

    std::vector<std::string> timer_descs;
    std::vector<double> timers;
    double ms_per_frame;
    double ms_per_frame_lastMax;

  private:
    void addSrcInfo();
    void addCompInfo();
    void addOGLInfo();
    void addSeedingParams();
    void addIntegrationParams();
    void addShadingParams();
    void addDrawingParams();
    void addExport();
    void addCameraParams();
    void addTimers();

    std::vector<TwBar*> antBars;  // Holds menu windows (currently just one)

    std::string ogl_cs_max_ssbos_str;
    std::string ogl_cs_max_ubos_str;
    std::string ogl_cs_max_textures_str;
    std::string ogl_cs_max_workgroups_str;
    std::string ogl_max_size_3dtex_str;
    std::string ogl_max_layers_2darraytex_str;
    std::string ogl_max_size_2dtex_str;
    std::string ogl_max_size_texturebuffers_str;
    std::string ogl_max_count_textures_str;
    std::string ogl_count_compression_formats_str;

    bool dirty;
  };
}

#endif