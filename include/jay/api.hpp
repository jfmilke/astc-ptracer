#ifndef JAY_API_HPP
#define JAY_API_HPP

#include <jay/core/application.hpp>
#include <jay/core/engine.hpp>
#include <jay/core/system.hpp>
#include <jay/core/menu.hpp>
#include <jay/core/camera.hpp>

#include <jay/advection/advected_field.hpp>
#include <jay/advection/vector_field.hpp>

#include <jay/graphics/render_passes/clear_pass.hpp>
#include <jay/graphics/render_passes/swap_pass.hpp>
#include <jay/core/utility_passes/camera_pass.hpp>
#include <jay/core/utility_passes/menu_pass.hpp>
#include <jay/core/utility_passes/image_pass.hpp>
#include <jay/core/utility_passes/timer_pass.hpp>
#include <jay/core/utility_passes/auto_pass.hpp>
#include <jay/advection/advection_passes/advection_pass.hpp>
#include <jay/advection/advection_passes/render_advection_pass.hpp>
#include <jay/advection/advection_passes/render_error_area_pass.hpp>
#include <jay/graphics/render_pass.hpp>
#include <jay/graphics/renderer.hpp>

#include <jay/types/image.hpp>
#include <jay/types/jaydata.hpp>
#include <jay/io/io_enums.hpp>
#include <jay/io/io.hpp>
#include <jay/io/image_io.hpp>
#include <jay/compression/compressor.hpp>
#include <jay/compression/astc.hpp>

#include <jay/analysis/performance_measure.hpp>
#include <jay/analysis/distance_measure.hpp>
#include <jay/analysis/psnr_measure.hpp>
#include <jay/analysis/statistics.hpp>

#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <globjects/base/AbstractStringSource.h>
#include <globjects/base/StaticStringSource.h>
#include <globjects/globjects.h>

#endif