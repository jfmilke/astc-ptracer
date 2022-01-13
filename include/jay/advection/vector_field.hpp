#ifndef JAY_ADVECTION_VECTOR_FIELD_HPP
#define JAY_ADVECTION_VECTOR_FIELD_HPP

#include <glm/glm.hpp>

#include <jay/advection/field_objects.hpp>

#include <jay/advection/advected_field.hpp>
#include <jay/types/image.hpp>
#include <jay/types/jaydata.hpp>
#include <jay/analysis/performance_measure.hpp>

#include <glbinding/gl/enum.h>

#include <vector>

#include <jay/export.hpp>

namespace jay
{
  struct JAY_EXPORT vector_field
  {
    compute_conf* c_conf = nullptr;
    texture_conf* t_conf = nullptr;
    seeding_conf* s_conf = nullptr;
    integration_conf* i_conf = nullptr;

    // Holds the vector field data
    std::unique_ptr<globjects::Texture>      tex0_ptr = nullptr;
    std::unique_ptr<globjects::Texture>      tex1_ptr = nullptr;

    std::unique_ptr<globjects::Buffer>       b_denormalization = nullptr;
    std::unique_ptr<globjects::Buffer>       b_test = nullptr;

    std::unique_ptr<globjects::UniformBlock> ubo_seeding     = nullptr;
    std::unique_ptr<globjects::UniformBlock> ubo_integration = nullptr;
    std::unique_ptr<globjects::Buffer>       b_seeding       = nullptr;
    std::unique_ptr<globjects::Buffer>       b_integration   = nullptr;

    std::unique_ptr<globjects::StaticStringSource>   compute_shader_source   = nullptr;
    std::unique_ptr<globjects::AbstractStringSource> compute_shader_template = nullptr;
    std::unique_ptr<globjects::Shader>               compute_shader          = nullptr;
    std::unique_ptr<globjects::Program>              compute_program         = nullptr;

    std::unique_ptr<advected_field> output = nullptr;

    vector_field(bool steady_vectorfield = true);

    void init_configuration(antMenu* menu, bool astc_compressed, bool texture_array);
    void init_texture_conf(antMenu* menu, bool astc_compressed, bool texture_array);
    void init_seeding_conf(antMenu* menu);
    void init_integration_conf(antMenu* menu);
    void init_compute_conf(antMenu* menu, bool astc_compressed, bool texture_array);
    void update_configuration(antMenu* menu);
    void update_seeding_conf(antMenu* menu);
    void update_integration_conf(antMenu* menu);

    // Assemble a compute shader based on the input data (only for 3D / sliced 3D data)
    double setup_compute_shader(bool componentwise_normalized = false, bool measure_time = true);
    // Pass the compute shader code and corresponding texture target directly
    //void setup_compute_shader(std::string compute_shader_code, gl::GLenum texture_target);

    // Create, initializes and assign Texture Objects
    double setup_textures(bool measure_time = true);
    double setup_astc_textures(bool measure_time = true);

    // Create, initializes & assigns various (Uniform) Buffer Objects
    double setup_storage_buffers(bool measure_time = true);
    double setup_uniform_buffers(bool measure_time = true);
    double setup_denormalization_buffer(bool measure_time = true);
    void setup_test_buffer();

    // Upload (new) data to the corresponding OpenGL Objects. Except for the storage_buffers (just resize them).
    double update_seeding(bool measure_time = true);
    double update_integration(bool measure_time = true);
    double update_storage_buffers(bool measure_time = true); // Only binds buffers as SSBOs & reallocates GPU memory for to be advected data
    double update_denormalization_buffer(std::vector<float>& denormalization_data, bool measure_time = true);

    // Uploads texture data
    double update_texture(gl::GLvoid* data, std::size_t offset_byte = 0, bool measure_time = true);
    double update_texture_t0(gl::GLvoid* data, std::size_t offset_byte = 0, bool measure_time = true);  // Just a rename of update_texture
    double update_texture_t1(gl::GLvoid* data, std::size_t offset_byte = 0, bool measure_time = true);

    double update_astc_texture(gl::GLvoid* data, std::size_t offset_byte = 0, bool measure_time = true);
    double update_astc_texture_t0(gl::GLvoid* data, std::size_t offset_byte = 0, bool measure_time = true);  // Just a rename of update_astc_texture
    double update_astc_texture_t1(gl::GLvoid* data, std::size_t offset_byte = 0, bool measure_time = true);

    void update_global_time(const std::uint32_t t, const bool last = false);

    void update_advection_count();

    double advect(bool measure_time = true);
    double unsteady_advect(std::vector<float>& data, bool measure_time = true);
    double unsteady_advect(std::vector<astc_datatype>& data, bool measure_time = true);

    // Returns an object holding all information for rendering the result
    advected_field* get_result();    

    void add_timer(std::string name, double timing, antMenu* menu);
    void add_unsteady_timer(unsteady_timings timings, antMenu* menu);
    
  protected:
    long int generation_count = -1;

    void setup_texture(std::unique_ptr<globjects::Texture>& texture, int texture_index, std::string sampler_name);
    void setup_compressed_texture(std::unique_ptr<globjects::Texture>& texture, int texture_index, std::string sampler_name);

    void update_texture(std::unique_ptr<globjects::Texture>& texture, const gl::GLvoid* data, std::size_t offset_byte);
    void update_astc_texture(std::unique_ptr<globjects::Texture>& texture, const gl::GLvoid* data, std::size_t offset_byte);

    gl::GLenum get_internal_format     (int vec_len);
    gl::GLenum get_internal_astc_format(int blocksize_x, int blocksize_y, int blocksize_z);
    gl::GLenum get_internal_astc_format(glm::uvec3);
    gl::GLenum get_format              (int vec_len);

    performance p;
  };
}

#endif