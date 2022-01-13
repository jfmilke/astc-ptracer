#ifndef JAY_CORE_SYSTEM_HPP
#define JAY_CORE_SYSTEM_HPP

#include <functional>

#include <jay/export.hpp>

namespace jay
{
struct JAY_EXPORT system
{
  std::function<void()> on_prepare = [ ] ( ) { };
  std::function<void()> on_update  = [ ] ( ) { };
};
}

#endif