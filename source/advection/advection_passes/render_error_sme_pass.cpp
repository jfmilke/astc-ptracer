#include <jay/advection/advection_passes/render_error_sme_pass.hpp>
#include <jay/core/camera.hpp>
#include <jay/core/menu.hpp>
#include <jay/advection/advected_field.hpp>

#include <glbinding/gl/gl.h>
#include <glbinding/gl/types.h>
#include <globjects/globjects.h>

#include <iostream>

namespace jay
{

  render_error_sme_pass::render_error_sme_pass(antMenu* menu, camera* camera, advected_field* field0, advected_field* field1)
  {
    on_prepare = [&, menu, camera, field0, field1]()
    {
      gl::glEnable(gl::GL_BLEND);
      gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA);
      gl::glEnable(gl::GL_DEPTH_TEST);
      gl::glDepthFunc(gl::GL_LESS);
      gl::glDisable(gl::GL_CULL_FACE);

      
    };
    on_update = [&, menu, camera, field0, field1]()
    {
      
    };
  }

  render_error_sme_pass make_render_error_sme_pass(antMenu* menu, camera* camera, advected_field* field0, advected_field* field1)
  {
    return render_error_sme_pass(menu, camera, field0, field1);
  }
}

