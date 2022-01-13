#include <jay/core/utility_passes/image_pass.hpp>
#include <jay/core/menu.hpp>
#include <jay/types/image.hpp>
#include <jay/io/image_io.hpp>
#include <glbinding/gl/gl.h>

namespace jay
{
  image_pass::image_pass(antMenu* menu)
  {
    on_prepare = [&]()
    {};
    on_update = [&, menu]()
    {
      if (menu->export_image())
      {
        gl::GLint dims[4] = { 0 };
        gl::glGetIntegerv(gl::GL_VIEWPORT, dims);
        gl::GLint fbWidth = dims[2];
        gl::GLint fbHeight = dims[3];

        image* img = new image();
        img->data.resize(fbWidth * fbHeight * 3);
        img->size = { fbWidth, fbHeight };
        gl::glReadPixels(0, 0, fbWidth, fbHeight, gl::GL_RGB, gl::GL_UNSIGNED_BYTE, img->data.data());
        auto status = image_io::export_bmp("../files/exports/" + menu->export_img_name, img, 3);

        if (status == -1)
          menu->export_img_status = false;
        else
          menu->export_img_status = true;
      }
    };
  }

  image_pass make_image_pass(antMenu* menu)
  {
    return image_pass(menu);
  }
}
