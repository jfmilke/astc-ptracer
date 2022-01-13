#include <catch2/catch.hpp>

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>

#include <chrono>

#include <jay/api.hpp>

TEST_CASE("Jay Unsteady Test.", "[jay::engine]")
{
  bool render_uncompressed = true;
  auto application = std::make_unique<jay::application>();
  
  // Add subsystems
  auto renderer = application->add_system<jay::renderer>();
  auto camera   = application->add_camera();
  auto menu     = application->add_menu();
  
  renderer->add_render_pass<jay::render_pass>(jay::make_clear_pass(application->window()));

  renderer->add_render_pass<jay::timer_pass>(jay::make_timer_pass(menu));

  if (render_uncompressed)
  {
    // Uncompressed
    // Known beforehand:
    std::string              filepath = "../files/";
    std::string              filename = "tangaroa.nc";
    std::vector<std::string> datasets = { "u", "v", "w" };

    std::string img_name = filename.substr(0, filename.find_last_of("."));

    // Start Filedriver
    jay::io filedriver = jay::io();
    filedriver.hdf5_open(filepath + filename, datasets);

    // Load data
    auto grid = filedriver.hdf5_get_grid();
    //auto hdf5_data = filedriver.hdf5_read_subset<float>({ 4, 4, 4, 4 }, { 0, 0, 0, 0 });
    // auto hdf5_data = filedriver.hdf5_read_subset<float>({ grid[0], grid[1], grid[2], grid[3] }, { 0, 0, 0, 0 });
    // Alternatively: Read full dataset
    auto hdf5_data = filedriver.hdf5_read<float>();

    // Create a steady field
    jay::vector_field* v_field = new jay::vector_field(false);

    menu->useSrcInfo(filename, hdf5_data, false);
    menu->useSeedingParams();
    menu->useIntegrationParams();
    menu->useShadingParams();
    menu->useDrawingParams();
    menu->useExport();
    menu->useOGLInfo();

    menu->seed_strategy_strided = false;
    menu->seed_directional = glm::uvec3(15, 30, 15);
    menu->calc_seed_stride();
    menu->calc_seed_stride_string();
    menu->calc_seed_count();
    menu->display_seed_params();
    menu->int_step_size_dt = 0.1;
    menu->int_step_size_h = 0.1;
    menu->int_global_step_count = 2000;
    menu->int_strategy_rk4 = true;
    menu->int_simulation_range = glm::vec4(1, 0.6, 0.2, 2.01);
    menu->calc_executed_steps();
    menu->displayIntegrationParams();
    menu->calc_cell_size();
    menu->int_cell_size.w = 1.0;

    menu->export_timers_name_string = img_name;
    menu->export_pos_name_string = img_name;
    menu->export_velo_name_string = img_name;
    menu->calcExportSize();

    // Automatic images
    renderer->add_render_pass<jay::auto_pass>(jay::make_auto_pass(menu, camera, img_name));

    // Advect
    renderer->add_render_pass<jay::advection_pass>(jay::make_advection_pass(hdf5_data.data, v_field, menu));

    // Update Camera
    renderer->add_render_pass<jay::camera_pass>(jay::make_camera_pass(application->window(), camera, application.get()));

    // Render Advection
    renderer->add_render_pass<jay::render_advection_pass>(jay::make_render_advection_pass(menu, camera, v_field->get_result()));

    // Capture images
    renderer->add_render_pass<jay::image_pass>(jay::make_image_pass(menu));

    // Render the menu
    renderer->add_render_pass<jay::menu_pass>(jay::make_menu_pass());

    // Swap pass
    renderer->add_render_pass<jay::render_pass>(jay::make_swap_pass(application->window()));

    application->run();
  }
  else
  {
    // ASTC compressed
    // Known beforehand:
    std::string              filepath  = "../files/";
    std::string              filename  = "tangaroa_EXH-1N-Mask-6x6x1.astc";
    std::string              filename_src  = "tangaroa.nc";
    std::string              peaksname = "tangaroa0.peaks";
    std::vector<std::string> datasets = { "u", "v", "w" };
    std::string img_name = filename.substr(0, filename.find_last_of("x"));

    // Start Filedriver
    jay::io filedriver = jay::io();

    // Load data
    auto astc_data = filedriver.astc_read(filepath + filename);
    auto peaks     = filedriver.read_vector<float>(filepath + peaksname);
    filedriver.hdf5_open(filepath + filename_src, datasets);
    auto grid = filedriver.hdf5_get_grid();


    // Create a steady field
    jay::vector_field* v_field = new jay::vector_field(false);

    menu->useSrcInfo(grid, grid.size(), 3, 0, false);
    menu->useCompInfo(filename, astc_data, 1);
    menu->useSeedingParams();
    menu->useIntegrationParams();
    menu->useShadingParams();
    menu->useDrawingParams();
    menu->useExport();
    menu->useOGLInfo();

    menu->seed_strategy_strided = false;
    menu->seed_directional = glm::uvec3(15, 30, 15);
    menu->calc_seed_stride();
    menu->calc_seed_stride_string();
    menu->calc_seed_count();
    menu->display_seed_params();
    menu->int_step_size_dt = 0.1;
    menu->int_step_size_h = 0.1;
    menu->int_global_step_count = 2000;
    menu->int_strategy_rk4 = true;
    menu->int_simulation_range = glm::vec4(1, 0.6, 0.2, 2.01);
    menu->calc_executed_steps();
    menu->displayIntegrationParams();
    menu->calc_cell_size();
    menu->int_cell_size.w = 1.0;

    menu->export_timers_name_string = img_name;
    menu->export_pos_name_string = img_name;
    menu->export_velo_name_string = img_name;
    menu->calcExportSize();

    // Automatic images
    renderer->add_render_pass<jay::auto_pass>(jay::make_auto_pass(menu, camera, img_name));

    // Advect
    renderer->add_render_pass<jay::advection_pass>(jay::make_advection_pass(astc_data.data, peaks, v_field, menu));

    // Update Camera
    renderer->add_render_pass<jay::camera_pass>(jay::make_camera_pass(application->window(), camera, application.get()));

    // Render Advection
    renderer->add_render_pass<jay::render_advection_pass>(jay::make_render_advection_pass(menu, camera, v_field->get_result()));

    // Capture images
    renderer->add_render_pass<jay::image_pass>(jay::make_image_pass(menu));

    // Render the menu
    renderer->add_render_pass<jay::menu_pass>(jay::make_menu_pass());

    // Swap pass
    renderer->add_render_pass<jay::render_pass>(jay::make_swap_pass(application->window()));

    application->run();
  }  
};