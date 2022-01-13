#ifndef JAY_ADVECTION_ADVECTED_FIELD_HPP
#define JAY_ADVECTION_ADVECTED_FIELD_HPP

#include <jay/advection/field_objects.hpp>

#include <jay/export.hpp>
#include <glm/glm.hpp>
#include <globjects/globjects.h>
#include <globjects/base/StaticStringSource.h>
#include <globjects/base/AbstractStringSource.h>
#include <globjects/base/File.h>
#include <jay/advection/field_objects.hpp>


namespace jay
{
  typedef struct antMenu antMenu;
  typedef struct camera camera;

  struct JAY_EXPORT advected_field
  {
    render_conf* r_conf = nullptr;
    shading_conf* s_conf = nullptr;
    draw_conf* d_conf = nullptr;

    std::unique_ptr<globjects::VertexArray> vao    = nullptr;
    std::unique_ptr<globjects::Buffer> b_positions = nullptr;
    std::unique_ptr<globjects::Buffer> b_velocity  = nullptr;
    std::unique_ptr<globjects::Buffer> b_indices   = nullptr;
    std::unique_ptr<globjects::Buffer> b_utility0  = nullptr;
    std::unique_ptr<globjects::Buffer> b_utility1  = nullptr;
    std::unique_ptr<globjects::Buffer> b_utility2 = nullptr;

    //std::unique_ptr<globjects::StaticStringSource>   vertex_shader_source   = nullptr;
    std::unique_ptr<globjects::File>   vertex_shader_source   = nullptr;
    std::unique_ptr<globjects::AbstractStringSource> vertex_shader_template = nullptr;
    std::unique_ptr<globjects::Shader>               vertex_shader          = nullptr;

    std::unique_ptr<globjects::StaticStringSource>   geometry_shader_source = nullptr;
    std::unique_ptr<globjects::AbstractStringSource> geometry_shader_template = nullptr;
    std::unique_ptr<globjects::Shader>               geometry_shader = nullptr;

    //std::unique_ptr<globjects::StaticStringSource>   fragment_shader_source   = nullptr;
    std::unique_ptr<globjects::File>   fragment_shader_source   = nullptr;
    std::unique_ptr<globjects::AbstractStringSource> fragment_shader_template = nullptr;
    std::unique_ptr<globjects::Shader>               fragment_shader          = nullptr;

    std::unique_ptr<globjects::Program> shader_program = nullptr;

    bool steady_advection = true;
    
    double t_frames_per_second = 0.0;       // FPS, averaged over 10 seconds
    double t_milliseconds_per_frame = 0.0;  // Inverse of FPS, averaged over 100 frames

    advected_field();
    void init_configuration(antMenu* menu);
    void init_render_conf(antMenu* menu);
    void init_shading_conf(antMenu* menu);
    void init_draw_conf(antMenu* menu);
    void update_configuration(antMenu* menu);
    void update_shading_conf(antMenu* menu);
    void update_draw_conf(antMenu* menu);
    void update_render_conf();
    void update_menu(antMenu* menu);

    void setup_render_shader();
    void enable_array_buffers();
    void enable_element_array_buffer();
    void setup_triangle_element_array_buffer();
    void setup_triangle_strip_element_array_buffer();
    void export_positions(antMenu* menu);
    void export_velocities(antMenu* menu);
    void export_image(antMenu* menu);
    void load_positions(std::string filepath);
    void load_velocities(std::string filepath);
    void update_draw_range();
    void live_update_uniforms(camera* cam);
    void live_update_draw_percent();
    void draw();
    void draw_triangle_elements();
    void draw_triangle_strip_elements();



    // Returns a vector holding the content of the specified buffer.
    // Content size must exactly match the number of retrievable items.
    template <typename T>
    std::vector<T> read_buffer(std::unique_ptr<globjects::Buffer>& buffer, std::size_t offset = 0, std::size_t read = 0)
    {
      auto b_size = buffer->getParameter64(gl::GL_BUFFER_SIZE);
      if (read == 0)
        read = b_size / sizeof(T);

      if (b_size == 0)
        return std::vector<T>();

      return buffer->getSubData<T>(read, offset);
    }

    // Returns a vector holding the content of the velocity buffer.
    // If T holds all vector components at once (e.g. glm::vec3) set vec_len = 1.
    template <typename T>
    std::vector<T> get_positions(std::size_t offset = 0, std::size_t read = 0)
    {
      return read_buffer<T>(b_positions, offset, read);
    }

    // Returns a vector holding the content of the position buffer.
    // If T holds all vector components at once (e.g. glm::vec3) set vec_len = 1.
    template <typename T>
    std::vector<T> get_velocity(std::size_t offset = 0, std::size_t read = 0)
    {
      return read_buffer<T>(b_velocity, offset, read);
    }

    std::vector<gl::GLint>   vertexFirst;
    std::vector<gl::GLsizei> vertexCount;
    std::vector<void*>       indexFirst;
    std::vector<int>         indexCount;

    protected:
      glm::mat4 projection = glm::mat4(1.0);
      glm::mat4 view = glm::mat4(1.0);
      glm::mat4 model = glm::mat4(1.0);
      glm::mat4 u_mvp = glm::mat4(1.0);

      gl::GLsizei draw_steps = 0;
      gl::GLsizei draw_seeds = 0;
  };
}

#endif