#include <catch2/catch.hpp>

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>

#include <chrono>

#include <jay/api.hpp>


TEST_CASE("Jay Unsteady Test.", "[jay::engine]")
{
  bool render_uncompressed = false;
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

    // Advect
    renderer->add_render_pass<jay::advection_pass>(jay::make_advection_pass(hdf5_data.data, v_field, menu));

    menu->useSrcInfo(filename, hdf5_data, false);
    menu->useSeedingParams();
    menu->useIntegrationParams();
    menu->useShadingParams();
    menu->useDrawingParams();
    menu->useExport();
    menu->useOGLInfo();

    // Render Advection
    renderer->add_render_pass<jay::render_advection_pass>(jay::make_render_advection_pass(menu, camera, v_field->get_result()));

    // Update Camera
    renderer->add_render_pass<jay::camera_pass>(jay::make_camera_pass(application->window(), camera, application.get()));

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
    std::string              filename  = "tangaroa_EXH-1N-Mask-4x4x1.astc";
    std::string              peaksname = "tangaroa0.peaks";
    std::vector<std::string> datasets = { "u", "v", "w" };
    std::vector<std::size_t> grid     = { 300, 180, 120, 201 };

    // Start Filedriver
    jay::io filedriver = jay::io();

    // Load data
    auto astc_data = filedriver.astc_read(filepath + filename);
    auto peaks     = filedriver.read_vector<float>(filepath + peaksname);

    // Create a steady field
    jay::vector_field* v_field = new jay::vector_field(false);

    // Advect
    renderer->add_render_pass<jay::advection_pass>(jay::make_advection_pass(astc_data.data, peaks, v_field, menu));
    menu->useSrcInfo(grid, 4, 3, 0, false);
    menu->useCompInfo(filename, astc_data, 1);
    menu->useSeedingParams();
    menu->useIntegrationParams();
    menu->useShadingParams();
    menu->useDrawingParams();
    menu->useExport();
    menu->useOGLInfo();

    // Render Advection
    renderer->add_render_pass<jay::render_advection_pass>(jay::make_render_advection_pass(menu, camera, v_field->get_result()));

    // Update Camera
    renderer->add_render_pass<jay::camera_pass>(jay::make_camera_pass(application->window(), camera, application.get()));

    // Capture images
    renderer->add_render_pass<jay::image_pass>(jay::make_image_pass(menu));

    // Render the menu
    renderer->add_render_pass<jay::menu_pass>(jay::make_menu_pass());

    // Swap pass
    renderer->add_render_pass<jay::render_pass>(jay::make_swap_pass(application->window()));

    application->run();
  }  
};