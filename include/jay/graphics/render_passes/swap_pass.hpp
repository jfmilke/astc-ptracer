#ifndef JAY_GRAPHICS_RENDER_PASSES_SWAP_PASS_HPP
#define JAY_GRAPHICS_RENDER_PASSES_SWAP_PASS_HPP

#include <jay/graphics/render_pass.hpp>
#include <jay/export.hpp>

typedef struct GLFWwindow GLFWwindow;

namespace jay
{
JAY_EXPORT render_pass make_swap_pass(GLFWwindow* window);
}

#endif