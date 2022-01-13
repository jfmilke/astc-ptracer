#ifndef JAY_UTILITY_PERMUTE_FOR_HPP
#define JAY_UTILITY_PERMUTE_FOR_HPP

#include <cstddef>
#include <functional>

namespace jay
{
// Permutes the loop for(auto i = start, i < end; i+= step) over all dimensions.
template <typename type>
void permute_for(
  const std::function<void(const type&)>& function, 
  const type&                             start   ,
  const type&                             end     ,
  const type&                             step    )
{
  std::function<void(type, std::size_t)> permute_for_internal =
    [&] (type indices, std::size_t depth)
    {
      if (depth < start.length())
      {
        for (auto i = start[depth]; i < end[depth]; i += step[depth])
        {
          indices[depth] = i;
          permute_for_internal(indices, depth + 1);
        }
      }
      else
        function(indices);
    };
  permute_for_internal(type(), 0);
}
}

#endif