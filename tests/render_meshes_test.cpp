#include <catch2/catch.hpp>

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>

#include <chrono>

#include <jay/api.hpp>
#include<boost/math/statistics/univariate_statistics.hpp>
#include <boost/range/adaptor/strided.hpp>
#include <boost/range/adaptors.hpp>

#include <iterator>
#include <iostream>
#include <vector>

#include <iostream>


TEST_CASE("Jay Steady Test.", "[jay::engine]")
{
  bool render_uncompressed = true;
  auto application = std::make_unique<jay::application>();

  std::vector<float> a{ 1,2,3,4,5,6,7,8,9 };


  // Add subsystems
  auto renderer = application->add_system<jay::renderer>();

  renderer->add_render_pass<jay::render_pass>(jay::make_clear_pass(application->window()));

  // Uncompressed
  // Known beforehand:
  std::string              filepath = "../files/exports/";
  std::string              pos0_fn  = "tangaroa_EXH-3N-Mask-4x4.positions";
  std::string              velo0_fn = "tangaroa_EXH-3N-Mask-4x4.velocities";
  //std::string              pos1_fn = "ctbl3d_cmp.positions";
  //std::string              velo1_fn = "ctbl3d_cmp.velocities";
  std::vector<std::string> datasets = { "u", "v", "w" };

  // Start Filedriver
  jay::io filedriver = jay::io();

  // Load data
  filedriver.hdf5_open("../files/tangaroa.nc", datasets);
  auto grid = filedriver.hdf5_get_grid();


  auto camera = application->add_camera(grid);
  auto menu = application->add_menu();

  menu->useSrcInfo(grid, grid.size(), 3, 0, grid.size() > 3);
  menu->useShadingParams();
  menu->useDrawingParams();
  menu->useExport();
  menu->useCameraParams(camera);

  // Create a steady field
  jay::advected_field* a_field0 = new jay::advected_field();
  a_field0->init_configuration(menu);
  a_field0->load_positions(filepath + pos0_fn);
  a_field0->load_velocities(filepath + velo0_fn);

  //jay::advected_field* a_field1 = new jay::advected_field();
  //a_field1->init_configuration(menu);
  //a_field1->d_conf->base_color = glm::vec4(1.0, 0.0, 0.0, 1.0);
  //a_field1->load_positions(filepath + pos1_fn);
  //a_field1->load_velocities(filepath + velo1_fn);

  // Render Advection
  renderer->add_render_pass<jay::render_advection_pass>(jay::make_render_advection_pass(menu, camera, a_field0));
  //renderer->add_render_pass<jay::render_advection_pass>(jay::make_render_advection_pass(menu, camera, a_field1));

  // Update Camera
  renderer->add_render_pass<jay::camera_pass>(jay::make_camera_pass(application->window(), camera, application.get()));

  // Capture images
  renderer->add_render_pass<jay::image_pass>(jay::make_image_pass(menu));

  // Render the menu
  renderer->add_render_pass<jay::menu_pass>(jay::make_menu_pass());

  // Swap pass
  renderer->add_render_pass<jay::render_pass>(jay::make_swap_pass(application->window()));

  application->run();
};