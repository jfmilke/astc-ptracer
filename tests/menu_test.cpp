#include <catch2/catch.hpp>

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <jay/api.hpp>

struct generation_pass : jay::render_pass
{
  generation_pass(std::vector<glm::vec4>& positions, std::vector<glm::vec4>& colors, std::vector<std::uint32_t>& indices)
  : positions               (positions)
  , colors                  (colors   )
  , indices                 (indices  )
  , vertex_buffer           (globjects::Buffer ::create                 ())
  , color_buffer            (globjects::Buffer ::create                 ())
  , index_buffer            (globjects::Buffer ::create                 ())
  , compute_shader_source   (globjects::Shader ::sourceFromString       (compute_shader_source_text))
  , compute_shader_template (globjects::Shader ::applyGlobalReplacements(compute_shader_source.get()))
  , compute_shader          (globjects::Shader ::create                 (gl::GL_COMPUTE_SHADER, compute_shader_template.get()))
  , program                 (globjects::Program::create                 ())
  {
    on_prepare = [&] ( )
    {
      program->attach(compute_shader.get());
      program->link  ();

      vertex_buffer->bindBase(gl::GL_SHADER_STORAGE_BUFFER, 0);
      vertex_buffer->setData (positions, gl::GLenum::GL_DYNAMIC_DRAW);
       
      color_buffer ->bindBase(gl::GL_SHADER_STORAGE_BUFFER, 1);
      color_buffer ->setData (colors   , gl::GLenum::GL_DYNAMIC_DRAW);

      index_buffer ->bindBase(gl::GL_SHADER_STORAGE_BUFFER, 2);
      index_buffer ->setData (indices  , gl::GLenum::GL_DYNAMIC_DRAW);
      
      program->use();
      gl::glDispatchCompute(1, 1, 1);
      gl::glMemoryBarrier  (gl::GL_ALL_BARRIER_BITS);

      vertex_buffer->bind(gl::GL_ARRAY_BUFFER);
      positions = vertex_buffer->getSubData<glm::vec4>    (positions.size(), 0);
      vertex_buffer->bind(gl::GL_ARRAY_BUFFER);
      colors    = color_buffer ->getSubData<glm::vec4>    (colors   .size(), 0);
      vertex_buffer->bind(gl::GL_ARRAY_BUFFER);
      indices   = index_buffer ->getSubData<std::uint32_t>(indices  .size(), 0);
    };
    on_update = [&] ( )
    {
      // Runs once above.
    };
  }
  
  std::string compute_shader_source_text = R"(#version 450

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding=0) buffer vertices
{
  vec4 vertex_data[];
};
layout(std430, binding=1) buffer colors
{
  vec4 color_data[];
};
layout(std430, binding=2) buffer indices
{
  uint index_data[];
};

void main()
{
  vertex_data[0] = vec4(-0.7, -0.7, 0.0, 1.0);
  vertex_data[1] = vec4( 0.0,  0.7, 0.0, 1.0);
  vertex_data[2] = vec4( 0.7, -0.7, 0.0, 1.0);
  color_data [0] = vec4( 1.0, 0.0, 0.0, 1.0);
  color_data [1] = vec4( 0.0, 1.0, 0.0, 1.0);
  color_data [2] = vec4( 0.0, 0.0, 1.0, 1.0);
  index_data [0] = 0;
  index_data [1] = 1;
  index_data [2] = 2;
}
)";

  std::vector<glm::vec4>&                          positions               ;
  std::vector<glm::vec4>&                          colors                  ;
  std::vector<std::uint32_t>&                      indices                 ;

  std::unique_ptr<globjects::Buffer>               vertex_buffer           ;
  std::unique_ptr<globjects::Buffer>               color_buffer            ;
  std::unique_ptr<globjects::Buffer>               index_buffer            ;

  std::unique_ptr<globjects::StaticStringSource>   compute_shader_source   ;
  std::unique_ptr<globjects::AbstractStringSource> compute_shader_template ;
  std::unique_ptr<globjects::Shader>               compute_shader          ;

  std::unique_ptr<globjects::Program>              program                 ;
};
struct rendering_pass  : jay::render_pass
{
  rendering_pass(const std::vector<glm::vec4>& positions, const std::vector<glm::vec4>& colors, const std::vector<std::uint32_t>& indices, jay::antMenu* menu) 
  : positions               (positions)
  , colors                  (colors   )
  , indices                 (indices  )
  , vertex_buffer           (globjects::Buffer     ::create                 ())
  , color_buffer            (globjects::Buffer     ::create                 ())
  , index_buffer            (globjects::Buffer     ::create                 ())
  , vertex_array            (globjects::VertexArray::create                 ())
  , vertex_shader_source    (globjects::Shader     ::sourceFromString       (vertex_shader_source_text  ))
  , fragment_shader_source  (globjects::Shader     ::sourceFromString       (fragment_shader_source_text))
  , vertex_shader_template  (globjects::Shader     ::applyGlobalReplacements(vertex_shader_source  .get()))
  , fragment_shader_template(globjects::Shader     ::applyGlobalReplacements(fragment_shader_source.get()))
  , vertex_shader           (globjects::Shader     ::create                 (gl::GL_VERTEX_SHADER  , vertex_shader_template  .get()))
  , fragment_shader         (globjects::Shader     ::create                 (gl::GL_FRAGMENT_SHADER, fragment_shader_template.get()))
  , program                 (globjects::Program    ::create                 ())
  , menu(menu)
  {
    on_prepare = [&] ( )
    {
      program->attach(vertex_shader  .get());
      program->attach(fragment_shader.get());
      program->link  ();
    };
    on_update = [&] ( )
    {
      vertex_array ->bind   ();
      //std::cout << "b" << std::endl;

      //std::cout << this->menu->seedsPerDirStr << std::endl;

      vertex_buffer->bind   (gl::GLenum::GL_ARRAY_BUFFER);
      vertex_buffer->setData(positions, gl::GLenum::GL_STATIC_DRAW);
      gl::glVertexAttribPointer    (0, 4, gl::GL_FLOAT, gl::GL_FALSE, 0, nullptr);
      gl::glEnableVertexAttribArray(0);
      
      color_buffer ->bind   (gl::GLenum::GL_ARRAY_BUFFER);
      color_buffer ->setData(colors, gl::GLenum::GL_STATIC_DRAW);
      gl::glVertexAttribPointer    (1, 4, gl::GL_FLOAT, gl::GL_FALSE, 0, nullptr);
      gl::glEnableVertexAttribArray(1);

      index_buffer ->bind   (gl::GLenum::GL_ELEMENT_ARRAY_BUFFER);
      index_buffer ->setData(indices, gl::GLenum::GL_STATIC_DRAW);
      
      program     ->use ();
      vertex_array->bind();
      gl::glDrawElements(gl::GL_TRIANGLES, indices.size(), gl::GL_UNSIGNED_INT, nullptr);
    };
  }

  std::string vertex_shader_source_text = R"(#version 450
layout(location = 0) in  vec4 vert_position;
layout(location = 1) in  vec4 vert_color   ;
layout(location = 0) out vec4 frag_color   ;
void main()
{
  frag_color  = vert_color;
  gl_Position = vert_position;
}
)";
  std::string fragment_shader_source_text = R"(#version 450
layout(location = 0) in  vec4 frag_color ;
layout(location = 0) out vec4 final_color;
void main()
{
  final_color = frag_color;
}
)";

  const std::vector<glm::vec4>&                    positions               ;
  const std::vector<glm::vec4>&                    colors                  ;
  const std::vector<std::uint32_t>&                indices                 ;

  std::unique_ptr<globjects::Buffer>               vertex_buffer           ;
  std::unique_ptr<globjects::Buffer>               color_buffer            ;
  std::unique_ptr<globjects::Buffer>               index_buffer            ;
  std::unique_ptr<globjects::VertexArray>          vertex_array            ;

  std::unique_ptr<globjects::StaticStringSource>   vertex_shader_source    ;
  std::unique_ptr<globjects::StaticStringSource>   fragment_shader_source  ;
  std::unique_ptr<globjects::AbstractStringSource> vertex_shader_template  ;
  std::unique_ptr<globjects::AbstractStringSource> fragment_shader_template;
  std::unique_ptr<globjects::Shader>               vertex_shader           ;
  std::unique_ptr<globjects::Shader>               fragment_shader         ;

  std::unique_ptr<globjects::Program>              program                 ;

  jay::antMenu*                                    menu;
};

TEST_CASE("Menu test.", "[jay::engine]")
{
  auto application = std::make_unique<jay::application>();
  auto renderer    = application->add_system<jay::renderer>();
  auto menu        = application->add_menu();
  renderer->add_render_pass<jay::render_pass>(jay::make_clear_pass(application->window()));

  // TODO: Replace with your render passes.
  std::vector<glm::vec4>     positions(3);
  std::vector<glm::vec4>     colors(3);
  std::vector<std::uint32_t> indices(3);

  // Known beforehand:
  std::string              filepath = "../files/";
  std::string              filename = "ctbl3d";
  std::string              fileending = ".nc";
  std::vector<std::string> datasets = { "u", "v", "w" };

  // Some fake values for uncompressed data
  std::vector<std::size_t> grid = { 100, 100, 10 };
  std::size_t grid_dim          = 3;
  std::size_t vec_len           = 3;
  std::size_t dataset_size      = 1000;
  bool steady                   = true;

  // Some fake values for compressed data
  std::size_t block_x  = 4;
  std::size_t block_y  = 4;
  std::size_t block_z  = 1;
  std::size_t dim_x    = 100;
  std::size_t dim_y    = 100;
  std::size_t dim_z    = 1;
  std::size_t img_len  = 100;
  std::size_t data_len = 1000;

  // Add to menu
  menu->useSrcInfo(grid, grid_dim, vec_len, dataset_size, steady);
  menu->useCompInfo(block_x, block_y, block_z, dim_x, dim_y, dim_z, img_len, data_len);
  menu->useSeedingParams();
  menu->useIntegrationParams();
  menu->useShadingParams();
  menu->useDrawingParams();
  menu->useExport();
  menu->useOGLInfo();

  renderer->add_render_pass<generation_pass>(positions, colors, indices);
  renderer->add_render_pass<rendering_pass>(positions, colors, indices, menu);

  renderer->add_render_pass<jay::menu_pass>(jay::make_menu_pass());
  renderer->add_render_pass<jay::render_pass>(jay::make_swap_pass(application->window()));
  application->run();
};